#!/usr/bin/env python3
"""
OpenCV로 영상 + BEV를 나란히 띄우고, 마우스로 원근 4점을 움직이며(호모그래피),
트랙바로 BEV 보정(offset/scale/pivot)을 실시간 확인한다.

옵션: --board-host 로 NPU에 RGB 영상 TCP 전송(--video-port, --send-width/height),
보드 JSON(--result-host 미지정 시 board-host 와 동일) 또는 --json-log 로
차선·객체 추론을 원본/BEV에 그린다.

--qt-port > 0 이면 bev_lane_bridge 와 동일 형식의 JSON 을 TCP 로 브로드캐스트해
D3-G fabless(Qt) 가 localhost:포트 로 접속해 동일 화면을 볼 수 있다.
"""

from __future__ import annotations

import argparse
import json
import socket
import sys
import threading
import time
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import cv2
import numpy as np

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from bev_lane_bridge import (  # noqa: E402
    BevLaneState,
    BevCorrection,
    QtBroadcastServer,
    build_bev_payload,
    load_matrix3_json,
)

LANE_COLORS = (
    (80, 180, 255),
    (80, 255, 180),
    (200, 120, 255),
    (255, 200, 100),
)


def affine_scale_pivot_3x3(
    sx: float,
    sy: float,
    px: float,
    py: float,
    ox: float,
    oy: float,
) -> np.ndarray:
    tx = px * (1.0 - sx) + ox
    ty = py * (1.0 - sy) + oy
    return np.array([[sx, 0.0, tx], [0.0, sy, ty], [0.0, 0.0, 1.0]], dtype=np.float64)


def apply_correction_to_bev_image(
    bev_bgr: np.ndarray,
    corr: BevCorrection,
) -> np.ndarray:
    """bev_lane_bridge.BeavCorrection 과 동일한 순서로 BEV 래스터에 적용."""
    h, w = bev_bgr.shape[:2]
    out = bev_bgr
    if corr.post_matrix is not None:
        out = cv2.warpPerspective(out, corr.post_matrix, (w, h), flags=cv2.INTER_LINEAR)
    a = affine_scale_pivot_3x3(
        corr.scale_x,
        corr.scale_y,
        corr.pivot_x,
        corr.pivot_y,
        corr.offset_x,
        corr.offset_y,
    )
    out = cv2.warpPerspective(out, a, (w, h), flags=cv2.INTER_LINEAR)
    return out


def bev_corners(bev_w: int, bev_h: int) -> np.ndarray:
    return np.array(
        [
            [0.0, 0.0],
            [float(bev_w - 1), 0.0],
            [float(bev_w - 1), float(bev_h - 1)],
            [0.0, float(bev_h - 1)],
        ],
        dtype=np.float32,
    )


