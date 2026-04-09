#!/usr/bin/env python3
"""
Minimal bridge: 보드 JSON의 차선 점만 호모그래피로 BEV 좌표로 변환해 Qt로 전달한다.
스무딩·다항 피팅·홀드·객체 스냅·트래킹 등 보정 로직은 넣지 않는다.

BEV 좌표 수정: --bev-tweak-json / --bev-post-matrix / --bev-offset-* / --bev-scale-* / --bev-pivot-*
(호모그래피 이후에만 적용).

차선: BEV 로 투영한 뒤 --lane-fit-degree>0 이면 y 방향 다항식(polyfit)으로 곡선 피팅 후
샘플 점으로 재생성(perception_bridge 와 동일 아이디어).
"""

from __future__ import annotations

import argparse
import json
import socket
import sys
import threading
import time
from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Tuple

import cv2
import numpy as np

from bev_lane_modules.geometry import load_matrix3_json as module_load_matrix3_json
from bev_lane_modules.geometry import merge_bev_correction as module_merge_bev_correction
from bev_lane_modules.payload import build_bev_payload as module_build_bev_payload
from bev_lane_modules.state import BevLaneState as ModuleBevLaneState


def smooth_lane_points(
    previous: List[Dict[str, int]],
    current: List[Dict[str, int]],
    alpha: float,
) -> List[Dict[str, int]]:
    if not previous or len(previous) != len(current):
        return [dict(point) for point in current]

    smoothed: List[Dict[str, int]] = []
    for prev, curr in zip(previous, current):
        smoothed.append(
            {
                "x": int(round(alpha * prev["x"] + (1.0 - alpha) * curr["x"])),
                "y": int(round(alpha * prev["y"] + (1.0 - alpha) * curr["y"])),
            }
        )
    return smoothed


def smooth_lane_set(
    previous_set: List[List[Dict[str, int]]],
    current_set: List[List[Dict[str, int]]],
    alpha: float,
) -> List[List[Dict[str, int]]]:
    smoothed_set: List[List[Dict[str, int]]] = [[], [], [], []]
    for lane_idx in range(4):
        previous = previous_set[lane_idx] if lane_idx < len(previous_set) else []
        current = current_set[lane_idx] if lane_idx < len(current_set) else []
        smoothed_set[lane_idx] = smooth_lane_points(previous, current, alpha)
    return smoothed_set


class BevLaneState:
    def __init__(self):
        self.lock = threading.Lock()
        self.smoothed_lanes: List[List[Dict[str, int]]] = [[], [], [], []]
        self.object_tracks: List[Dict[str, Any]] = []
        self.next_track_id = 1
        self.smoothed_steering_deg = 0.0

    def smooth(self, lanes: List[List[Dict[str, int]]], alpha: float) -> List[List[Dict[str, int]]]:
        with self.lock:
            self.smoothed_lanes = smooth_lane_set(self.smoothed_lanes, lanes, alpha)
            return [[dict(point) for point in lane] for lane in self.smoothed_lanes]

    def smooth_steering(self, steering_deg: float, alpha: float) -> float:
        with self.lock:
            self.smoothed_steering_deg = alpha * self.smoothed_steering_deg + (1.0 - alpha) * steering_deg
            return self.smoothed_steering_deg

    def update_object_tracks(
        self,
        objects: List[Dict[str, Any]],
        max_distance: float,
        confirm_frames: int,
        miss_frames: int,
        alpha: float,
        max_objects: int,
    ) -> List[Dict[str, Any]]:
        with self.lock:
            detections = [dict(obj) for obj in objects]
            unmatched_tracks = set(range(len(self.object_tracks)))
            matches: List[Tuple[int, int, float]] = []

            for det_idx, det in enumerate(detections):
                best_track = -1
                best_dist = max_distance
                det_x = float(det.get("bev_x", 0.0))
                det_y = float(det.get("bev_y", 0.0))
                for track_idx in list(unmatched_tracks):
                    track = self.object_tracks[track_idx]
                    dx = det_x - float(track.get("bev_x", 0.0))
                    dy = det_y - float(track.get("bev_y", 0.0))
                    dist = float((dx * dx + dy * dy) ** 0.5)
                    if dist <= best_dist:
                        best_dist = dist
                        best_track = track_idx
                if best_track >= 0:
                    unmatched_tracks.remove(best_track)
                    matches.append((det_idx, best_track, best_dist))

            matched_detection_ids = set()
            for det_idx, track_idx, _dist in matches:
                det = detections[det_idx]
                track = self.object_tracks[track_idx]
                track["bev_x"] = alpha * float(track.get("bev_x", 0.0)) + (1.0 - alpha) * float(det.get("bev_x", 0.0))
                track["bev_y"] = alpha * float(track.get("bev_y", 0.0)) + (1.0 - alpha) * float(det.get("bev_y", 0.0))
                track["cls"] = det.get("cls", track.get("cls"))
                track["score"] = det.get("score", track.get("score"))
                track["hits"] = int(track.get("hits", 0)) + 1
                track["misses"] = 0
                track["confirmed"] = track["hits"] >= confirm_frames
                matched_detection_ids.add(det_idx)

            for track_idx in list(unmatched_tracks):
                track = self.object_tracks[track_idx]
                track["misses"] = int(track.get("misses", 0)) + 1

            for det_idx, det in enumerate(detections):
                if det_idx in matched_detection_ids:
                    continue
                self.object_tracks.append(
                    {
                        "track_id": self.next_track_id,
                        "bev_x": float(det.get("bev_x", 0.0)),
                        "bev_y": float(det.get("bev_y", 0.0)),
                        "cls": det.get("cls"),
                        "score": det.get("score"),
                        "hits": 1,
                        "misses": 0,
                        "confirmed": confirm_frames <= 1,
                    }
                )
                self.next_track_id += 1

            self.object_tracks = [
                track for track in self.object_tracks if int(track.get("misses", 0)) <= miss_frames
            ]

            published = [
                {
                    "track_id": int(track["track_id"]),
                    "bev_x": int(round(float(track["bev_x"]))),
                    "bev_y": int(round(float(track["bev_y"]))),
                    "cls": track.get("cls"),
                    "score": track.get("score"),
                }
                for track in self.object_tracks
                if bool(track.get("confirmed", False))
            ]
            published.sort(key=lambda obj: float(obj.get("bev_y", -1e9)), reverse=True)
            if max_objects > 0:
                published = published[:max_objects]
            return published


