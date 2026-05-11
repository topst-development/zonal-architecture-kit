#!/usr/bin/env python3

import argparse
import json
from pathlib import Path

import cv2


WINDOW_NAME = "pick_bev_points"


class PointPicker:
    def __init__(self, frame, video_path: str, frame_index: int):
        self.original = frame
        self.display = frame.copy()
        self.points = []
        self.video_path = video_path
        self.frame_index = frame_index

    def redraw(self):
        self.display = self.original.copy()
        for idx, (x, y) in enumerate(self.points):
            cv2.circle(self.display, (x, y), 6, (0, 255, 255), -1)
            cv2.putText(
                self.display,
                str(idx),
                (x + 8, y - 8),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                (0, 255, 255),
                2,
                cv2.LINE_AA,
            )

        help_lines = [
            "Left click: add point",
            "u: undo last point",
            "c: clear all points",
            "s: save and exit",
            "q or ESC: quit without saving",
        ]
        y = 24
        for line in help_lines:
            cv2.putText(
                self.display,
                line,
                (16, y),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                (255, 255, 255),
                2,
                cv2.LINE_AA,
            )
            y += 24

    def on_mouse(self, event, x, y, _flags, _userdata):
        if event == cv2.EVENT_LBUTTONDOWN:
            self.points.append((int(x), int(y)))
            self.redraw()

    def save(self, output_path: Path):
        payload = {
            "video": self.video_path,
            "frame_index": self.frame_index,
            "image_width": int(self.original.shape[1]),
            "image_height": int(self.original.shape[0]),
            "image_points": [{"x": x, "y": y} for x, y in self.points],
        }
        output_path.write_text(json.dumps(payload, indent=2))


def parse_args():
    parser = argparse.ArgumentParser(
        description="Pick image-space points for BEV homography calibration."
    )
    parser.add_argument(
        "--video",
        default="output.mp4",
        help="Input video path (default: output.mp4)",
    )
    parser.add_argument(
        "--frame-index",
        type=int,
        default=0,
        help="Frame index to load from the video (default: 0)",
    )
    parser.add_argument(
        "--output",
        default="bev_image_points.json",
        help="Output JSON path (default: bev_image_points.json)",
    )
    return parser.parse_args()


def load_frame(video_path: str, frame_index: int):
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        raise RuntimeError(f"failed to open video: {video_path}")

    if frame_index > 0:
        cap.set(cv2.CAP_PROP_POS_FRAMES, frame_index)

    ok, frame = cap.read()
    cap.release()
    if not ok or frame is None:
        raise RuntimeError(f"failed to read frame {frame_index} from: {video_path}")
    return frame


def main():
    args = parse_args()
    frame = load_frame(args.video, args.frame_index)
    picker = PointPicker(frame, args.video, args.frame_index)
    picker.redraw()

    cv2.namedWindow(WINDOW_NAME, cv2.WINDOW_NORMAL)
    cv2.setMouseCallback(WINDOW_NAME, picker.on_mouse)

    while True:
        cv2.imshow(WINDOW_NAME, picker.display)
        key = cv2.waitKey(20) & 0xFF

        if key in (27, ord("q")):
            break
        if key == ord("u") and picker.points:
            picker.points.pop()
            picker.redraw()
        elif key == ord("c"):
            picker.points.clear()
            picker.redraw()
        elif key == ord("s"):
            output_path = Path(args.output)
            picker.save(output_path)
            print(f"saved {len(picker.points)} points to {output_path}")
            break

    cv2.destroyAllWindows()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