def src_points_from_homography(H: np.ndarray, bev_w: int, bev_h: int) -> np.ndarray:
    Hi = np.linalg.inv(H.astype(np.float64))
    c = bev_corners(bev_w, bev_h).reshape(1, 4, 2)
    src = cv2.perspectiveTransform(c, Hi.astype(np.float32))[0]
    return src.astype(np.float32)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Interactive BEV tune: drag 4 points + trackbars.")
    p.add_argument("--video", default="output.mp4", help="Video file")
    p.add_argument("--camera", type=int, default=None, help="If set, use this camera index instead of --video")
    p.add_argument("--homography", default=None, help="Initial 3x3 JSON (optional; default trapezoid if omitted)")
    p.add_argument("--bev-width", type=int, default=800)
    p.add_argument("--bev-height", type=int, default=480)
    p.add_argument("--bev-tweak-json", default=None, help="Load initial offset/scale/pivot")
    p.add_argument("--bev-post-matrix", default=None, help="Optional 3x3 post matrix JSON")
    p.add_argument("--save-homography", default="homography_tuned.json", help="Output path for H (s key)")
    p.add_argument("--save-tweak", default="bev_tweak_tuned.json", help="Output path for tweak (s key)")
    p.add_argument(
        "--save-post-matrix",
        default="bev_post_matrix_tuned.json",
        help="If --bev-post-matrix was used, also write current post matrix here on save",
    )
    p.add_argument(
        "--board-host",
        default=None,
        help="NPU IP: send each frame as RGB888 TCP to --video-port (same protocol as bev_lane_bridge)",
    )
    p.add_argument("--video-port", type=int, default=9999, help="TCP port for raw RGB stream to board")
    p.add_argument(
        "--send-width",
        type=int,
        default=None,
        help="Resize width for NPU stream (default: --infer-width or video width)",
    )
    p.add_argument("--send-height", type=int, default=None, help="Resize height for NPU stream")
    p.add_argument(
        "--result-host",
        default=None,
        help="JSON 추론 수신 IP (미입력이고 --board-host 만 있으면 동일 호스트 사용)",
    )
    p.add_argument("--result-port", type=int, default=9998, help="TCP port for JSON lines")
    p.add_argument(
        "--json-log",
        default=None,
        help="Offline: newline-delimited JSON (one frame per line), synced with video frames",
    )
    p.add_argument(
        "--infer-width",
        type=int,
        default=1280,
        help="Resolution sent to board / used in transform (default: 1280)",
    )
    p.add_argument("--infer-height", type=int, default=720, help="Default: 720")
    p.add_argument(
        "--sync-frame-index",
        action="store_true",
        help="Use payload whose frame_index matches viewer frame counter (TCP buffer)",
    )
    p.add_argument(
        "--qt-host",
        default="0.0.0.0",
        help="Bind address for Qt telemetry clients (D3-G fabless)",
    )
    p.add_argument(
        "--qt-port",
        type=int,
        default=0,
        help="If >0, TCP server: newline JSON for Qt (same schema as bev_lane_bridge). Example: 10000",
    )
    p.add_argument("--qt-max-objects", type=int, default=32, help="Max objects in Qt payload")
    p.add_argument(
        "--lane-fit-degree",
        type=int,
        default=2,
        help="BEV lane polynomial fit for preview/Qt: 0=off, 1=line, 2=quadratic",
    )
    p.add_argument("--lane-fit-samples", type=int, default=24, help="Points sampled along fitted curve per lane")
    p.add_argument("--lane-fit-min-points", type=int, default=4, help="Minimum raw BEV points required before fitting")
    p.add_argument("--lane-top-ratio", type=float, default=0.12, help="Normalized top Y on BEV canvas for fit range")
    p.add_argument("--lane-bottom-ratio", type=float, default=0.97, help="Normalized bottom Y on BEV canvas for fit range")
    p.add_argument("--lane-smoothing", type=float, default=0.7, help="EMA factor for BEV lane stabilization (0-1)")
    return p.parse_args()