def fit_lane_curve(
    points: List[Dict[str, Any]],
    degree: int,
    sample_count: int,
    min_points: int,
    top_y: Optional[float] = None,
    bottom_y: Optional[float] = None,
) -> List[Dict[str, int]]:
    if len(points) < max(2, min_points):
        return [{"x": int(round(float(p.get("x", 0)))), "y": int(round(float(p.get("y", 0))))} for p in points]

    sorted_points = sorted(points, key=lambda p: float(p.get("y", 0.0)), reverse=True)
    ys = np.array([float(point.get("y", 0.0)) for point in sorted_points], dtype=np.float32)
    xs = np.array([float(point.get("x", 0.0)) for point in sorted_points], dtype=np.float32)

    unique_ys, unique_indices = np.unique(ys, return_index=True)
    xs = xs[unique_indices]
    ys = unique_ys

    if len(ys) < max(2, min_points):
        return [{"x": int(round(float(p.get("x", 0)))), "y": int(round(float(p.get("y", 0))))} for p in sorted_points]

    fit_degree = min(max(1, degree), len(ys) - 1)
    if fit_degree < 1:
        return [{"x": int(round(float(p.get("x", 0)))), "y": int(round(float(p.get("y", 0))))} for p in sorted_points]

    try:
        coeffs = np.polyfit(ys, xs, fit_degree)
    except (np.linalg.LinAlgError, ValueError):
        return [{"x": int(round(float(p.get("x", 0)))), "y": int(round(float(p.get("y", 0))))} for p in sorted_points]

    y_start = float(np.max(ys)) if bottom_y is None else float(bottom_y)
    y_end = float(np.min(ys)) if top_y is None else float(top_y)
    if abs(y_start - y_end) < 1e-3:
        return [{"x": int(round(float(p.get("x", 0)))), "y": int(round(float(p.get("y", 0))))} for p in sorted_points]

    sample_ys = np.linspace(y_start, y_end, max(2, sample_count), dtype=np.float32)
    sample_xs = np.polyval(coeffs, sample_ys)

    fitted: List[Dict[str, int]] = []
    for x_val, y_val in zip(sample_xs, sample_ys):
        fitted.append(
            {
                "x": int(round(float(x_val))),
                "y": int(round(float(y_val))),
            }
        )
    return fitted


