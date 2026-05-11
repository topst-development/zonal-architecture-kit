#!/usr/bin/env python3

import argparse
import json
import socket
import sys
import threading
import time
from typing import Any, Dict, List, Optional

import cv2
import numpy as np


def parse_args():
    parser = argparse.ArgumentParser(
        description="Bridge video to topst-nn-server and forward GUI-friendly JSON to Qt clients."
    )
    parser.add_argument("--board-host", default="192.168.0.100", help="topst-nn-server host")
    parser.add_argument("--video-port", type=int, default=9999, help="topst-nn-server video TCP port")
    parser.add_argument("--result-port", type=int, default=9998, help="topst-nn-server JSON result port")
    parser.add_argument("--qt-host", default="0.0.0.0", help="Bridge bind host for Qt clients")
    parser.add_argument("--qt-port", type=int, default=10000, help="Bridge bind port for Qt clients")
    parser.add_argument("--video", default="output.mp4", help="Input video path")
    parser.add_argument("--width", type=int, default=1280, help="Frame width sent to inference app")
    parser.add_argument("--height", type=int, default=720, help="Frame height sent to inference app")
    parser.add_argument("--homography", default=None, help="Path to 3x3 homography JSON file")
    parser.add_argument("--bev-width", type=int, default=800, help="BEV canvas width")
    parser.add_argument("--bev-height", type=int, default=480, help="BEV canvas height")
    parser.add_argument("--lane-smoothing", type=float, default=0.7, help="EMA factor for lane stabilization (0-1)")
    parser.add_argument("--lane-fit-degree", type=int, default=2, help="Polynomial degree for lane curve fitting")
    parser.add_argument("--lane-fit-samples", type=int, default=24, help="Number of points generated from fitted lane curves")
    parser.add_argument("--lane-fit-min-points", type=int, default=4, help="Minimum input points required before fitting")
    parser.add_argument("--lane-hold-frames", type=int, default=5, help="Frames to keep previously detected lanes when current detection is weak")
    parser.add_argument("--lane-top-ratio", type=float, default=0.22, help="Normalized top Y ratio for rendered lane length")
    parser.add_argument("--lane-bottom-ratio", type=float, default=0.92, help="Normalized bottom Y ratio for rendered lane length")
    parser.add_argument("--max-objects", type=int, default=5, help="Maximum number of objects forwarded to Qt")
    parser.add_argument("--object-track-distance", type=float, default=60.0, help="Maximum BEV pixel distance for matching detections to existing tracks")
    parser.add_argument("--object-track-confirm", type=int, default=2, help="Frames required before a tracked object is published")
    parser.add_argument("--object-track-miss", type=int, default=4, help="Frames to keep a tracked object alive without detections")
    parser.add_argument("--object-track-alpha", type=float, default=0.7, help="EMA factor for tracked object position smoothing (0-1)")
    parser.add_argument("--loop", action="store_true", help="Loop the video")
    parser.add_argument("--fps", type=float, default=None, help="Override send FPS")
    parser.add_argument("--no-throttle", action="store_true", help="Send frames as fast as possible")
    parser.add_argument(
        "--snap-fallback-nearest",
        action="store_true",
        help="If object BEV x is outside all lane corridors, assign the nearest corridor (within --snap-fallback-max-px).",
    )
    parser.add_argument(
        "--snap-fallback-max-px",
        type=float,
        default=100.0,
        help="Max lateral distance (px) from corridor strip [x_min,x_max] to accept --snap-fallback-nearest (default: 100).",
    )
    parser.add_argument(
        "--lane-ui-offset",
        type=int,
        default=0,
        help="Added to corridor lane_id (0..2) before mapping to Qt lane_status[0..4]. Use e.g. +1/-1 if UI L1/L2 labels disagree with road lanes.",
    )
    return parser.parse_args()