class TuneUI:
    TB_OX = "ox+200"
    TB_OY = "oy+200"
    TB_SX = "sx*100"
    TB_SY = "sy*100"
    TB_PX = "pivot_x"
    TB_PY = "pivot_y"

    def __init__(
        self,
        bev_w: int,
        bev_h: int,
        initial_src: np.ndarray,
        tweak_path: Optional[str],
        post_path: Optional[str],
    ):
        self.bev_w = bev_w
        self.bev_h = bev_h
        self.src_pts = initial_src.copy()
        self.dst_pts = bev_corners(bev_w, bev_h)
        self.drag_idx: int = -1
        self.drag_radius = 18

        d = {
            "offset_x": 0.0,
            "offset_y": 0.0,
            "scale_x": 1.0,
            "scale_y": 1.0,
            "pivot_x": bev_w / 2.0,
            "pivot_y": bev_h / 2.0,
        }
        if tweak_path:
            t = json.loads(open(tweak_path, "r", encoding="utf-8").read())
            for k in d:
                if k in t:
                    d[k] = float(t[k])

        self.post: Optional[np.ndarray] = load_matrix3_json(post_path) if post_path else None

        self._ox = int(round(d["offset_x"] + 200))
        self._oy = int(round(d["offset_y"] + 200))
        self._sx = max(25, min(400, int(round(d["scale_x"] * 100))))
        self._sy = max(25, min(400, int(round(d["scale_y"] * 100))))
        self._px = int(round(d["pivot_x"]))
        self._py = int(round(d["pivot_y"]))

    def current_homography(self) -> np.ndarray:
        try:
            return cv2.getPerspectiveTransform(self.src_pts, self.dst_pts)
        except cv2.error:
            return np.eye(3, dtype=np.float32)

    def current_correction(self) -> BevCorrection:
        ox = float(self._ox - 200)
        oy = float(self._oy - 200)
        sx = self._sx / 100.0
        sy = self._sy / 100.0
        px = float(self._px)
        py = float(self._py)
        return BevCorrection(self.post, ox, oy, sx, sy, px, py)

    def on_trackbar(self, _val: int) -> None:
        pass

    def setup_trackbars(self, win: str) -> None:
        cv2.createTrackbar(self.TB_OX, win, self._ox, 400, self._on_ox)
        cv2.createTrackbar(self.TB_OY, win, self._oy, 400, self._on_oy)
        cv2.createTrackbar(self.TB_SX, win, self._sx, 400, self._on_sx)
        cv2.createTrackbar(self.TB_SY, win, self._sy, 400, self._on_sy)
        cv2.createTrackbar(self.TB_PX, win, self._px, max(1, self.bev_w - 1), self._on_px)
        cv2.createTrackbar(self.TB_PY, win, self._py, max(1, self.bev_h - 1), self._on_py)

    def _on_ox(self, v: int) -> None:
        self._ox = v

    def _on_oy(self, v: int) -> None:
        self._oy = v

    def _on_sx(self, v: int) -> None:
        self._sx = max(25, v)

    def _on_sy(self, v: int) -> None:
        self._sy = max(25, v)

    def _on_px(self, v: int) -> None:
        self._px = v

    def _on_py(self, v: int) -> None:
        self._py = v

    def nearest_point(self, x: float, y: float) -> int:
        best = -1
        best_d = float(self.drag_radius**2)
        for i, (px, py) in enumerate(self.src_pts):
            dx = x - px
            dy = y - py
            s = dx * dx + dy * dy
            if s <= best_d:
                best_d = s
                best = i
        return best


class InferenceBuffer:
    """TCP로 들어오는 추론 JSON 최신값 + (옵션) frame_index 버퍼."""

    def __init__(self, sync_by_index: bool, max_buffer: int = 256):
        self._lock = threading.Lock()
        self._latest: Optional[Dict[str, Any]] = None
        self._by_index: Dict[int, Dict[str, Any]] = {}
        self.sync_by_index = sync_by_index
        self.max_buffer = max_buffer

    def push(self, payload: Dict[str, Any]) -> None:
        with self._lock:
            self._latest = payload
            idx = int(payload.get("frame_index", -1))
            if idx >= 0:
                self._by_index[idx] = payload
                while len(self._by_index) > self.max_buffer:
                    self._by_index.pop(min(self._by_index.keys()), None)

    def get_for_frame(self, frame_idx: int) -> Optional[Dict[str, Any]]:
        with self._lock:
            if self.sync_by_index:
                hit = self._by_index.get(frame_idx)
                if hit is not None:
                    return dict(hit)
            if self._latest is not None:
                return dict(self._latest)
            return None