def fit_lane_set(
    lane_set: List[List[Dict[str, Any]]],
    degree: int,
    sample_count: int,
    min_points: int,
    top_y: Optional[float] = None,
    bottom_y: Optional[float] = None,
) -> List[List[Dict[str, int]]]:
    fitted_set: List[List[Dict[str, int]]] = [[], [], [], []]
    for lane_idx in range(4):
        lane_points = lane_set[lane_idx] if lane_idx < len(lane_set) else []
        fitted_set[lane_idx] = fit_lane_curve(
            lane_points, degree, sample_count, min_points, top_y, bottom_y
        )
    return fitted_set


@dataclass
class BevCorrection:
    """호모그래피로 얻은 BEV 좌표에만 적용 (순서: projective post → 피벗 스케일 → 오프셋)."""

    post_matrix: Optional[np.ndarray]
    offset_x: float
    offset_y: float
    scale_x: float
    scale_y: float
    pivot_x: float
    pivot_y: float

    def apply_xy(self, x: float, y: float) -> Tuple[int, int]:
        xf = float(x)
        yf = float(y)
        if self.post_matrix is not None:
            m = self.post_matrix.astype(np.float64)
            v = np.array([xf, yf, 1.0], dtype=np.float64)
            w = m @ v
            if abs(w[2]) < 1e-12:
                return int(round(xf)), int(round(yf))
            xf = w[0] / w[2]
            yf = w[1] / w[2]
        xf = (xf - self.pivot_x) * self.scale_x + self.pivot_x + self.offset_x
        yf = (yf - self.pivot_y) * self.scale_y + self.pivot_y + self.offset_y
        return int(round(xf)), int(round(yf))


def merge_bev_correction(
    bev_w: int,
    bev_h: int,
    tweak_json_path: Optional[str],
    post_matrix_path: Optional[str],
    offset_x: Optional[float],
    offset_y: Optional[float],
    scale_x: Optional[float],
    scale_y: Optional[float],
    pivot_x: Optional[float],
    pivot_y: Optional[float],
) -> BevCorrection:
    d: Dict[str, float] = {
        "offset_x": 0.0,
        "offset_y": 0.0,
        "scale_x": 1.0,
        "scale_y": 1.0,
        "pivot_x": bev_w / 2.0,
        "pivot_y": bev_h / 2.0,
    }
    if tweak_json_path:
        t = json.loads(open(tweak_json_path, "r", encoding="utf-8").read())
        for key in d:
            if key in t:
                d[key] = float(t[key])
    if offset_x is not None:
        d["offset_x"] = offset_x
    if offset_y is not None:
        d["offset_y"] = offset_y
    if scale_x is not None:
        d["scale_x"] = scale_x
    if scale_y is not None:
        d["scale_y"] = scale_y
    if pivot_x is not None:
        d["pivot_x"] = pivot_x
    if pivot_y is not None:
        d["pivot_y"] = pivot_y

    post: Optional[np.ndarray] = None
    if post_matrix_path:
        post = load_matrix3_json(post_matrix_path)

    return BevCorrection(
        post,
        d["offset_x"],
        d["offset_y"],
        d["scale_x"],
        d["scale_y"],
        d["pivot_x"],
        d["pivot_y"],
    )