class QtBroadcastServer:
    def __init__(self, host: str, port: int):
        self.host = host
        self.port = port
        self.server_sock: Optional[socket.socket] = None
        self.clients: List[socket.socket] = []
        self.lock = threading.Lock()
        self.stop_event = threading.Event()
        self.thread: Optional[threading.Thread] = None

    def start(self):
        self.server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_sock.bind((self.host, self.port))
        self.server_sock.listen(5)
        self.thread = threading.Thread(target=self._accept_loop, daemon=True)
        self.thread.start()
        print(f"[bridge] Qt broadcast server listening on {self.host}:{self.port}")

    def _accept_loop(self):
        assert self.server_sock is not None
        while not self.stop_event.is_set():
            try:
                client, addr = self.server_sock.accept()
            except OSError:
                break
            with self.lock:
                self.clients.append(client)
            print(f"[bridge] Qt client connected: {addr[0]}:{addr[1]}")

    def broadcast(self, payload: Dict[str, Any]):
        message = (json.dumps(payload, separators=(",", ":")) + "\n").encode("utf-8")
        stale: List[socket.socket] = []
        with self.lock:
            for client in self.clients:
                try:
                    client.sendall(message)
                except OSError:
                    stale.append(client)
            for client in stale:
                try:
                    client.close()
                except OSError:
                    pass
                if client in self.clients:
                    self.clients.remove(client)

    def stop(self):
        self.stop_event.set()
        if self.server_sock is not None:
            try:
                self.server_sock.close()
            except OSError:
                pass
        with self.lock:
            for client in self.clients:
                try:
                    client.close()
                except OSError:
                    pass
            self.clients.clear()


def connect_with_retry(host: str, port: int, stop_event: threading.Event, label: str) -> socket.socket:
    while not stop_event.is_set():
        try:
            return connect(host, port)
        except OSError as exc:
            print(f"[bridge] waiting for {label} {host}:{port} ({exc})")
            time.sleep(1.0)
    raise RuntimeError(f"stopped while waiting for {label} connection")


class BridgeState:
    def __init__(self):
        self.last_result: Dict[str, Any] = {}
        self.smoothed_lanes: List[List[Dict[str, int]]] = [[], [], [], []]
        self.smoothed_bev_lanes: List[List[Dict[str, int]]] = [[], [], [], []]
        self.held_lanes: List[List[Dict[str, int]]] = [[], [], [], []]
        self.held_bev_lanes: List[List[Dict[str, int]]] = [[], [], [], []]
        self.lane_hold_counts: List[int] = [0, 0, 0, 0]
        self.bev_lane_hold_counts: List[int] = [0, 0, 0, 0]
        self.object_tracks: List[Dict[str, Any]] = []
        self.next_track_id = 1
        self.lock = threading.Lock()

    def update_result(self, payload: Dict[str, Any]):
        with self.lock:
            self.last_result = payload

    def get_result(self) -> Dict[str, Any]:
        with self.lock:
            return dict(self.last_result)

    def smooth_lanes(
        self,
        lanes: List[List[Dict[str, int]]],
        bev_lanes: List[List[Dict[str, int]]],
        alpha: float,
    ) -> tuple[List[List[Dict[str, int]]], List[List[Dict[str, int]]]]:
        with self.lock:
            self.smoothed_lanes = _smooth_lane_set(self.smoothed_lanes, lanes, alpha)
            self.smoothed_bev_lanes = _smooth_lane_set(self.smoothed_bev_lanes, bev_lanes, alpha)
            return self.smoothed_lanes, self.smoothed_bev_lanes

    def hold_lanes(
        self,
        lanes: List[List[Dict[str, int]]],
        bev_lanes: List[List[Dict[str, int]]],
        hold_frames: int,
    ) -> tuple[List[List[Dict[str, int]]], List[List[Dict[str, int]]]]:
        with self.lock:
            lanes = _apply_lane_hold(
                self.held_lanes,
                self.lane_hold_counts,
                lanes,
                hold_frames,
            )
            bev_lanes = _apply_lane_hold(
                self.held_bev_lanes,
                self.bev_lane_hold_counts,
                bev_lanes,
                hold_frames,
            )
            self.held_lanes = [[dict(point) for point in lane] for lane in lanes]
            self.held_bev_lanes = [[dict(point) for point in lane] for lane in bev_lanes]
            return lanes, bev_lanes

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
            matches: List[tuple[int, int, float]] = []

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
                track["raw_bev_x"] = float(det.get("raw_bev_x", det.get("bev_x", 0.0)))
                track["raw_bev_y"] = float(det.get("raw_bev_y", det.get("bev_y", 0.0)))
                track["lane_id"] = int(det.get("lane_id", track.get("lane_id", -1)))
                track["lateral_offset"] = float(det.get("lateral_offset", 0.0))
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
                        "raw_bev_x": float(det.get("raw_bev_x", det.get("bev_x", 0.0))),
                        "raw_bev_y": float(det.get("raw_bev_y", det.get("bev_y", 0.0))),
                        "lane_id": int(det.get("lane_id", -1)),
                        "lateral_offset": float(det.get("lateral_offset", 0.0)),
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
                    "raw_bev_x": float(track.get("raw_bev_x", track["bev_x"])),
                    "raw_bev_y": float(track.get("raw_bev_y", track["bev_y"])),
                    "lane_id": int(track.get("lane_id", -1)),
                    "lateral_offset": float(track.get("lateral_offset", 0.0)),
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


