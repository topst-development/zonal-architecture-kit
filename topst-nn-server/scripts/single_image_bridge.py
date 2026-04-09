#!/usr/bin/env python3

import argparse
import json
import socket
import time
from pathlib import Path
from typing import Any, Dict, Optional

import cv2

import perception_bridge as bridge


def parse_args():
    parser = argparse.ArgumentParser(
        description="Send one image to topst-nn-server, receive one result, and rebroadcast a stable Qt payload."
    )
    parser.add_argument("--board-host", default="192.168.0.100", help="topst-nn-server host")
    parser.add_argument("--video-port", type=int, default=9999, help="topst-nn-server video TCP port")
    parser.add_argument("--result-port", type=int, default=9998, help="topst-nn-server JSON result port")
    parser.add_argument("--qt-host", default="0.0.0.0", help="Bind host for Qt clients")
    parser.add_argument("--qt-port", type=int, default=10000, help="Bind port for Qt clients")
    parser.add_argument("--image", required=True, help="Input image path")
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
    parser.add_argument("--object-track-confirm", type=int, default=1, help="Frames required before a tracked object is published")
    parser.add_argument("--object-track-miss", type=int, default=1, help="Frames to keep a tracked object alive without detections")
    parser.add_argument("--object-track-alpha", type=float, default=0.7, help="EMA factor for tracked object position smoothing (0-1)")
    parser.add_argument("--result-timeout", type=float, default=5.0, help="Seconds to wait for one inference JSON result")
    parser.add_argument("--broadcast-fps", type=float, default=5.0, help="How often to rebroadcast the static Qt payload")
    parser.add_argument("--save-raw-json", default=None, help="Optional path to save raw inference JSON")
    parser.add_argument("--save-qt-json", default=None, help="Optional path to save final Qt payload JSON")
    return parser.parse_args()


def load_image_rgb(image_path: str, width: int, height: int) -> bytes:
    frame = cv2.imread(image_path, cv2.IMREAD_COLOR)
    if frame is None:
        raise RuntimeError(f"failed to load image: {image_path}")
    resized = cv2.resize(frame, (width, height), interpolation=cv2.INTER_AREA)
    rgb = cv2.cvtColor(resized, cv2.COLOR_BGR2RGB)
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


def save_json(path: Optional[str], payload: Dict[str, Any]):
    if not path:
        return
    Path(path).write_text(json.dumps(payload, indent=2), encoding="utf-8")


def main():
    args = parse_args()

    homography = bridge.load_homography(args.homography)
    if homography is not None:
        print(f"[image-bridge] loaded homography from {args.homography}")

    qt_server = bridge.QtBroadcastServer(args.qt_host, args.qt_port)
    qt_server.start()

    state = bridge.BridgeState()
    image_bytes = load_image_rgb(args.image, args.width, args.height)
    print(f"[image-bridge] loaded {args.image} and prepared {args.width}x{args.height} RGB frame")

    result_sock = bridge.connect(args.board_host, args.result_port)
    print(f"[image-bridge] connected to inference JSON {args.board_host}:{args.result_port}")

    video_sock = bridge.connect(args.board_host, args.video_port)
    print(f"[image-bridge] connected to inference input {args.board_host}:{args.video_port}")
    video_sock.sendall(image_bytes)
    video_sock.close()
    print("[image-bridge] sent single image frame")

    raw_result = recv_one_json_line(result_sock, args.result_timeout)
    result_sock.close()
    print("[image-bridge] received inference JSON")

    qt_payload = bridge.build_qt_payload(
        raw_result,
        homography,
        args.bev_width,
        args.bev_height,
        args.width,
        args.height,
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

    save_json(args.save_raw_json, raw_result)
    save_json(args.save_qt_json, qt_payload)

    interval = 1.0 / max(args.broadcast_fps, 0.1)
    print(f"[image-bridge] rebroadcasting static Qt payload on {args.qt_host}:{args.qt_port}")
    try:
        while True:
            qt_server.broadcast(qt_payload)
            time.sleep(interval)
    except KeyboardInterrupt:
        pass
    finally:
        qt_server.stop()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