class NpuVideoSender:
    """NPU topst-nn 스타일: 프레임을 RGB888 고정 바이트로 TCP 전송."""

    def __init__(self, host: str, port: int, width: int, height: int):
        self.host = host
        self.port = port
        self.width = width
        self.height = height
        self._sock: Optional[socket.socket] = None

    def _close(self) -> None:
        if self._sock is not None:
            try:
                self._sock.close()
            except OSError:
                pass
            self._sock = None

    def _connect(self) -> None:
        self._close()
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((self.host, self.port))
        self._sock = s
        print(f"[bev_tune] NPU video -> {self.host}:{self.port}  {self.width}x{self.height} RGB888")

    def send_bgr(self, frame: np.ndarray) -> None:
        small = cv2.resize(frame, (self.width, self.height), interpolation=cv2.INTER_AREA)
        rgb = cv2.cvtColor(small, cv2.COLOR_BGR2RGB)
        payload = rgb.tobytes()
        for _ in range(2):
            try:
                if self._sock is None:
                    self._connect()
                if self._sock is not None:
                    self._sock.sendall(payload)
                return
            except OSError as exc:
                print(f"[bev_tune] NPU video send ({exc}), retry")
                self._close()

    def close(self) -> None:
        self._close()


def tcp_json_receiver(host: str, port: int, buf: InferenceBuffer, stop: threading.Event) -> None:
    while not stop.is_set():
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(1.0)
            s.connect((host, port))
            s.settimeout(None)
            print(f"[bev_tune] inference TCP connected {host}:{port}")
            data = b""
            while not stop.is_set():
                chunk = s.recv(8192)
                if not chunk:
                    break
                data += chunk
                while b"\n" in data:
                    line, data = data.split(b"\n", 1)
                    line = line.strip()
                    if not line:
                        continue
                    try:
                        payload = json.loads(line.decode("utf-8", errors="replace"))
                        buf.push(payload)
                    except json.JSONDecodeError:
                        continue
            s.close()
        except OSError as exc:
            if not stop.is_set():
                print(f"[bev_tune] inference TCP wait {host}:{port} ({exc})")
            time.sleep(1.0)


class JsonLogReader:
    def __init__(self, path: str):
        raw = Path(path).read_text(encoding="utf-8").splitlines()
        self._lines = [ln for ln in raw if ln.strip()]
        self._idx = 0

    def next_payload(self) -> Optional[Dict[str, Any]]:
        if self._idx >= len(self._lines):
            return None
        line = self._lines[self._idx]
        self._idx += 1
        return json.loads(line)

    def rewind(self) -> None:
        self._idx = 0


def draw_inference_on_image(
    img: np.ndarray,
    payload: Dict[str, Any],
    frame_w: int,
    frame_h: int,
) -> None:
    pw = int(payload.get("width", frame_w))
    ph = int(payload.get("height", frame_h))
    sx = frame_w / max(pw, 1)
    sy = frame_h / max(ph, 1)

    for lane in payload.get("lanes") or []:
        lid = int(lane.get("lane_id", -1))
        color = LANE_COLORS[lid % len(LANE_COLORS)]
        pts: List[Tuple[int, int]] = []
        for p in lane.get("points") or []:
            x = int(round(float(p.get("x", 0.0)) * sx))
            y = int(round(float(p.get("y", 0.0)) * sy))
            pts.append((x, y))
        if len(pts) >= 2:
            cv2.polylines(img, [np.array(pts, dtype=np.int32)], False, color, 2, cv2.LINE_AA)

    for obj in payload.get("objects") or []:
        x0 = int(round(float(obj.get("x_min", 0.0)) * sx))
        y0 = int(round(float(obj.get("y_min", 0.0)) * sy))
        x1 = int(round(float(obj.get("x_max", 0.0)) * sx))
        y1 = int(round(float(obj.get("y_max", 0.0)) * sy))
        cls = int(obj.get("cls", 0))
        sc = float(obj.get("score", 0.0))
        cv2.rectangle(img, (x0, y0), (x1, y1), (80, 220, 80), 2)
        cv2.putText(
            img,
            f"{cls}:{sc:.2f}",
            (x0, max(16, y0 - 4)),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.45,
            (80, 220, 80),
            1,
            cv2.LINE_AA,
        )


