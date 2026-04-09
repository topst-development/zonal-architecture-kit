#!/usr/bin/env python3

import argparse
import json
import socket
import time
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import cv2
import numpy as np

import perception_bridge as bridge


WINDOW_NAME = "single_image_bev_calibration"
SOURCE_COLOR = (0, 255, 255)
DEST_COLOR = (255, 200, 0)


def parse_args():
    parser = argparse.ArgumentParser(
        description="Interactive BEV homography calibration using a single image inference result."
    )
    parser.add_argument("--board-host", default="192.168.0.100", help="topst-nn-server host")
    parser.add_argument("--video-port", type=int, default=9999, help="topst-nn-server video TCP port")
    parser.add_argument("--result-port", type=int, default=9998, help="topst-nn-server JSON result port")
    parser.add_argument("--qt-host", default="0.0.0.0", help="Bind host for Qt preview clients")
    parser.add_argument("--qt-port", type=int, default=10000, help="Bind port for Qt preview clients")
    parser.add_argument("--image", required=True, help="Input image path")
    parser.add_argument("--frame-width", type=int, default=1280, help="Image width sent to inference app")
    parser.add_argument("--frame-height", type=int, default=720, help="Image height sent to inference app")
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
    parser.add_argument("--object-track-confirm", type=int, default=1, help="Frames required before a tracked object is published")
    parser.add_argument("--object-track-miss", type=int, default=1, help="Frames to keep a tracked object alive without detections")
    parser.add_argument("--object-track-alpha", type=float, default=0.7, help="EMA factor for tracked object position smoothing (0-1)")
    parser.add_argument("--qt-left-margin", type=int, default=10, help="Qt lane canvas left margin")
    parser.add_argument("--qt-right-margin", type=int, default=10, help="Qt lane canvas right margin")
    parser.add_argument("--qt-top-margin", type=int, default=10, help="Qt lane canvas top margin")
    parser.add_argument("--qt-bottom-margin", type=int, default=120, help="Qt lane canvas bottom margin")
    parser.add_argument("--qt-lane-offset-x", type=int, default=0, help="Additional Qt lane X offset")
    parser.add_argument("--qt-lane-offset-y", type=int, default=0, help="Additional Qt lane Y offset")
    parser.add_argument("--points", default=None, help="Optional JSON file with source_points/dest_points")
    parser.add_argument("--output", default="single_image_bev_homography.json", help="Output JSON path")
    parser.add_argument("--result-timeout", type=float, default=5.0, help="Seconds to wait for one inference JSON result")
    parser.add_argument("--save-raw-json", default=None, help="Optional path to save raw inference JSON")
    return parser.parse_args()


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
            candidates.append(("src", idx, float((px - x) ** 2 + (py - y) ** 2)))
        for idx, (px, py) in enumerate(self.dst_points):
            dx = self.src_w + px - x
            candidates.append(("dst", idx, float(dx ** 2 + (py - y) ** 2)))
        kind, idx, dist = min(candidates, key=lambda item: item[2])
        if dist <= 20 ** 2:
            return kind, idx
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
    payload = json.loads(Path(path).read_text(encoding="utf-8"))
    src = payload.get("source_points")
    dst = payload.get("dest_points")
    if not src or not dst or len(src) < 4 or len(dst) < 4:
        raise ValueError("points file must contain source_points and dest_points with 4 entries each")
    return src[:4], dst[:4]


def load_image_bgr(image_path: str, width: int, height: int):
    frame = cv2.imread(image_path, cv2.IMREAD_COLOR)
    if frame is None:
        raise RuntimeError(f"failed to load image: {image_path}")
    return cv2.resize(frame, (width, height), interpolation=cv2.INTER_AREA)


def load_image_rgb_bytes(image_path: str, width: int, height: int) -> bytes:
    frame = load_image_bgr(image_path, width, height)
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    return rgb.tobytes()


def recv_one_json_line(sock: socket.socket, timeout: float) -> Dict[str, Any]:
    sock.settimeout(timeout)
    buffer = b""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            chunk = sock.recv(4096)
        except socket.timeout:
            continue
        if not chunk:
            raise RuntimeError("result socket closed before JSON arrived")
        buffer += chunk
        if b"\n" not in buffer:
            continue
        line, _rest = buffer.split(b"\n", 1)
        line = line.strip()
        if not line:
            continue
        return json.loads(line.decode("utf-8", errors="replace"))
    raise RuntimeError(f"timed out after {timeout:.1f}s waiting for inference JSON")


def overlay_inference(frame: np.ndarray, payload: Dict[str, Any], dst_w: int, dst_h: int):
    src_w = int(payload.get("width", dst_w))
    src_h = int(payload.get("height", dst_h))

    for lane in payload.get("lanes", []) or []:
        points = lane.get("points", []) or []
        if len(points) < 2:
            continue
        poly = []
        for point in points:
            x = int(round(float(point.get("x", 0.0)) * dst_w / max(src_w, 1)))
            y = int(round(float(point.get("y", 0.0)) * dst_h / max(src_h, 1)))
            poly.append([x, y])
        cv2.polylines(frame, [np.array(poly, dtype=np.int32)], False, (0, 255, 255), 2, cv2.LINE_AA)

    for obj in payload.get("objects", []) or []:
        x1 = int(round(float(obj.get("x_min", 0.0)) * dst_w / max(src_w, 1)))
        y1 = int(round(float(obj.get("y_min", 0.0)) * dst_h / max(src_h, 1)))
        x2 = int(round(float(obj.get("x_max", 0.0)) * dst_w / max(src_w, 1)))
        y2 = int(round(float(obj.get("y_max", 0.0)) * dst_h / max(src_h, 1)))
        cv2.rectangle(frame, (x1, y1), (x2, y2), (255, 255, 255), 2, cv2.LINE_AA)