def load_matrix3_json(path: str) -> np.ndarray:
    raw = json.loads(open(path, "r", encoding="utf-8").read())
    matrix = raw.get("matrix", raw.get("homography", raw))
    arr = np.asarray(matrix, dtype=np.float32)
    if arr.shape != (3, 3):
        raise ValueError(f"expected 3x3 matrix, got {arr.shape}")
    return arr


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="BEV-only lane bridge (no smoothing / fit / snap / tracking)."
    )
    p.add_argument("--board-host", default="192.168.0.100", help="Inference host")
    p.add_argument("--video-port", type=int, default=9999, help="RGB TCP port")
    p.add_argument("--result-port", type=int, default=9998, help="JSON result TCP port")
    p.add_argument("--qt-host", default="0.0.0.0", help="Bind for Qt clients")
    p.add_argument("--qt-port", type=int, default=10000, help="Port for Qt clients")
    p.add_argument("--video", default="output.mp4", help="Video file to stream to board")
    p.add_argument("--width", type=int, default=1280, help="Frame width sent to board")
    p.add_argument("--height", type=int, default=720, help="Frame height sent to board")
    p.add_argument(
        "--homography",
        required=True,
        help="JSON file with 3x3 homography (or {\"homography\": [[...], ...]})",
    )
    p.add_argument("--bev-width", type=int, default=1280, help="BEV canvas width (Qt width/height)")
    p.add_argument("--bev-height", type=int, default=720, help="BEV canvas height")
    p.add_argument("--max-objects", type=int, default=32, help="Cap raw objects forwarded (0 = no cap)")
    p.add_argument("--object-track-distance", type=float, default=60.0, help="Maximum BEV pixel distance for matching detections to existing tracks")
    p.add_argument("--object-track-confirm", type=int, default=2, help="Frames required before a tracked object is published")
    p.add_argument("--object-track-miss", type=int, default=4, help="Frames to keep a tracked object alive without detections")
    p.add_argument("--object-track-alpha", type=float, default=0.7, help="EMA factor for tracked object position smoothing (0-1)")
    p.add_argument(
        "--no-objects",
        action="store_true",
        help="Do not transform or forward detection objects (lanes only).",
    )
    p.add_argument("--loop", action="store_true", help="Loop input video")
    p.add_argument("--fps", type=float, default=None, help="Override send FPS")
    p.add_argument("--no-throttle", action="store_true", help="Send video frames as fast as possible")
    p.add_argument(
        "--no-video",
        action="store_true",
        help="Do not open/send video; only subscribe to JSON and broadcast BEV (board fed elsewhere).",
    )
    p.add_argument(
        "--bev-tweak-json",
        default=None,
        help="Optional JSON: offset_x, offset_y, scale_x, scale_y, pivot_x, pivot_y (CLI overrides same keys).",
    )
    p.add_argument(
        "--bev-post-matrix",
        default=None,
        help="Optional 3x3 JSON applied in BEV space after homography (same format as homography file).",
    )
    p.add_argument("--bev-offset-x", type=float, default=None, help="Override: add to BEV x after scale")
    p.add_argument("--bev-offset-y", type=float, default=None, help="Override: add to BEV y after scale")
    p.add_argument("--bev-scale-x", type=float, default=None, help="Override: scale x around pivot (default 1)")
    p.add_argument("--bev-scale-y", type=float, default=None, help="Override: scale y around pivot (default 1)")
    p.add_argument("--bev-pivot-x", type=float, default=None, help="Override: scale pivot x (default bev_width/2)")
    p.add_argument("--bev-pivot-y", type=float, default=None, help="Override: scale pivot y (default bev_height/2)")
    p.add_argument(
        "--lane-fit-degree",
        type=int,
        default=2,
        help="BEV lane polynomial fit: 0=off, 1=line, 2=quadratic (default: 2)",
    )
    p.add_argument("--lane-fit-samples", type=int, default=24, help="Points sampled along fitted curve per lane")
    p.add_argument(
        "--lane-fit-min-points",
        type=int,
        default=4,
        help="Minimum raw BEV points required before fitting",
    )
    p.add_argument(
        "--lane-smoothing",
        type=float,
        default=0.7,
        help="EMA factor for BEV lane stabilization (0-1)",
    )
    p.add_argument(
        "--steering-smoothing",
        type=float,
        default=0.75,
        help="EMA factor for Pure Pursuit steering stabilization (0-1)",
    )
    p.add_argument(
        "--lane-top-ratio",
        type=float,
        default=0.12,
        help="Normalized top Y on BEV canvas for fit range",
    )
    p.add_argument(
        "--lane-bottom-ratio",
        type=float,
        default=0.97,
        help="Normalized bottom Y on BEV canvas for fit range",
    )
    p.add_argument(
        "--pp-lookahead-ratio",
        type=float,
        default=0.58,
        help="Normalized BEV Y used as Pure Pursuit look-ahead target",
    )
    p.add_argument(
        "--pp-wheelbase-ratio",
        type=float,
        default=0.12,
        help="Pseudo wheelbase as ratio of BEV height for Pure Pursuit",
    )
    p.add_argument(
        "--pp-ego-x-ratio",
        type=float,
        default=0.5,
        help="Normalized ego vehicle X position in BEV",
    )
    p.add_argument(
        "--pp-ego-y-ratio",
        type=float,
        default=0.97,
        help="Normalized ego vehicle Y position in BEV",
    )
    p.add_argument(
        "--pp-max-steer-deg",
        type=float,
        default=35.0,
        help="Clamp Pure Pursuit steering output in degrees",
    )
    return p.parse_args()