def connect(host: str, port: int) -> socket.socket:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    return sock


def open_video(path: str):
    cap = cv2.VideoCapture(path)
    if not cap.isOpened():
        raise RuntimeError(f"failed to open video: {path}")
    return cap


def object_to_lane_index(obj: Dict[str, Any], frame_width: int) -> int:
    x_min = float(obj.get("x_min", 0.0))
    x_max = float(obj.get("x_max", 0.0))
    center_x = 0.5 * (x_min + x_max)
    lane_index = int(center_x * 5.0 / max(frame_width, 1))
    return max(0, min(4, lane_index))


def load_homography(path: Optional[str]) -> Optional[np.ndarray]:
    if not path:
        return None

    payload = json.loads(open(path, "r", encoding="utf-8").read())
    matrix = payload.get("homography", payload)
    arr = np.asarray(matrix, dtype=np.float32)
    if arr.shape != (3, 3):
        raise ValueError(f"invalid homography shape: {arr.shape}")
    return arr


def scale_points(
    points: List[Dict[str, Any]],
    src_w: int,
    src_h: int,
    dst_w: int,
    dst_h: int,
) -> List[Dict[str, float]]:
    sx = float(dst_w) / max(src_w, 1)
    sy = float(dst_h) / max(src_h, 1)
    return [
        {"x": float(p.get("x", 0.0)) * sx, "y": float(p.get("y", 0.0)) * sy}
        for p in points
    ]


def transform_points(
    points: List[Dict[str, Any]],
    homography: Optional[np.ndarray],
    src_w: int,
    src_h: int,
    dst_w: int,
    dst_h: int,
) -> List[Dict[str, int]]:
    scaled_points = scale_points(points, src_w, src_h, dst_w, dst_h)
    if homography is None or not points:
        return [{"x": int(round(p["x"])), "y": int(round(p["y"]))} for p in scaled_points]

    src = np.array(
        [[[p["x"], p["y"]] for p in scaled_points]],
        dtype=np.float32,
    )
    dst = cv2.perspectiveTransform(src, homography)[0]
    return [{"x": int(round(x)), "y": int(round(y))} for x, y in dst]


def transform_object(
    obj: Dict[str, Any],
    homography: Optional[np.ndarray],
    src_w: int,
    src_h: int,
    dst_w: int,
    dst_h: int,
) -> Dict[str, Any]:
    bev_obj = dict(obj)
    x_min = float(obj.get("x_min", 0.0))
    x_max = float(obj.get("x_max", 0.0))
    y_max = float(obj.get("y_max", 0.0))
    contact = [{"x": 0.5 * (x_min + x_max), "y": y_max}]
    bev_contact = transform_points(contact, homography, src_w, src_h, dst_w, dst_h)[0]
    bev_obj["bev_x"] = bev_contact["x"]
    bev_obj["bev_y"] = bev_contact["y"]
    return bev_obj


