#!/usr/bin/env python3

import argparse
import json
import socket
import threading
import time
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import cv2
import numpy as np
import perception_bridge as bridge_pipeline


WINDOW_NAME = "bev_calibration"
SOURCE_COLOR = (0, 255, 255)
DEST_COLOR = (255, 200, 0)
LANE_COLORS = [
    (255, 80, 80),
    (80, 220, 120),
    (80, 140, 255),
    (0, 220, 255),
]


def parse_args():
    parser = argparse.ArgumentParser(
        description="Interactive BEV homography calibration using live inference JSON."
    )
    parser.add_argument("--board-host", default="192.168.0.100", help="topst-nn-server host")
    parser.add_argument("--video-port", type=int, default=9999, help="topst-nn-server video TCP port")
    parser.add_argument("--result-port", type=int, default=9998, help="topst-nn-server JSON result port")
    parser.add_argument("--video", default="output.mp4", help="Reference video path")
    parser.add_argument("--qt-host", default="0.0.0.0", help="Bind host for Qt preview clients")
    parser.add_argument("--qt-port", type=int, default=10000, help="Bind port for Qt preview clients")
    parser.add_argument("--frame-width", type=int, default=1280, help="Reference video display width")
    parser.add_argument("--frame-height", type=int, default=720, help="Reference video display height")
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
    parser.add_argument("--qt-left-margin", type=int, default=10, help="Qt lane canvas left margin")
    parser.add_argument("--qt-right-margin", type=int, default=10, help="Qt lane canvas right margin")
    parser.add_argument("--qt-top-margin", type=int, default=10, help="Qt lane canvas top margin")
    parser.add_argument("--qt-bottom-margin", type=int, default=120, help="Qt lane canvas bottom margin")
    parser.add_argument("--qt-lane-offset-x", type=int, default=0, help="Additional Qt lane X offset")
    parser.add_argument("--qt-lane-offset-y", type=int, default=0, help="Additional Qt lane Y offset")
    parser.add_argument("--fps", type=float, default=15.0, help="Reference video playback FPS")
    parser.add_argument("--loop", action="store_true", help="Loop the video while sending to inference app")
    parser.add_argument("--points", default=None, help="Optional JSON file with source_points/dest_points")
    parser.add_argument("--output", default="bev_homography.json", help="Output JSON path")
    return parser.parse_args()


class SharedResult:
    def __init__(self):
        self.payload: Dict[str, Any] = {}
        self.lock = threading.Lock()

    def update(self, payload: Dict[str, Any]):
        with self.lock:
            self.payload = payload

    def get(self) -> Dict[str, Any]:
        with self.lock:
            return dict(self.payload)


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
        print(f"[calib] Qt preview server listening on {self.host}:{self.port}")

    def _accept_loop(self):
        assert self.server_sock is not None
        while not self.stop_event.is_set():
            try:
                client, addr = self.server_sock.accept()
            except OSError:
                break
            with self.lock:
                self.clients.append(client)
            print(f"[calib] Qt client connected: {addr[0]}:{addr[1]}")

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


class PointEditor:
    def __init__(self, src_points: List[List[int]], dst_points: List[List[int]], src_w: int):
        self.src_points = src_points
        self.dst_points = dst_points
        self.src_w = src_w
        self.active: Optional[Tuple[str, int]] = None

    def on_mouse(self, event, x, y, _flags, _userdata):
        if event == cv2.EVENT_LBUTTONDOWN:
            self.active = self._pick_point(x, y)
        elif event == cv2.EVENT_MOUSEMOVE and self.active is not None:
            self._move_point(self.active, x, y)
        elif event == cv2.EVENT_LBUTTONUP:
            self.active = None

    def _pick_point(self, x: int, y: int) -> Optional[Tuple[str, int]]:
        candidates: List[Tuple[str, int, float]] = []
        for idx, (px, py) in enumerate(self.src_points):
            dist = (px - x) ** 2 + (py - y) ** 2
            candidates.append(("src", idx, dist))
        for idx, (px, py) in enumerate(self.dst_points):
            dx = self.src_w + px - x
            dist = dx ** 2 + (py - y) ** 2
            candidates.append(("dst", idx, dist))
        kind, idx, dist = min(candidates, key=lambda item: item[2])
        if dist <= 20 ** 2:
            return (kind, idx)
        return None

    def _move_point(self, active: Tuple[str, int], x: int, y: int):
        kind, idx = active
        if kind == "src":
            self.src_points[idx] = [max(0, x), max(0, y)]
        else:
            self.dst_points[idx] = [max(0, x - self.src_w), max(0, y)]