class QtBroadcastServer:
    def __init__(self, host: str, port: int):
        self.host = host
        self.port = port
        self.server_sock: Optional[socket.socket] = None
        self.clients: List[socket.socket] = []
        self.lock = threading.Lock()
        self.stop_event = threading.Event()
        self.thread: Optional[threading.Thread] = None

    def start(self) -> None:
        self.server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_sock.bind((self.host, self.port))
        self.server_sock.listen(5)
        self.thread = threading.Thread(target=self._accept_loop, daemon=True)
        self.thread.start()
        print(f"[bev-lane] Qt server {self.host}:{self.port}")

    def _accept_loop(self) -> None:
        assert self.server_sock is not None
        while not self.stop_event.is_set():
            try:
                client, addr = self.server_sock.accept()
            except OSError:
                break
            with self.lock:
                self.clients.append(client)
            print(f"[bev-lane] Qt client {addr[0]}:{addr[1]}")

    def broadcast(self, payload: Dict[str, Any]) -> None:
        msg = (json.dumps(payload, separators=(",", ":")) + "\n").encode("utf-8")
        stale: List[socket.socket] = []
        with self.lock:
            for c in self.clients:
                try:
                    c.sendall(msg)
                except OSError:
                    stale.append(c)
            for c in stale:
                try:
                    c.close()
                except OSError:
                    pass
                if c in self.clients:
                    self.clients.remove(c)

    def stop(self) -> None:
        self.stop_event.set()
        if self.server_sock is not None:
            try:
                self.server_sock.close()
            except OSError:
                pass
        with self.lock:
            for c in self.clients:
                try:
                    c.close()
                except OSError:
                    pass
            self.clients.clear()


def connect(host: str, port: int) -> socket.socket:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    return s


def connect_with_retry(host: str, port: int, stop_event: threading.Event, label: str) -> socket.socket:
    while not stop_event.is_set():
        try:
            return connect(host, port)
        except OSError as exc:
            print(f"[bev-lane] waiting for {label} {host}:{port} ({exc})")
            time.sleep(1.0)
    raise RuntimeError(f"stopped while waiting for {label}")


def load_homography(path: str) -> np.ndarray:
    return module_load_matrix3_json(path)


def scale_points(
    points: List[Dict[str, Any]],
    src_w: int,
    src_h: int,
    dst_w: int,
    dst_h: int,
) -> List[Dict[str, float]]:
    sx = float(dst_w) / max(src_w, 1)
    sy = float(dst_h) / max(src_h, 1)
    return [{"x": float(p.get("x", 0.0)) * sx, "y": float(p.get("y", 0.0)) * sy} for p in points]


def transform_points(
    points: List[Dict[str, Any]],
    homography: np.ndarray,
    src_w: int,
    src_h: int,
    dst_w: int,
    dst_h: int,
    correction: Optional[BevCorrection] = None,
) -> List[Dict[str, int]]:
    scaled = scale_points(points, src_w, src_h, dst_w, dst_h)
    if not scaled:
        return []
    src = np.array([[[p["x"], p["y"]] for p in scaled]], dtype=np.float32)
    dst = cv2.perspectiveTransform(src, homography)[0]
    out: List[Dict[str, int]] = []
    for x, y in dst:
        if correction is not None:
            ix, iy = correction.apply_xy(float(x), float(y))
        else:
            ix, iy = int(round(x)), int(round(y))
        out.append({"x": ix, "y": iy})
    return out


def transform_object_foot(
    obj: Dict[str, Any],
    homography: np.ndarray,
    src_w: int,
    src_h: int,
    dst_w: int,
    dst_h: int,
    correction: Optional[BevCorrection] = None,
) -> Dict[str, Any]:
    out = dict(obj)
    x_min = float(obj.get("x_min", 0.0))
    x_max = float(obj.get("x_max", 0.0))
    y_max = float(obj.get("y_max", 0.0))
    foot = [{"x": 0.5 * (x_min + x_max), "y": y_max}]
    pt = transform_points(foot, homography, src_w, src_h, dst_w, dst_h, correction)[0]
    out["bev_x"] = pt["x"]
    out["bev_y"] = pt["y"]
    return out


def build_centerline(
    left_lane: List[Dict[str, int]],
    right_lane: List[Dict[str, int]],
) -> List[Dict[str, int]]:
    if not left_lane or not right_lane:
        return []

    point_count = min(len(left_lane), len(right_lane))
    centerline: List[Dict[str, int]] = []
    for point_idx in range(point_count):
        left = left_lane[point_idx]
        right = right_lane[point_idx]
        centerline.append(
            {
                "x": int(round((float(left["x"]) + float(right["x"])) * 0.5)),
                "y": int(round((float(left["y"]) + float(right["y"])) * 0.5)),
            }
        )
    return centerline


def choose_lookahead_point(
    centerline: List[Dict[str, int]],
    target_y: float,
) -> Optional[Dict[str, int]]:
    if not centerline:
        return None
    return min(centerline, key=lambda point: abs(float(point["y"]) - target_y))