def draw_qt_payload_on_bev(
    bev_img: np.ndarray,
    qt_payload: Dict[str, Any],
) -> None:
    for lane_idx, lane_points in enumerate(qt_payload.get("lanes") or []):
        color = LANE_COLORS[lane_idx % len(LANE_COLORS)]
        if len(lane_points) >= 2:
            arr = np.array(
                [(int(p.get("x", 0)), int(p.get("y", 0))) for p in lane_points],
                dtype=np.int32,
            )
            cv2.polylines(bev_img, [arr], False, color, 2, cv2.LINE_AA)

    for obj in qt_payload.get("bev_objects") or []:
        bx = int(obj.get("bev_x", 0))
        by = int(obj.get("bev_y", 0))
        cls = int(obj.get("cls", 0) or 0)
        cv2.circle(bev_img, (bx, by), 7, (60, 60, 255), -1)
        cv2.putText(
            bev_img,
            str(cls),
            (bx + 8, by - 4),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (60, 60, 255),
            1,
            cv2.LINE_AA,
        )


def draw_ui_overlay(
    img: np.ndarray,
    paused: bool,
    has_inference: bool,
    send_video: bool,
    qt_broadcast: bool,
) -> None:
    h, w = img.shape[:2]
    lines = [
        "Drag: move 4 src points (homography)",
        "SPACE: pause/resume  r: reset H from file  s: save JSON",
    ]
    if has_inference:
        lines.append("Inference: lanes + objects (TCP or --json-log)")
    if send_video:
        lines.append("NPU: RGB stream (--board-host)")
    if qt_broadcast:
        lines.append("Qt: TCP JSON (--qt-port)")
    lines.append("q/ESC: quit")
    if paused:
        lines.insert(1, "[PAUSED]")
    y = 22
    for line in lines:
        cv2.putText(img, line, (12, y), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 255, 0), 2, cv2.LINE_AA)
        y += 26
    cv2.rectangle(img, (4, 4), (w - 4, h - 4), (80, 80, 80), 1)