def compute_homography(src_points: List[List[int]], dst_points: List[List[int]]) -> np.ndarray:
    src = np.asarray(src_points, dtype=np.float32)
    dst = np.asarray(dst_points, dtype=np.float32)
    return cv2.getPerspectiveTransform(src, dst)


def draw_points(panel: np.ndarray, points: List[List[int]], x_offset: int, color: Tuple[int, int, int], label: str):
    for idx, (x, y) in enumerate(points):
        px = int(x + x_offset)
        py = int(y)
        cv2.circle(panel, (px, py), 8, color, -1, cv2.LINE_AA)
        cv2.circle(panel, (px, py), 11, (24, 28, 34), 2, cv2.LINE_AA)
        cv2.putText(panel, f"{label}{idx}", (px + 10, py - 8), cv2.FONT_HERSHEY_SIMPLEX, 0.55, color, 2, cv2.LINE_AA)


def save_homography(output: str, src_points: List[List[int]], dst_points: List[List[int]], homography: np.ndarray, args):
    payload = {
        "image": args.image,
        "source_width": args.frame_width,
        "source_height": args.frame_height,
        "bev_width": args.bev_width,
        "bev_height": args.bev_height,
        "source_points": src_points,
        "dest_points": dst_points,
        "homography": homography.tolist(),
    }
    Path(output).write_text(json.dumps(payload, indent=2), encoding="utf-8")


def main():
    args = parse_args()
    src_points, dst_points = load_points(args.points, args.frame_width, args.frame_height, args.bev_width, args.bev_height)
    source_frame = load_image_bgr(args.image, args.frame_width, args.frame_height)
    image_bytes = load_image_rgb_bytes(args.image, args.frame_width, args.frame_height)

    qt_server = bridge.QtBroadcastServer(args.qt_host, args.qt_port)
    qt_server.start()
    state = bridge.BridgeState()

    result_sock = bridge.connect(args.board_host, args.result_port)
    print(f"[single-calib] connected to inference JSON {args.board_host}:{args.result_port}")
    video_sock = bridge.connect(args.board_host, args.video_port)
    print(f"[single-calib] connected to inference input {args.board_host}:{args.video_port}")
    video_sock.sendall(image_bytes)
    video_sock.close()
    print("[single-calib] sent single image frame")

    raw_result = recv_one_json_line(result_sock, args.result_timeout)
    result_sock.close()
    print("[single-calib] received inference JSON")

    if args.save_raw_json:
        Path(args.save_raw_json).write_text(json.dumps(raw_result, indent=2), encoding="utf-8")

    editor = PointEditor(src_points, dst_points, args.frame_width)
    cv2.namedWindow(WINDOW_NAME, cv2.WINDOW_NORMAL)
    cv2.setMouseCallback(WINDOW_NAME, editor.on_mouse)

    try:
        while True:
            frame = source_frame.copy()
            overlay_inference(frame, raw_result, args.frame_width, args.frame_height)

            homography = compute_homography(editor.src_points, editor.dst_points)
            qt_payload = bridge.build_qt_payload(
                raw_result,
                homography,
                args.bev_width,
                args.bev_height,
                args.frame_width,
                args.frame_height,
                state,
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

            bev = np.zeros((args.bev_height, args.bev_width, 3), dtype=np.uint8)
            bev[:] = (12, 18, 24)
            lanes = qt_payload.get("lanes", []) or []
            for points in lanes[:4]:
                if len(points) < 2:
                    continue
                poly = np.array(
                    [[int(point.get("x", 0)), int(point.get("y", 0))] for point in points],
                    dtype=np.int32,
                )
                cv2.polylines(bev, [poly], False, (160, 170, 180), 3, cv2.LINE_AA)
            for obj in qt_payload.get("bev_objects", []) or []:
                x = int(obj.get("bev_x", 0))
                y = int(obj.get("bev_y", 0))
                cv2.circle(bev, (x, y), 8, (255, 255, 255), -1, cv2.LINE_AA)
                cv2.circle(bev, (x, y), 12, (255, 170, 100), 2, cv2.LINE_AA)

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
            key = cv2.waitKey(16) & 0xFF
            if key in (27, ord("q")):
                break
            if key == ord("r"):
                editor.src_points, editor.dst_points = default_points(
                    args.frame_width, args.frame_height, args.bev_width, args.bev_height
                )
            if key == ord("s"):
                save_homography(args.output, editor.src_points, editor.dst_points, homography, args)
                print(f"[single-calib] saved homography to {args.output}")
    finally:
        qt_server.stop()
        cv2.destroyAllWindows()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