def compute_pure_pursuit_steering(
    left_lane: List[Dict[str, int]],
    right_lane: List[Dict[str, int]],
    bev_width: int,
    bev_height: int,
    lookahead_ratio: float,
    wheelbase_ratio: float,
    ego_x_ratio: float,
    ego_y_ratio: float,
    max_steer_deg: float,
) -> Tuple[float, List[Dict[str, int]], Optional[Dict[str, int]]]:
    centerline = build_centerline(left_lane, right_lane)
    if not centerline:
        return 0.0, [], None

    ego_x = float(bev_width) * ego_x_ratio
    ego_y = float(bev_height) * ego_y_ratio
    target_y = float(bev_height) * lookahead_ratio
    lookahead_point = choose_lookahead_point(centerline, target_y)
    if lookahead_point is None:
        return 0.0, centerline, None

    dx = float(lookahead_point["x"]) - ego_x
    dy = ego_y - float(lookahead_point["y"])
    if dy <= 1e-3:
        return 0.0, centerline, lookahead_point

    lookahead_dist = max((dx * dx + dy * dy) ** 0.5, 1e-3)
    wheelbase = max(float(bev_height) * wheelbase_ratio, 1e-3)
    alpha = float(np.arctan2(dx, dy))
    steering_rad = float(np.arctan2(2.0 * wheelbase * np.sin(alpha), lookahead_dist))
    steering_deg = float(np.degrees(steering_rad))
    steering_deg = max(-max_steer_deg, min(max_steer_deg, steering_deg))
    return steering_deg, centerline, lookahead_point


def build_bev_payload(
    result: Dict[str, Any],
    homography: np.ndarray,
    bev_width: int,
    bev_height: int,
    source_width: int,
    source_height: int,
    include_objects: bool,
    max_objects: int,
    correction: Optional[BevCorrection] = None,
    lane_fit_degree: int = 2,
    lane_fit_samples: int = 24,
    lane_fit_min_points: int = 4,
    lane_top_ratio: float = 0.12,
    lane_bottom_ratio: float = 0.97,
    lane_state: Optional[BevLaneState] = None,
    lane_smoothing: float = 0.7,
    steering_smoothing: float = 0.75,
    pp_lookahead_ratio: float = 0.58,
    pp_wheelbase_ratio: float = 0.12,
    pp_ego_x_ratio: float = 0.5,
    pp_ego_y_ratio: float = 0.97,
    pp_max_steer_deg: float = 35.0,
    object_track_distance: float = 60.0,
    object_track_confirm: int = 2,
    object_track_miss: int = 4,
    object_track_alpha: float = 0.7,
) -> Dict[str, Any]:
    pw = int(result.get("width", 800))
    ph = int(result.get("height", 480))
    lanes_in = result.get("lanes", []) or []

    bev_by_id: List[List[Dict[str, int]]] = [[], [], [], []]
    for lane in lanes_in:
        lid = int(lane.get("lane_id", -1))
        if 0 <= lid < 4:
            bev_by_id[lid] = transform_points(
                lane.get("points", []) or [],
                homography,
                pw,
                ph,
                source_width,
                source_height,
                correction,
            )

    if lane_fit_degree > 0:
        bev_top_y = float(bev_height) * lane_top_ratio
        bev_bottom_y = float(bev_height) * lane_bottom_ratio
        bev_by_id = fit_lane_set(
            bev_by_id,
            lane_fit_degree,
            lane_fit_samples,
            lane_fit_min_points,
            bev_top_y,
            bev_bottom_y,
        )

    if lane_state is not None:
        bev_by_id = lane_state.smooth(bev_by_id, lane_smoothing)

    qt_lanes: List[List[Dict[str, int]]] = [[], [], [], []]
    qt_lanes[1] = bev_by_id[1]
    qt_lanes[2] = bev_by_id[2]

    steering_deg, centerline, lookahead_point = compute_pure_pursuit_steering(
        qt_lanes[1],
        qt_lanes[2],
        bev_width,
        bev_height,
        pp_lookahead_ratio,
        pp_wheelbase_ratio,
        pp_ego_x_ratio,
        pp_ego_y_ratio,
        pp_max_steer_deg,
    )
    if lane_state is not None:
        steering_deg = lane_state.smooth_steering(steering_deg, steering_smoothing)

    perf = result.get("perf", {}) or {}
    payload: Dict[str, Any] = {
        "frame_index": int(result.get("frame_index", 0)),
        "width": bev_width,
        "height": bev_height,
        "lanes": qt_lanes,
        "bev_width": bev_width,
        "bev_height": bev_height,
        "bev_lanes": qt_lanes,
        "perf": {
            "fps": float(perf.get("fps", 0.0)),
            "cpu": int(perf.get("cpu", 0)),
            "mem": int(perf.get("mem", 0)),
        },
        "speed_kmh": float(result.get("speed_kmh", 0.0)),
        "steering": float(steering_deg),
        "steering_source": "pure_pursuit",
        "centerline": centerline,
        "lookahead_point": lookahead_point if lookahead_point is not None else {},
    }

    if include_objects:
        objs: List[Dict[str, Any]] = list(result.get("objects", []) or [])
        if max_objects > 0:
            objs = objs[:max_objects]
        bev_objs = [
            transform_object_foot(o, homography, pw, ph, source_width, source_height, correction)
            for o in objs
        ]
        if lane_state is not None:
            bev_objs = lane_state.update_object_tracks(
                bev_objs,
                object_track_distance,
                object_track_confirm,
                object_track_miss,
                object_track_alpha,
                max_objects,
            )
        payload["objects"] = objs
        payload["bev_objects"] = bev_objs
        lane_status = [False, False, False, False, False]
        for o in bev_objs:
            cx = float(o.get("bev_x", 0.0))
            idx = int(cx * 5.0 / max(bev_width, 1))
            lane_status[max(0, min(4, idx))] = True
        payload["lane_status"] = lane_status
    else:
        payload["objects"] = []
        payload["bev_objects"] = []
        payload["lane_status"] = [False, False, False, False, False]

    return payload


