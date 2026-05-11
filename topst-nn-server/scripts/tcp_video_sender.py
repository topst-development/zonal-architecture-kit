#!/usr/bin/env python3

import argparse
import json
import socket
import sys
import time
import threading

import cv2


def parse_args():
    parser = argparse.ArgumentParser(
        description="Send video frames to topst-nn-server TCP input mode."
    )
    parser.add_argument(
        "--host",
        default="192.168.0.100",
        help="Board IP address (default: 192.168.0.100)",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=9999,
        help="Board TCP port (default: 9999)",
    )
    parser.add_argument(
        "--json-port",
        type=int,
        default=9998,
        help="Board JSON result port (default: 9998)",
    )
    parser.add_argument(
        "--video",
        default="output.mp4",
        help="Input video path (default: output.mp4)",
    )
    parser.add_argument(
        "--width",
        type=int,
        default=1280,
        help="Frame width expected by the board app (default: 1280)",
    )
    parser.add_argument(
        "--height",
        type=int,
        default=720,
        help="Frame height expected by the board app (default: 720)",
    )
    parser.add_argument(
        "--loop",
        action="store_true",
        help="Loop the video when it reaches the end",
    )
    parser.add_argument(
        "--fps",
        type=float,
        default=None,
        help="Override send FPS. If omitted, use video FPS.",
    )
    parser.add_argument(
        "--no-throttle",
        action="store_true",
        help="Send frames as fast as possible without FPS pacing.",
    )
    return parser.parse_args()


def open_video(path):
    cap = cv2.VideoCapture(path)
    if not cap.isOpened():
        raise RuntimeError(f"failed to open video: {path}")
    return cap


def connect(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    return sock


def json_receiver(host, port, stop_event):
    sock = connect(host, port)
    buffer = b""
    print(f"connected to JSON result socket {host}:{port}")

    try:
        while not stop_event.is_set():
            data = sock.recv(4096)
            if not data:
                break
            buffer += data
            while b"\n" in buffer:
                line, buffer = buffer.split(b"\n", 1)
                line = line.strip()
                if line:
                    try:
                        payload = json.loads(line.decode("utf-8", errors="replace"))
                    except json.JSONDecodeError:
                        print("[json-raw]", line.decode("utf-8", errors="replace"))
                        continue
                    print(format_result(payload))
    finally:
        sock.close()


def format_result(payload):
    frame_index = payload.get("frame_index", -1)
    perf = payload.get("perf", {})
    objects = payload.get("objects", [])
    lanes = payload.get("lanes", [])

    lines = []
    lines.append(
        f"\n[frame {frame_index}] "
        f"fps={perf.get('fps', 0):.2f} "
        f"cpu={perf.get('cpu', 0)}% "
        f"mem={perf.get('mem', 0)}%"
    )

    if objects:
        lines.append(f"  objects ({len(objects)}):")
        for obj in objects:
            lines.append(
                "    "
                f"model={obj.get('model')} "
                f"cls={obj.get('cls')} "
                f"score={obj.get('score', 0):.3f} "
                f"box=({obj.get('x_min', 0):.1f}, {obj.get('y_min', 0):.1f})-"
                f"({obj.get('x_max', 0):.1f}, {obj.get('y_max', 0):.1f})"
            )
    else:
        lines.append("  objects: none")

    if lanes:
        lines.append(f"  lanes ({len(lanes)}):")
        for lane in lanes:
            points = lane.get("points", [])
            if points:
                first = points[0]
                last = points[-1]
                lines.append(
                    "    "
                    f"model={lane.get('model')} "
                    f"lane_id={lane.get('lane_id')} "
                    f"points={len(points)} "
                    f"first=({first.get('x')}, {first.get('y')}) "
                    f"last=({last.get('x')}, {last.get('y')})"
                )
            else:
                lines.append(
                    "    "
                    f"model={lane.get('model')} "
                    f"lane_id={lane.get('lane_id')} "
                    "points=0"
                )
    else:
        lines.append("  lanes: none")

    return "\n".join(lines)


def main():
    args = parse_args()
    cap = open_video(args.video)
    sock = connect(args.host, args.port)
    stop_event = threading.Event()
    receiver = threading.Thread(
        target=json_receiver,
        args=(args.host, args.json_port, stop_event),
        daemon=True,
    )
    receiver.start()

    fps = args.fps if args.fps is not None else cap.get(cv2.CAP_PROP_FPS)
    if fps <= 0.0:
        fps = 30.0
    frame_interval = 0.0 if args.no_throttle else (1.0 / fps)

    print(
        f"sending {args.video} to {args.host}:{args.port} "
        f"as {args.width}x{args.height} RGB888"
        f"{' without throttling' if args.no_throttle else f' @ {fps:.2f}fps'}"
    )

    try:
        while True:
            started = time.time()
            ret, frame = cap.read()
            if not ret:
                if args.loop:
                    cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                    continue
                break

            resized = cv2.resize(frame, (args.width, args.height), interpolation=cv2.INTER_AREA)
            rgb = cv2.cvtColor(resized, cv2.COLOR_BGR2RGB)
            sock.sendall(rgb.tobytes())

            elapsed = time.time() - started
            sleep_time = frame_interval - elapsed
            #if frame_interval > 0.0 and sleep_time > 0:
                #time.sleep(sleep_time)
    except KeyboardInterrupt:
        pass
    finally:
        stop_event.set()
        sock.close()
        cap.release()

    return 0


if __name__ == "__main__":
    sys.exit(main())