def main() -> int:
    args = parse_args()
    bev_w, bev_h = args.bev_width, args.bev_height

    if args.camera is not None:
        cap = cv2.VideoCapture(args.camera)
        src_label = f"camera {args.camera}"
    else:
        cap = cv2.VideoCapture(args.video)
        src_label = args.video

    if not cap.isOpened():
        print(f"cannot open: {src_label}", file=sys.stderr)
        return 1

    ok, frame0 = cap.read()
    if not ok:
        print("cannot read first frame", file=sys.stderr)
        return 1
    fh, fw = frame0.shape[:2]
    cap.set(cv2.CAP_PROP_POS_FRAMES, 0)

    if args.homography:
        H0 = load_matrix3_json(args.homography)
        initial_src = src_points_from_homography(H0, bev_w, bev_h)
    else:
        margin_x = int(fw * 0.1)
        margin_y = int(fh * 0.15)
        initial_src = np.array(
            [
                [margin_x, fh - margin_y],
                [fw - margin_x, fh - margin_y],
                [fw - margin_x * 2, margin_y * 2],
                [margin_x * 2, margin_y * 2],
            ],
            dtype=np.float32,
        )

    default_src_pts = initial_src.copy()
    ui = TuneUI(bev_w, bev_h, initial_src, args.bev_tweak_json, args.bev_post_matrix)
    H_saved_file = load_matrix3_json(args.homography) if args.homography else None

    win = "BEV tune (src | BEV)"
    cv2.namedWindow(win, cv2.WINDOW_NORMAL)
    ui.setup_trackbars(win)

    state = {"paused": False, "frame": frame0}

    def on_mouse(event: int, x: int, y: int, _flags: int, _p: object) -> None:
        if x >= fw:
            return
        if event == cv2.EVENT_LBUTTONDOWN:
            idx = ui.nearest_point(float(x), float(y))
            if idx >= 0:
                ui.drag_idx = idx
        elif event == cv2.EVENT_MOUSEMOVE and ui.drag_idx >= 0:
            ui.src_pts[ui.drag_idx][0] = float(np.clip(x, 0, fw - 1))
            ui.src_pts[ui.drag_idx][1] = float(np.clip(y, 0, fh - 1))
        elif event == cv2.EVENT_LBUTTONUP:
            ui.drag_idx = -1

    cv2.setMouseCallback(win, on_mouse)

    infer_w = args.infer_width
    infer_h = args.infer_height
    send_w = args.send_width if args.send_width is not None else infer_w
    send_h = args.send_height if args.send_height is not None else infer_h

    inference_host = args.result_host if args.result_host is not None else args.board_host

    inf_buffer: Optional[InferenceBuffer] = None
    json_log: Optional[JsonLogReader] = None
    stop_inf = threading.Event()
    npu_video: Optional[NpuVideoSender] = None
    lane_state = BevLaneState()

    if args.board_host:
        npu_video = NpuVideoSender(args.board_host, args.video_port, send_w, send_h)

    if inference_host and not args.json_log:
        inf_buffer = InferenceBuffer(sync_by_index=args.sync_frame_index)
        inf_thread = threading.Thread(
            target=tcp_json_receiver,
            args=(inference_host, args.result_port, inf_buffer, stop_inf),
            daemon=True,
        )
        inf_thread.start()
        print(f"[bev_tune] inference JSON TCP {inference_host}:{args.result_port}")
    elif args.json_log:
        json_log = JsonLogReader(args.json_log)
        print(f"[bev_tune] inference log {args.json_log} ({len(json_log._lines)} lines)")

    has_inference = inf_buffer is not None or json_log is not None
    send_video = npu_video is not None

    qt_server: Optional[QtBroadcastServer] = None
    if args.qt_port > 0:
        qt_server = QtBroadcastServer(args.qt_host, args.qt_port)
        qt_server.start()
        print(
            f"[bev_tune] Qt TCP {args.qt_host}:{args.qt_port} "
            f"(D3-G: same JSON as bev_lane_bridge; set config server_ip to this PC)"
        )
    qt_broadcast = qt_server is not None

    print(f"[bev_tune] {src_label}  {fw}x{fh}  BEV {bev_w}x{bev_h}  infer_scale {infer_w}x{infer_h}")
    if send_video:
        print(f"[bev_tune] NPU video stream {send_w}x{send_h} -> {args.board_host}:{args.video_port}")
    print("[bev_tune] s: save homography + tweak JSON   q: quit")

    frame_idx = -1
    last_json_payload: Optional[Dict[str, Any]] = None

    while True:
        if not state["paused"]:
            ok, frame = cap.read()
            if not ok:
                cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                if json_log is not None:
                    json_log.rewind()
                frame_idx = -1
                ok, frame = cap.read()
                if not ok:
                    break
            state["frame"] = frame
            frame_idx += 1
            if npu_video is not None:
                npu_video.send_bgr(frame)
        frame = state["frame"]

        payload: Optional[Dict[str, Any]] = None
        if inf_buffer is not None:
            payload = inf_buffer.get_for_frame(frame_idx)
        elif json_log is not None:
            if not state["paused"]:
                payload = json_log.next_payload()
                if payload is None:
                    cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                    json_log.rewind()
                    frame_idx = -1
                    payload = json_log.next_payload()
                last_json_payload = payload
            else:
                payload = last_json_payload

        H = ui.current_homography()
        try:
            bev = cv2.warpPerspective(frame, H, (bev_w, bev_h), flags=cv2.INTER_LINEAR)
        except cv2.error:
            bev = np.zeros((bev_h, bev_w, 3), dtype=np.uint8)
        bev = apply_correction_to_bev_image(bev, ui.current_correction())

        left = frame.copy()
        for i, (px, py) in enumerate(ui.src_pts):
            cv2.circle(left, (int(px), int(py)), 8, (0, 255, 255), -1)
            cv2.putText(left, str(i), (int(px) + 10, int(py) - 6), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
        pts_i = ui.src_pts.astype(np.int32).reshape(-1, 1, 2)
        cv2.polylines(left, [pts_i], True, (0, 200, 255), 2)

        if payload:
            draw_inference_on_image(left, payload, fw, fh)
            preview_payload = build_bev_payload(
                payload,
                H,
                bev_w,
                bev_h,
                infer_w,
                infer_h,
                True,
                args.qt_max_objects,
                ui.current_correction(),
                args.lane_fit_degree,
                args.lane_fit_samples,
                args.lane_fit_min_points,
                args.lane_top_ratio,
                args.lane_bottom_ratio,
                lane_state,
                args.lane_smoothing,
            )
            draw_qt_payload_on_bev(bev, preview_payload)

        if qt_server is not None and payload is not None and not state["paused"]:
            qt_payload = build_bev_payload(
                payload,
                H,
                bev_w,
                bev_h,
                infer_w,
                infer_h,
                True,
                args.qt_max_objects,
                ui.current_correction(),
                args.lane_fit_degree,
                args.lane_fit_samples,
                args.lane_fit_min_points,
                args.lane_top_ratio,
                args.lane_bottom_ratio,
                lane_state,
                args.lane_smoothing,
            )
            qt_server.broadcast(qt_payload)

        draw_ui_overlay(left, state["paused"], has_inference, send_video, qt_broadcast)

        if bev.shape[0] != left.shape[0]:
            scale = left.shape[0] / float(bev.shape[0])
            bw = int(round(bev.shape[1] * scale))
            bev_show = cv2.resize(bev, (bw, left.shape[0]), interpolation=cv2.INTER_LINEAR)
        else:
            bev_show = bev

        combo = np.hstack([left, bev_show])
        cv2.imshow(win, combo)

        key = cv2.waitKey(30 if not state["paused"] else 100) & 0xFF
        if key in (27, ord("q")):
            break
        if key == ord(" "):
            state["paused"] = not state["paused"]
        if key == ord("r"):
            if H_saved_file is not None:
                ui.src_pts = src_points_from_homography(H_saved_file, bev_w, bev_h)
            else:
                ui.src_pts = default_src_pts.copy()
            print("[bev_tune] reset src quad")
        if key == ord("s"):
            H_out = ui.current_homography()
            corr = ui.current_correction()
            hom_payload = {"homography": H_out.astype(float).tolist()}
            Path(args.save_homography).write_text(json.dumps(hom_payload, indent=2), encoding="utf-8")
            tweak_payload = {
                "offset_x": float(corr.offset_x),
                "offset_y": float(corr.offset_y),
                "scale_x": float(corr.scale_x),
                "scale_y": float(corr.scale_y),
                "pivot_x": float(corr.pivot_x),
                "pivot_y": float(corr.pivot_y),
            }
            Path(args.save_tweak).write_text(json.dumps(tweak_payload, indent=2), encoding="utf-8")
            if corr.post_matrix is not None:
                post_payload = {"matrix": corr.post_matrix.astype(float).tolist()}
                Path(args.save_post_matrix).write_text(json.dumps(post_payload, indent=2), encoding="utf-8")
                print(f"[bev_tune] wrote {args.save_post_matrix}")
            print(f"[bev_tune] wrote {args.save_homography} , {args.save_tweak}")

    stop_inf.set()
    if qt_server is not None:
        qt_server.stop()
    if npu_video is not None:
        npu_video.close()
    cap.release()
    cv2.destroyAllWindows()
    return 0


if __name__ == "__main__":
    sys.exit(main())