def result_loop(
    board_host: str,
    result_port: int,
    qt: QtBroadcastServer,
    stop_event: threading.Event,
    homography: np.ndarray,
    bev_width: int,
    bev_height: int,
    source_width: int,
    source_height: int,
    include_objects: bool,
    max_objects: int,
    correction: Optional[BevCorrection],
    lane_fit_degree: int = 2,
    lane_fit_samples: int = 24,
    lane_fit_min_points: int = 4,
    lane_top_ratio: float = 0.12,
    lane_bottom_ratio: float = 0.97,
    lane_state: Optional[BevLaneState] = None,
    lane_smoothing: float = 0.7,
    steering_smoothing: float = 0.75,
    pp_lookahead_ratio: float = 0.58,
    pp_wheelbase_ratio: float = 0.12,
    pp_ego_x_ratio: float = 0.5,
    pp_ego_y_ratio: float = 0.97,
    pp_max_steer_deg: float = 35.0,
    object_track_distance: float = 60.0,
    object_track_confirm: int = 2,
    object_track_miss: int = 4,
    object_track_alpha: float = 0.7,
) -> None:
    while not stop_event.is_set():
        sock = connect_with_retry(board_host, result_port, stop_event, "JSON")
        buf = b""
        print(f"[bev-lane] JSON connected {board_host}:{result_port}")
        try:
            while not stop_event.is_set():
                data = sock.recv(4096)
                if not data:
                    print("[bev-lane] JSON disconnected, retry")
                    break
                buf += data
                while b"\n" in buf:
                    line, buf = buf.split(b"\n", 1)
                    line = line.strip()
                    if not line:
                        continue
                    try:
                        result = json.loads(line.decode("utf-8", errors="replace"))
                    except json.JSONDecodeError:
                        continue
                    out = module_build_bev_payload(
                        result,
                        homography,
                        bev_width,
                        bev_height,
                        source_width,
                        source_height,
                        include_objects,
                        max_objects,
                        correction,
                        lane_fit_degree,
                        lane_fit_samples,
                        lane_fit_min_points,
                        lane_top_ratio,
                        lane_bottom_ratio,
                        lane_state,
                        lane_smoothing,
                        steering_smoothing,
                        pp_lookahead_ratio,
                        pp_wheelbase_ratio,
                        pp_ego_x_ratio,
                        pp_ego_y_ratio,
                        pp_max_steer_deg,
                        object_track_distance,
                        object_track_confirm,
                        object_track_miss,
                        object_track_alpha,
                    )
                    qt.broadcast(out)
        except OSError as exc:
            print(f"[bev-lane] result error: {exc}")
        finally:
            try:
                sock.close()
            except OSError:
                pass
        time.sleep(0.5)


def open_video(path: str):
    cap = cv2.VideoCapture(path)
    if not cap.isOpened():
        raise RuntimeError(f"cannot open video: {path}")
    return cap


