#!/usr/bin/env python3
"""
bev_tune_viewer 에서 s 키로 저장한 2개 JSON 을 기본으로 사용하는 Qt 브릿지.

  - homography_tuned.json  : 3x3 호모그래피 (--homography)
  - bev_tweak_tuned.json   : BEV 픽셀 보정 (--bev-tweak-json)

NPU 가 보내는 추론 JSON 을 받아 BEV 변환 후 Qt(D3-G fabless) 로 브로드캐스트한다.
로직은 bev_lane_bridge.py 와 동일하며, 기본 파일명만 튜닝 결과에 맞춰 둔다.
"""

from __future__ import annotations

import argparse
import sys
import threading
import time
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from bev_lane_bridge import (  # noqa: E402
    QtBroadcastServer,
    load_homography,
    merge_bev_correction,
    result_loop,
    video_loop,
)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="Tuned homography + bev_tweak JSON -> NPU JSON in -> BEV -> Qt out.",
        epilog="Defaults expect homography_tuned.json & bev_tweak_tuned.json next to this script or CWD.",
    )
    p.add_argument("--board-host", default="192.168.0.100", help="NPU / inference board IP")
    p.add_argument("--video-port", type=int, default=9999, help="RGB TCP port to board")
    p.add_argument("--result-port", type=int, default=9998, help="JSON result TCP from board")
    p.add_argument("--qt-host", default="0.0.0.0", help="Bind for Qt clients")
    p.add_argument("--qt-port", type=int, default=10000, help="Port for Qt (fabless config)")
    p.add_argument("--video", default="output.mp4", help="Video file to stream to board")
    p.add_argument("--width", type=int, default=1280, help="Frame width sent to board")
    p.add_argument("--height", type=int, default=720, help="Frame height sent to board")
    p.add_argument(
        "--homography",
        default="homography_tuned.json",
        help="3x3 JSON from bev_tune_viewer (s key), default: homography_tuned.json",
    )
    p.add_argument("--bev-width", type=int, default=800, help="BEV canvas width for Qt")
    p.add_argument("--bev-height", type=int, default=480, help="BEV canvas height for Qt")
    p.add_argument("--max-objects", type=int, default=32, help="Max objects in Qt payload (0 = no cap)")
    p.add_argument("--no-objects", action="store_true", help="Lanes only, no detections")
    p.add_argument("--loop", action="store_true", help="Loop video file")
    p.add_argument("--fps", type=float, default=None, help="Override video send FPS")
    p.add_argument("--no-throttle", action="store_true", help="Send video frames as fast as possible")
    p.add_argument(
        "--no-video",
        action="store_true",
        help="Do not send video; only read NPU JSON and broadcast to Qt (feed board elsewhere)",
    )
    p.add_argument(
        "--bev-tweak-json",
        default="bev_tweak_tuned.json",
        help="Offset/scale/pivot JSON from bev_tune_viewer, default: bev_tweak_tuned.json",
    )
    p.add_argument("--bev-post-matrix", default=None, help="Optional extra 3x3 in BEV space")
    p.add_argument("--bev-offset-x", type=float, default=None)
    p.add_argument("--bev-offset-y", type=float, default=None)
    p.add_argument("--bev-scale-x", type=float, default=None)
    p.add_argument("--bev-scale-y", type=float, default=None)
    p.add_argument("--bev-pivot-x", type=float, default=None)
    p.add_argument("--bev-pivot-y", type=float, default=None)
    p.add_argument("--lane-fit-degree", type=int, default=2, help="0=off; BEV polynomial lane fit (default 2)")
    p.add_argument("--lane-fit-samples", type=int, default=24)
    p.add_argument("--lane-fit-min-points", type=int, default=4)
    p.add_argument("--lane-top-ratio", type=float, default=0.22)
    p.add_argument("--lane-bottom-ratio", type=float, default=0.92)
    return p.parse_args()


def _resolve_path(p: str) -> Path:
    path = Path(p)
    if path.is_file():
        return path.resolve()
    # next to script
    alt = SCRIPT_DIR / p
    if alt.is_file():
        return alt.resolve()
    return path


def main() -> int:
    args = parse_args()

    homography_path = _resolve_path(args.homography)
    if not homography_path.is_file():
        print(f"[tuned-qt-bridge] missing homography file: {args.homography}", file=sys.stderr)
        return 1

    tweak_path: str | None = args.bev_tweak_json
    if tweak_path:
        tp = _resolve_path(tweak_path)
        if tp.is_file():
            tweak_path = str(tp)
        else:
            print(f"[tuned-qt-bridge] tweak file not found ({tweak_path}), using identity BEV tweak")
            tweak_path = None

    homography = load_homography(str(homography_path))
    print(f"[tuned-qt-bridge] homography: {homography_path}")

    correction = merge_bev_correction(
        args.bev_width,
        args.bev_height,
        tweak_path,
        args.bev_post_matrix,
        args.bev_offset_x,
        args.bev_offset_y,
        args.bev_scale_x,
        args.bev_scale_y,
        args.bev_pivot_x,
        args.bev_pivot_y,
    )
    print(
        f"[tuned-qt-bridge] tweak: {tweak_path or '(none)'}  "
        f"offset=({correction.offset_x:.2f},{correction.offset_y:.2f}) "
        f"scale=({correction.scale_x:.4f},{correction.scale_y:.4f})"
    )
    if args.lane_fit_degree > 0:
        print(
            f"[tuned-qt-bridge] lane polyfit: degree={args.lane_fit_degree} "
            f"samples={args.lane_fit_samples} y=[{args.lane_top_ratio:.2f},{args.lane_bottom_ratio:.2f}]"
        )
    else:
        print("[tuned-qt-bridge] lane polyfit: off")

    qt = QtBroadcastServer(args.qt_host, args.qt_port)
    qt.start()
    stop_event = threading.Event()

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
        ),
        daemon=True,
    )
    receiver.start()

    try:
        if args.no_video:
            print("[tuned-qt-bridge] --no-video: JSON -> Qt only (Ctrl+C to stop)")
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