def _smooth_lane_points(
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


def _smooth_lane_set(
    previous_set: List[List[Dict[str, int]]],
    current_set: List[List[Dict[str, int]]],
    alpha: float,
) -> List[List[Dict[str, int]]]:
    smoothed_set: List[List[Dict[str, int]]] = [[], [], [], []]
    for lane_idx in range(4):
        previous = previous_set[lane_idx] if lane_idx < len(previous_set) else []
        current = current_set[lane_idx] if lane_idx < len(current_set) else []
        smoothed_set[lane_idx] = _smooth_lane_points(previous, current, alpha)
    return smoothed_set


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


def _apply_lane_hold(
    previous_set: List[List[Dict[str, int]]],
    hold_counts: List[int],
    current_set: List[List[Dict[str, int]]],
    hold_frames: int,
) -> List[List[Dict[str, int]]]:
    output: List[List[Dict[str, int]]] = [[], [], [], []]
    for lane_idx in range(4):
        current = current_set[lane_idx] if lane_idx < len(current_set) else []
        previous = previous_set[lane_idx] if lane_idx < len(previous_set) else []

        if current:
            output[lane_idx] = [dict(point) for point in current]
            hold_counts[lane_idx] = hold_frames
        elif previous and hold_counts[lane_idx] > 0:
            output[lane_idx] = [dict(point) for point in previous]
            hold_counts[lane_idx] -= 1
        else:
            output[lane_idx] = []
            hold_counts[lane_idx] = 0
    return output


def _interpolate_lane_x(points: List[Dict[str, int]], target_y: float) -> Optional[float]:
    if len(points) < 2:
        return None

    sorted_points = sorted(points, key=lambda p: float(p["y"]), reverse=True)
    for idx in range(len(sorted_points) - 1):
        p0 = sorted_points[idx]
        p1 = sorted_points[idx + 1]
        y0 = float(p0["y"])
        y1 = float(p1["y"])
        if abs(y0 - y1) < 1e-6:
            continue
        top_y = min(y0, y1)
        bottom_y = max(y0, y1)
        if top_y <= target_y <= bottom_y:
            t = (target_y - y0) / (y1 - y0)
            return float(p0["x"]) + t * (float(p1["x"]) - float(p0["x"]))

    nearest = min(sorted_points, key=lambda p: abs(float(p["y"]) - target_y))
    return float(nearest["x"])


def _corridor_center_at_y(lanes: List[List[Dict[str, int]]], target_y: float) -> List[Dict[str, float]]:
    lane_xs: List[Optional[float]] = [
        _interpolate_lane_x(lane_points, target_y) for lane_points in lanes[:4]
    ]
    corridors: List[Dict[str, float]] = []
    for idx in range(len(lane_xs) - 1):
        left_x = lane_xs[idx]
        right_x = lane_xs[idx + 1]
        if left_x is None or right_x is None:
            continue
        x_min = min(left_x, right_x)
        x_max = max(left_x, right_x)
        corridors.append(
            {
                "lane_id": float(idx),
                "x_min": x_min,
                "x_max": x_max,
                "center_x": 0.5 * (left_x + right_x),
            }
        )
    return corridors


def _lateral_dist_to_corridor(raw_x: float, corridor: Dict[str, float]) -> float:
    lo = float(corridor["x_min"])
    hi = float(corridor["x_max"])
    if lo <= raw_x <= hi:
        return 0.0
    return min(abs(raw_x - lo), abs(raw_x - hi))


def snap_objects_to_lanes(
    bev_objects: List[Dict[str, Any]],
    lanes: List[List[Dict[str, int]]],
    _bev_width: int,
    fallback_nearest: bool = False,
    fallback_max_lateral_px: float = 100.0,
) -> List[Dict[str, Any]]:
    snapped_objects: List[Dict[str, Any]] = []
    lane_centers: List[List[Dict[str, int]]] = [lane for lane in lanes[:4]]

    for obj in bev_objects:
        snapped = dict(obj)
        raw_x = float(obj.get("bev_x", 0.0))
        raw_y = float(obj.get("bev_y", 0.0))
        snapped["raw_bev_x"] = raw_x
        snapped["raw_bev_y"] = raw_y

        corridors = _corridor_center_at_y(lane_centers, raw_y)
        inside_corridor: Optional[Dict[str, float]] = None
        for corridor in corridors:
            if corridor["x_min"] <= raw_x <= corridor["x_max"]:
                inside_corridor = corridor
                break

        chosen: Optional[Dict[str, float]] = inside_corridor
        if chosen is None and fallback_nearest and corridors:
            best_c: Optional[Dict[str, float]] = None
            best_d = float("inf")
            for corridor in corridors:
                d = _lateral_dist_to_corridor(raw_x, corridor)
                if d < best_d:
                    best_d = d
                    best_c = corridor
            if best_c is not None and best_d <= fallback_max_lateral_px:
                chosen = best_c
                snapped["snap_fallback"] = True
                snapped["snap_lateral_miss_px"] = float(best_d)

        if chosen is not None:
            snapped["lane_id"] = int(chosen["lane_id"])
            snapped["bev_x"] = int(round(chosen["center_x"]))
            snapped["bev_y"] = int(round(raw_y))
            snapped["lateral_offset"] = float(raw_x - chosen["center_x"])
        else:
            continue

        snapped_objects.append(snapped)

    return snapped_objects


def build_qt_payload(
    result: Dict[str, Any],
    homography: Optional[np.ndarray],
    bev_width: int,
    bev_height: int,
    source_width: int,
    source_height: int,
    state: BridgeState,
    lane_smoothing: float,
    lane_fit_degree: int,
    lane_fit_samples: int,
    lane_fit_min_points: int,
    lane_hold_frames: int,
    lane_top_ratio: float,
    lane_bottom_ratio: float,
    max_objects: int,
    object_track_distance: float,
    object_track_confirm: int,
    object_track_miss: int,
    object_track_alpha: float,
    lane_ui_offset: int = 0,
    snap_fallback_nearest: bool = False,
    snap_fallback_max_px: float = 100.0,
) -> Dict[str, Any]:
    payload_width = int(result.get("width", 800))
    payload_height = int(result.get("height", 480))
    src_width = source_width
    src_height = source_height
    top_y = float(src_height) * lane_top_ratio
    bottom_y = float(src_height) * lane_bottom_ratio
    bev_top_y = float(bev_height) * lane_top_ratio
    bev_bottom_y = float(bev_height) * lane_bottom_ratio
    lanes = result.get("lanes", []) or []
    objects = result.get("objects", []) or []
    perf = result.get("perf", {}) or {}

    lane_groups: List[List[Dict[str, Any]]] = [[], [], [], []]
    bev_lane_groups: List[List[Dict[str, Any]]] = [[], [], [], []]
    for lane in lanes:
        lane_id = int(lane.get("lane_id", -1))
        if 0 <= lane_id < 4:
            lane_groups[lane_id] = transform_points(
                lane.get("points", []) or [],
                None,
                payload_width,
                payload_height,
                src_width,
                src_height,
            )
            bev_lane_groups[lane_id] = transform_points(
                lane.get("points", []) or [],
                homography,
                payload_width,
                payload_height,
                src_width,
                src_height,
            )

    lane_groups = fit_lane_set(
        lane_groups,
        lane_fit_degree,
        lane_fit_samples,
        lane_fit_min_points,
        top_y,
        bottom_y,
    )
    bev_lane_groups = fit_lane_set(
        bev_lane_groups,
        lane_fit_degree,
        lane_fit_samples,
        lane_fit_min_points,
        bev_top_y,
        bev_bottom_y,
    )
    lane_groups, bev_lane_groups = state.hold_lanes(lane_groups, bev_lane_groups, lane_hold_frames)

    bev_objects = [
        transform_object(obj, homography, payload_width, payload_height, src_width, src_height)
        for obj in objects
    ]
    if homography is not None:
        bev_objects.sort(key=lambda obj: float(obj.get("bev_y", -1e9)), reverse=True)
    else:
        bev_objects.sort(key=lambda obj: float(obj.get("y_max", -1e9)), reverse=True)
    if max_objects > 0:
        bev_objects = bev_objects[:max_objects]

    limited_objects = objects[:max_objects] if max_objects > 0 else objects
    qt_width = bev_width if homography is not None else src_width
    qt_height = bev_height if homography is not None else src_height
    lane_groups, bev_lane_groups = state.smooth_lanes(lane_groups, bev_lane_groups, lane_smoothing)
    qt_lanes = bev_lane_groups if homography is not None else lane_groups

    if homography is not None:
        bev_objects = snap_objects_to_lanes(
            bev_objects,
            qt_lanes,
            bev_width,
            fallback_nearest=snap_fallback_nearest,
            fallback_max_lateral_px=snap_fallback_max_px,
        )
        bev_objects = state.update_object_tracks(
            bev_objects,
            object_track_distance,
            object_track_confirm,
            object_track_miss,
            object_track_alpha,
            max_objects,
        )

    lane_status = [False, False, False, False, False]
    if homography is None:
        for obj in limited_objects:
            x_min = float(obj.get("x_min", 0.0)) * src_width / max(payload_width, 1)
            x_max = float(obj.get("x_max", 0.0)) * src_width / max(payload_width, 1)
            scaled_obj = dict(obj)
            scaled_obj["x_min"] = x_min
            scaled_obj["x_max"] = x_max
            lane_status[object_to_lane_index(scaled_obj, src_width)] = True
    else:
        for obj in bev_objects:
            lane_index = int(obj.get("lane_id", -1))
            if 0 <= lane_index <= 4:
                ui_slot = max(0, min(4, lane_index + lane_ui_offset))
                lane_status[ui_slot] = True
            else:
                center_x = float(obj.get("bev_x", 0.0))
                q = int(center_x * 5.0 / max(bev_width, 1))
                q = max(0, min(4, q))
                lane_status[q] = True

    return {
        "frame_index": int(result.get("frame_index", 0)),
        "width": qt_width,
        "height": qt_height,
        "lanes": qt_lanes,
        "bev_width": bev_width,
        "bev_height": bev_height,
        "bev_lanes": bev_lane_groups,
        "objects": limited_objects,
        "bev_objects": bev_objects,
        "lane_status": lane_status,
        "perf": {
            "fps": float(perf.get("fps", 0.0)),
            "cpu": int(perf.get("cpu", 0)),
            "mem": int(perf.get("mem", 0)),
        },
        "speed_kmh": float(result.get("speed_kmh", perf.get("fps", 0.0))),
        "steering": float(result.get("steering", 0.0)),
    }


def result_receiver(
    board_host: str,
    result_port: int,
    qt_server: QtBroadcastServer,
    state: BridgeState,
    stop_event: threading.Event,
    homography: Optional[np.ndarray],
    bev_width: int,
    bev_height: int,
    source_width: int,
    source_height: int,
    lane_smoothing: float,
    lane_fit_degree: int,
    lane_fit_samples: int,
    lane_fit_min_points: int,
    lane_hold_frames: int,
    lane_top_ratio: float,
    lane_bottom_ratio: float,
    max_objects: int,
    object_track_distance: float,
    object_track_confirm: int,
    object_track_miss: int,
    object_track_alpha: float,
    lane_ui_offset: int = 0,
    snap_fallback_nearest: bool = False,
    snap_fallback_max_px: float = 100.0,
):
    while not stop_event.is_set():
        sock = connect_with_retry(board_host, result_port, stop_event, "result")
        buffer = b""
        print(f"[bridge] connected to inference JSON {board_host}:{result_port}")
        try:
            while not stop_event.is_set():
                data = sock.recv(4096)
                if not data:
                    print("[bridge] result socket disconnected, retrying")
                    break
                buffer += data
                while b"\n" in buffer:
                    line, buffer = buffer.split(b"\n", 1)
                    line = line.strip()
                    if not line:
                        continue
                    try:
                        payload = json.loads(line.decode("utf-8", errors="replace"))
                    except json.JSONDecodeError:
                        print("[bridge] dropped malformed JSON line")
                        continue
                    state.update_result(payload)
                    qt_payload = build_qt_payload(
                        payload,
                        homography,
                        bev_width,
                        bev_height,
                        source_width,
                        source_height,
                        state,
                        lane_smoothing,
                        lane_fit_degree,
                        lane_fit_samples,
                        lane_fit_min_points,
                        lane_hold_frames,
                        lane_top_ratio,
                        lane_bottom_ratio,
                        max_objects,
                        object_track_distance,
                        object_track_confirm,
                        object_track_miss,
                        object_track_alpha,
                        lane_ui_offset,
                        snap_fallback_nearest,
                        snap_fallback_max_px,
                    )
                    qt_server.broadcast(qt_payload)
        except OSError as exc:
            print(f"[bridge] result receiver error: {exc}")
        finally:
            try:
                sock.close()
            except OSError:
                pass
        time.sleep(0.5)


def video_sender(args, stop_event: threading.Event):
    cap = open_video(args.video)
    fps = args.fps if args.fps is not None else cap.get(cv2.CAP_PROP_FPS)
    if fps <= 0.0:
        fps = 30.0
    frame_interval = 0.0 if args.no_throttle else (1.0 / fps)

    try:
        while not stop_event.is_set():
            sock = connect_with_retry(args.board_host, args.video_port, stop_event, "video")
            print(
                f"[bridge] sending {args.video} to {args.board_host}:{args.video_port} as {args.width}x{args.height} RGB888"
            )
            try:
                while not stop_event.is_set():
                    started = time.time()
                    ret, frame = cap.read()
                    if not ret:
                        if args.loop:
                            cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                            continue
                        print("[bridge] video finished")
                        return
                    resized = cv2.resize(frame, (args.width, args.height), interpolation=cv2.INTER_AREA)
                    rgb = cv2.cvtColor(resized, cv2.COLOR_BGR2RGB)
                    sock.sendall(rgb.tobytes())
                    elapsed = time.time() - started
                    sleep_time = frame_interval - elapsed
                    if frame_interval > 0.0 and sleep_time > 0:
                        time.sleep(sleep_time)
            except OSError as exc:
                print(f"[bridge] video sender error: {exc}")
                time.sleep(0.5)
            finally:
                try:
                    sock.close()
                except OSError:
                    pass
    finally:
        cap.release()


def main():
    args = parse_args()
    homography = load_homography(args.homography)
    if homography is not None:
        print(f"[bridge] loaded homography from {args.homography}")
    qt_server = QtBroadcastServer(args.qt_host, args.qt_port)
    qt_server.start()
    state = BridgeState()
    stop_event = threading.Event()

    receiver = threading.Thread(
        target=result_receiver,
        args=(
            args.board_host,
            args.result_port,
            qt_server,
            state,
            stop_event,
            homography,
            args.bev_width,
            args.bev_height,
            args.width,
            args.height,
            args.lane_smoothing,
            args.lane_fit_degree,
            args.lane_fit_samples,
            args.lane_fit_min_points,
            args.lane_hold_frames,
            args.lane_top_ratio,
            args.lane_bottom_ratio,
            args.max_objects,
            args.object_track_distance,
            args.object_track_confirm,
            args.object_track_miss,
            args.object_track_alpha,
            args.lane_ui_offset,
            args.snap_fallback_nearest,
            args.snap_fallback_max_px,
        ),
        daemon=True,
    )
    receiver.start()

    try:
        video_sender(args, stop_event)
    except KeyboardInterrupt:
        pass
    finally:
        stop_event.set()
        qt_server.stop()

    return 0


if __name__ == "__main__":
    sys.exit(main())