def video_loop(args: argparse.Namespace, stop_event: threading.Event) -> None:
    cap = open_video(args.video)
    fps = args.fps if args.fps is not None else cap.get(cv2.CAP_PROP_FPS)
    if fps <= 0.0:
        fps = 30.0
    interval = 0.0 if args.no_throttle else 1.0 / fps
    try:
        while not stop_event.is_set():
            sock = connect_with_retry(args.board_host, args.video_port, stop_event, "video")
            print(f"[bev-lane] video -> {args.board_host}:{args.video_port} {args.width}x{args.height} RGB")
            try:
                while not stop_event.is_set():
                    t0 = time.time()
                    ok, frame = cap.read()
                    if not ok:
                        if args.loop:
                            cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                            continue
                        print("[bev-lane] video end")
                        return
                    small = cv2.resize(frame, (args.width, args.height), interpolation=cv2.INTER_AREA)
                    rgb = cv2.cvtColor(small, cv2.COLOR_BGR2RGB)
                    sock.sendall(rgb.tobytes())
                    elapsed = time.time() - t0
                    sleep = interval - elapsed
                    if interval > 0.0 and sleep > 0:
                        time.sleep(sleep)
            except OSError as exc:
                print(f"[bev-lane] video error: {exc}")
                time.sleep(0.5)
            finally:
                try:
                    sock.close()
                except OSError:
                    pass
    finally:
        cap.release()


def main() -> int:
    args = parse_args()
    homography = load_homography(args.homography)
    print(f"[bev-lane] loaded homography from {args.homography}")

    correction = module_merge_bev_correction(
        args.bev_width,
        args.bev_height,
        args.bev_tweak_json,
        args.bev_post_matrix,
        args.bev_offset_x,
        args.bev_offset_y,
        args.bev_scale_x,
        args.bev_scale_y,
        args.bev_pivot_x,
        args.bev_pivot_y,
    )
    if (
        correction.post_matrix is not None
        or correction.offset_x != 0.0
        or correction.offset_y != 0.0
        or correction.scale_x != 1.0
        or correction.scale_y != 1.0
        or correction.pivot_x != args.bev_width / 2.0
        or correction.pivot_y != args.bev_height / 2.0
    ):
        print(
            f"[bev-lane] BEV correction: post={'yes' if correction.post_matrix is not None else 'no'}, "
            f"offset=({correction.offset_x:.2f},{correction.offset_y:.2f}), "
            f"scale=({correction.scale_x:.4f},{correction.scale_y:.4f}), "
            f"pivot=({correction.pivot_x:.1f},{correction.pivot_y:.1f})"
        )
    if args.lane_fit_degree > 0:
        print(
            f"[bev-lane] lane polyfit: degree={args.lane_fit_degree} "
            f"samples={args.lane_fit_samples} min_pts={args.lane_fit_min_points} "
            f"y_ratio=[{args.lane_top_ratio:.2f},{args.lane_bottom_ratio:.2f}]"
        )
    else:
        print("[bev-lane] lane polyfit: off")

    qt = QtBroadcastServer(args.qt_host, args.qt_port)
    qt.start()
    stop_event = threading.Event()
    lane_state = ModuleBevLaneState()

    receiver = threading.Thread(
        target=result_loop,
        args=(
            args.board_host,
            args.result_port,
            qt,
            stop_event,
            homography,
            args.bev_width,
            args.bev_height,
            args.width,
            args.height,
            not args.no_objects,
            args.max_objects,
            correction,
            args.lane_fit_degree,
            args.lane_fit_samples,
            args.lane_fit_min_points,
            args.lane_top_ratio,
            args.lane_bottom_ratio,
            lane_state,
            args.lane_smoothing,
            args.steering_smoothing,
            args.pp_lookahead_ratio,
            args.pp_wheelbase_ratio,
            args.pp_ego_x_ratio,
            args.pp_ego_y_ratio,
            args.pp_max_steer_deg,
            args.object_track_distance,
            args.object_track_confirm,
            args.object_track_miss,
            args.object_track_alpha,
        ),
        daemon=True,
    )
    receiver.start()

    try:
        if args.no_video:
            print("[bev-lane] --no-video: JSON-only (Ctrl+C to stop)")
            try:
                while True:
                    time.sleep(1.0)
            except KeyboardInterrupt:
                pass
        else:
            video_loop(args, stop_event)
    except KeyboardInterrupt:
        pass
    finally:
        stop_event.set()
        qt.stop()

    return 0


if __name__ == "__main__":
    sys.exit(main())