def default_points(frame_w: int, frame_h: int, bev_w: int, bev_h: int):
    src = [
        [int(frame_w * 0.32), int(frame_h * 0.84)],
        [int(frame_w * 0.68), int(frame_h * 0.84)],
        [int(frame_w * 0.45), int(frame_h * 0.55)],
        [int(frame_w * 0.55), int(frame_h * 0.55)],
    ]
    dst = [
        [int(bev_w * 0.25), int(bev_h * 0.92)],
        [int(bev_w * 0.75), int(bev_h * 0.92)],
        [int(bev_w * 0.25), int(bev_h * 0.08)],
        [int(bev_w * 0.75), int(bev_h * 0.08)],
    ]
    return src, dst


def load_points(path: Optional[str], frame_w: int, frame_h: int, bev_w: int, bev_h: int):
    if not path:
        return default_points(frame_w, frame_h, bev_w, bev_h)
    payload = json.loads(Path(path).read_text())
    src = payload.get("source_points")
    dst = payload.get("dest_points")
    if not src or not dst or len(src) < 4 or len(dst) < 4:
        raise ValueError("points file must contain source_points and dest_points with 4 entries each")
    return src[:4], dst[:4]


def connect_json(host: str, port: int):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    return sock


def json_receiver(host: str, port: int, shared: SharedResult, stop_event: threading.Event):
    sock = connect_json(host, port)
    buffer = b""
    print(f"[calib] connected to {host}:{port}")
    try:
        while not stop_event.is_set():
            data = sock.recv(4096)
            if not data:
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
                    continue
                shared.update(payload)
    finally:
        sock.close()


def video_sender(
    host: str,
    port: int,
    video_path: str,
    width: int,
    height: int,
    fps: float,
    loop: bool,
    stop_event: threading.Event,
):
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        raise RuntimeError(f"failed to open video for sending: {video_path}")

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    print(f"[calib] sending video to {host}:{port}")

    interval = 1.0 / max(fps, 1.0)
    try:
        while not stop_event.is_set():
            started = time.time()
            ok, frame = cap.read()
            if not ok:
                if loop:
                    cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                    continue
                break
            resized = cv2.resize(frame, (width, height), interpolation=cv2.INTER_AREA)
            rgb = cv2.cvtColor(resized, cv2.COLOR_BGR2RGB)
            sock.sendall(rgb.tobytes())
            elapsed = time.time() - started
            sleep_time = interval - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)
    finally:
        cap.release()
        sock.close()


def draw_points(canvas: np.ndarray, points: List[List[int]], offset_x: int, color: Tuple[int, int, int], label: str):
    for idx, (x, y) in enumerate(points):
        px = x + offset_x
        cv2.circle(canvas, (px, y), 7, color, -1)
        cv2.putText(
            canvas,
            f"{label}{idx}",
            (px + 10, y - 8),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.55,
            color,
            2,
            cv2.LINE_AA,
        )
    if len(points) >= 4:
        poly = np.array([[x + offset_x, y] for x, y in points], dtype=np.int32)
        cv2.polylines(canvas, [poly], True, color, 2, cv2.LINE_AA)


def scale_point(x: float, y: float, src_w: int, src_h: int, dst_w: int, dst_h: int):
    sx = float(dst_w) / max(src_w, 1)
    sy = float(dst_h) / max(src_h, 1)
    return x * sx, y * sy


def overlay_inference(frame: np.ndarray, payload: Dict[str, Any], dst_w: int, dst_h: int):
    lanes = payload.get("lanes", [])
    objects = payload.get("objects", [])
    src_w = int(payload.get("width", dst_w))
    src_h = int(payload.get("height", dst_h))

    for lane in lanes:
        lane_id = int(lane.get("lane_id", 0))
        pts = lane.get("points", [])
        if len(pts) < 2:
            continue
        color = LANE_COLORS[lane_id % len(LANE_COLORS)]
        scaled = [scale_point(float(p["x"]), float(p["y"]), src_w, src_h, dst_w, dst_h) for p in pts]
        poly = np.array([[int(x), int(y)] for x, y in scaled], dtype=np.int32)
        cv2.polylines(frame, [poly], False, color, 3, cv2.LINE_AA)

    for obj in objects:
        x1, y1 = scale_point(
            float(obj.get("x_min", 0.0)),
            float(obj.get("y_min", 0.0)),
            src_w,
            src_h,
            dst_w,
            dst_h,
        )
        x2, y2 = scale_point(
            float(obj.get("x_max", 0.0)),
            float(obj.get("y_max", 0.0)),
            src_w,
            src_h,
            dst_w,
            dst_h,
        )
        cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), (255, 255, 255), 2)


def compute_homography(src_points: List[List[int]], dst_points: List[List[int]]) -> np.ndarray:
    src = np.asarray(src_points, dtype=np.float32)
    dst = np.asarray(dst_points, dtype=np.float32)
    return cv2.getPerspectiveTransform(src, dst)


def transform_lane_points(payload: Dict[str, Any], homography: np.ndarray, dst_w: int, dst_h: int):
    bev_lanes = []
    src_w = int(payload.get("width", dst_w))
    src_h = int(payload.get("height", dst_h))
    for lane in payload.get("lanes", []):
        pts = lane.get("points", [])
        if not pts:
            bev_lanes.append((int(lane.get("lane_id", 0)), []))
            continue
        scaled_pts = [
            scale_point(float(p["x"]), float(p["y"]), src_w, src_h, dst_w, dst_h)
            for p in pts
        ]
        src = np.array([[list(pt) for pt in scaled_pts]], dtype=np.float32)
        dst = cv2.perspectiveTransform(src, homography)[0]
        bev_pts = [(int(round(x)), int(round(y))) for x, y in dst]
        bev_lanes.append((int(lane.get("lane_id", 0)), bev_pts))
    return bev_lanes


def transform_objects(payload: Dict[str, Any], homography: np.ndarray, dst_w: int, dst_h: int):
    bev_objects = []
    src_w = int(payload.get("width", dst_w))
    src_h = int(payload.get("height", dst_h))
    for obj in payload.get("objects", []):
        center_x = 0.5 * (float(obj.get("x_min", 0.0)) + float(obj.get("x_max", 0.0)))
        bottom_y = float(obj.get("y_max", 0.0))
        scaled_x, scaled_y = scale_point(center_x, bottom_y, src_w, src_h, dst_w, dst_h)
        src = np.array([[[scaled_x, scaled_y]]], dtype=np.float32)
        dst = cv2.perspectiveTransform(src, homography)[0][0]
        bev_objects.append((int(round(dst[0])), int(round(dst[1]))))
    return bev_objects


def draw_bev(bev_w: int, bev_h: int, payload: Dict[str, Any], homography: np.ndarray, src_w: int, src_h: int):
    canvas = np.zeros((bev_h, bev_w, 3), dtype=np.uint8)
    canvas[:] = (12, 18, 24)

    bev_lanes = transform_lane_points(payload, homography, src_w, src_h)
    for lane_id, pts in bev_lanes:
        if len(pts) < 2:
            continue
        color = LANE_COLORS[lane_id % len(LANE_COLORS)]
        poly = np.array(pts, dtype=np.int32)
        cv2.polylines(canvas, [poly], False, color, 3, cv2.LINE_AA)

    for x, y in transform_objects(payload, homography, src_w, src_h):
        cv2.circle(canvas, (x, y), 8, (255, 255, 255), -1)
        cv2.circle(canvas, (x, y), 13, (255, 160, 80), 2)

    cv2.rectangle(canvas, (bev_w // 2 - 20, bev_h - 40), (bev_w // 2 + 20, bev_h - 8), (80, 180, 255), -1)
    cv2.putText(canvas, "BEV", (18, 28), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (220, 230, 235), 2, cv2.LINE_AA)
    return canvas


def _lane_point_to_canvas(
    point: Dict[str, Any],
    frame_w: int,
    frame_h: int,
    canvas_x: int,
    canvas_y: int,
    canvas_w: int,
    canvas_h: int,
    offset_x: int,
    offset_y: int,
) -> Tuple[int, int]:
    x = int(round(float(point.get("x", 0)) * canvas_w / max(frame_w, 1) + canvas_x + offset_x))
    y = int(round(float(point.get("y", 0)) * canvas_h / max(frame_h, 1) + canvas_y + offset_y))
    return x, y


def draw_qt_payload_bev(
    bev_w: int,
    bev_h: int,
    qt_payload: Dict[str, Any],
    left_margin: int,
    right_margin: int,
    top_margin: int,
    bottom_margin: int,
    lane_offset_x: int,
    lane_offset_y: int,
):
    canvas = np.zeros((bev_h, bev_w, 3), dtype=np.uint8)
    canvas[:] = (12, 18, 24)

    canvas_x = max(0, left_margin)
    canvas_y = max(0, top_margin)
    canvas_w = max(1, bev_w - left_margin - right_margin)
    canvas_h = max(1, bev_h - top_margin - bottom_margin)
    frame_w = int(qt_payload.get("width", bev_w))
    frame_h = int(qt_payload.get("height", bev_h))

    cv2.rectangle(
        canvas,
        (canvas_x, canvas_y),
        (canvas_x + canvas_w, canvas_y + canvas_h),
        (38, 52, 64),
        1,
        cv2.LINE_AA,
    )

    lanes = qt_payload.get("lanes", []) or []
    lane_color = (152, 161, 171)
    left_lane = lanes[1] if len(lanes) > 1 else []
    right_lane = lanes[2] if len(lanes) > 2 else []
    if len(left_lane) > 1 and len(right_lane) > 1:
        lane_fill = []
        for point in left_lane:
            lane_fill.append(
                _lane_point_to_canvas(
                    point,
                    frame_w,
                    frame_h,
                    canvas_x,
                    canvas_y,
                    canvas_w,
                    canvas_h,
                    lane_offset_x,
                    lane_offset_y,
                )
            )
        for point in reversed(right_lane):
            lane_fill.append(
                _lane_point_to_canvas(
                    point,
                    frame_w,
                    frame_h,
                    canvas_x,
                    canvas_y,
                    canvas_w,
                    canvas_h,
                    lane_offset_x,
                    lane_offset_y,
                )
            )
        lane_fill_np = np.array(lane_fill, dtype=np.int32)
        cv2.fillPoly(canvas, [lane_fill_np], (72, 96, 112))

        mid_idx = min(len(left_lane), len(right_lane)) - 1
        if mid_idx >= 2:
            left_head = _lane_point_to_canvas(
                left_lane[mid_idx],
                frame_w,
                frame_h,
                canvas_x,
                canvas_y,
                canvas_w,
                canvas_h,
                lane_offset_x,
                lane_offset_y,
            )
            right_head = _lane_point_to_canvas(
                right_lane[mid_idx],
                frame_w,
                frame_h,
                canvas_x,
                canvas_y,
                canvas_w,
                canvas_h,
                lane_offset_x,
                lane_offset_y,
            )
            left_tail = _lane_point_to_canvas(
                left_lane[mid_idx - 2],
                frame_w,
                frame_h,
                canvas_x,
                canvas_y,
                canvas_w,
                canvas_h,
                lane_offset_x,
                lane_offset_y,
            )
            right_tail = _lane_point_to_canvas(
                right_lane[mid_idx - 2],
                frame_w,
                frame_h,
                canvas_x,
                canvas_y,
                canvas_w,
                canvas_h,
                lane_offset_x,
                lane_offset_y,
            )

            tail = (
                int(round((left_tail[0] + right_tail[0]) * 0.5)),
                int(round((left_tail[1] + right_tail[1]) * 0.5)),
            )
            head = (
                int(round((left_head[0] + right_head[0]) * 0.5)),
                int(round((left_head[1] + right_head[1]) * 0.5)),
            )
            dx = float(head[0] - tail[0])
            dy = float(head[1] - tail[1])
            length = max(1.0, float((dx * dx + dy * dy) ** 0.5))
            ux = dx / length
            uy = dy / length
            px = -uy
            py = ux
            arrow_left = (
                int(round(head[0] - ux * 26 + px * 10)),
                int(round(head[1] - uy * 26 + py * 10)),
            )
            arrow_right = (
                int(round(head[0] - ux * 26 - px * 10)),
                int(round(head[1] - uy * 26 - py * 10)),
            )
            cv2.line(canvas, tail, head, (188, 220, 234), 5, cv2.LINE_AA)
            cv2.fillPoly(
                canvas,
                [np.array([head, arrow_left, arrow_right], dtype=np.int32)],
                (188, 220, 234),
            )

    for lane_idx, points in enumerate(lanes[:4]):
        if len(points) < 2:
            continue
        poly = np.array(
            [
                _lane_point_to_canvas(
                    point,
                    frame_w,
                    frame_h,
                    canvas_x,
                    canvas_y,
                    canvas_w,
                    canvas_h,
                    lane_offset_x,
                    lane_offset_y,
                )
                for point in points
            ],
            dtype=np.int32,
        )
        cv2.polylines(canvas, [poly], False, lane_color, 4, cv2.LINE_AA)

    for obj in qt_payload.get("bev_objects", []) or []:
        x, y = _lane_point_to_canvas(
            {"x": obj.get("bev_x", 0), "y": obj.get("bev_y", 0)},
            frame_w,
            frame_h,
            canvas_x,
            canvas_y,
            canvas_w,
            canvas_h,
            lane_offset_x,
            lane_offset_y,
        )
        cv2.circle(canvas, (x, y), 8, (255, 255, 255), -1)
        cv2.circle(canvas, (x, y), 13, (255, 160, 80), 2)
        lane_id = int(obj.get("lane_id", -1))
        label = f"L{lane_id}" if lane_id >= 0 else "OBJ"
        cv2.putText(
            canvas,
            label,
            (x + 12, y - 10),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.45,
            (230, 236, 240),
            1,
            cv2.LINE_AA,
        )

    lane_status = qt_payload.get("lane_status", []) or []
    active_count = sum(1 for active in lane_status if active)
    cv2.rectangle(canvas, (bev_w // 2 - 20, bev_h - 40), (bev_w // 2 + 20, bev_h - 8), (80, 180, 255), -1)
    cv2.putText(canvas, "QT PREVIEW", (18, 28), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (220, 230, 235), 2, cv2.LINE_AA)
    cv2.putText(canvas, f"ACTIVE OBJ LANES: {active_count}", (18, 56), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (170, 190, 205), 2, cv2.LINE_AA)
    return canvas


def save_homography(output: str, src_points: List[List[int]], dst_points: List[List[int]], homography: np.ndarray, args):
    payload = {
        "video": args.video,
        "source_width": args.frame_width,
        "source_height": args.frame_height,
        "bev_width": args.bev_width,
        "bev_height": args.bev_height,
        "source_points": src_points,
        "dest_points": dst_points,
        "homography": homography.tolist(),
    }
    Path(output).write_text(json.dumps(payload, indent=2))


def main():
    args = parse_args()
    src_points, dst_points = load_points(args.points, args.frame_width, args.frame_height, args.bev_width, args.bev_height)
    shared = SharedResult()
    bridge_state = bridge_pipeline.BridgeState()
    stop_event = threading.Event()
    qt_server = QtBroadcastServer(args.qt_host, args.qt_port)
    qt_server.start()
    receiver = threading.Thread(
        target=json_receiver,
        args=(args.board_host, args.result_port, shared, stop_event),
        daemon=True,
    )
    receiver.start()

    sender = threading.Thread(
        target=video_sender,
        args=(
            args.board_host,
            args.video_port,
            args.video,
            args.frame_width,
            args.frame_height,
            args.fps,
            args.loop,
            stop_event,
        ),
        daemon=True,
    )
    sender.start()

    cap = cv2.VideoCapture(args.video)
    if not cap.isOpened():
        raise RuntimeError(f"failed to open video: {args.video}")

    editor = PointEditor(src_points, dst_points, args.frame_width)
    cv2.namedWindow(WINDOW_NAME, cv2.WINDOW_NORMAL)
    cv2.setMouseCallback(WINDOW_NAME, editor.on_mouse)

    frame_interval = 1.0 / max(args.fps, 1.0)

    try:
        while True:
            started = time.time()
            ok, frame = cap.read()
            if not ok:
                cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                ok, frame = cap.read()
                if not ok:
                    break

            frame = cv2.resize(frame, (args.frame_width, args.frame_height), interpolation=cv2.INTER_AREA)
            payload = shared.get()
            overlay_inference(frame, payload, args.frame_width, args.frame_height)

            homography = compute_homography(editor.src_points, editor.dst_points)
            bev = draw_bev(
                args.bev_width,
                args.bev_height,
                payload,
                homography,
                args.frame_width,
                args.frame_height,
            )
            qt_payload = None
            if payload:
                qt_payload = bridge_pipeline.build_qt_payload(
                    payload,
                    homography,
                    args.bev_width,
                    args.bev_height,
                    args.frame_width,
                    args.frame_height,
                    bridge_state,
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
                )
                qt_server.broadcast(qt_payload)
                bev = draw_qt_payload_bev(
                    args.bev_width,
                    args.bev_height,
                    qt_payload,
                    args.qt_left_margin,
                    args.qt_right_margin,
                    args.qt_top_margin,
                    args.qt_bottom_margin,
                    args.qt_lane_offset_x,
                    args.qt_lane_offset_y,
                )

            panel_h = max(frame.shape[0], bev.shape[0])
            panel_w = frame.shape[1] + bev.shape[1]
            panel = np.zeros((panel_h, panel_w, 3), dtype=np.uint8)
            panel[: frame.shape[0], : frame.shape[1]] = frame
            panel[: bev.shape[0], frame.shape[1] : frame.shape[1] + bev.shape[1]] = bev

            draw_points(panel, editor.src_points, 0, SOURCE_COLOR, "S")
            draw_points(panel, editor.dst_points, frame.shape[1], DEST_COLOR, "D")

            cv2.putText(panel, "SOURCE", (16, 34), cv2.FONT_HERSHEY_SIMPLEX, 0.9, SOURCE_COLOR, 2, cv2.LINE_AA)
            cv2.putText(panel, "BEV", (frame.shape[1] + 16, 34), cv2.FONT_HERSHEY_SIMPLEX, 0.9, DEST_COLOR, 2, cv2.LINE_AA)
            cv2.putText(
                panel,
                "drag points | s: save | r: reset | q: quit",
                (16, panel_h - 18),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.65,
                (220, 220, 220),
                2,
                cv2.LINE_AA,
            )

            cv2.imshow(WINDOW_NAME, panel)
            key = cv2.waitKey(1) & 0xFF
            if key in (27, ord("q")):
                break
            if key == ord("r"):
                editor.src_points, editor.dst_points = default_points(
                    args.frame_width, args.frame_height, args.bev_width, args.bev_height
                )
            if key == ord("s"):
                save_homography(args.output, editor.src_points, editor.dst_points, homography, args)
                print(f"[calib] saved homography to {args.output}")

            elapsed = time.time() - started
            sleep_time = frame_interval - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)
    finally:
        stop_event.set()
        qt_server.stop()
        cap.release()
        cv2.destroyAllWindows()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
