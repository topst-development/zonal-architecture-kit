from __future__ import annotations

import json
from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Tuple

import cv2
import numpy as np


@dataclass
class BevCorrection:
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


def load_matrix3_json(path: str) -> np.ndarray:
    raw = json.loads(open(path, "r", encoding="utf-8").read())
    matrix = raw.get("matrix", raw.get("homography", raw))
    arr = np.asarray(matrix, dtype=np.float32)
    if arr.shape != (3, 3):
        raise ValueError(f"expected 3x3 matrix, got {arr.shape}")
    return arr


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
    values: Dict[str, float] = {
        "offset_x": 0.0,
        "offset_y": 0.0,
        "scale_x": 1.0,
        "scale_y": 1.0,
        "pivot_x": bev_w / 2.0,
        "pivot_y": bev_h / 2.0,
    }
    if tweak_json_path:
        tweak = json.loads(open(tweak_json_path, "r", encoding="utf-8").read())
        for key in values:
            if key in tweak:
                values[key] = float(tweak[key])
    if offset_x is not None:
        values["offset_x"] = offset_x
    if offset_y is not None:
        values["offset_y"] = offset_y
    if scale_x is not None:
        values["scale_x"] = scale_x
    if scale_y is not None:
        values["scale_y"] = scale_y
    if pivot_x is not None:
        values["pivot_x"] = pivot_x
    if pivot_y is not None:
        values["pivot_y"] = pivot_y

    post_matrix = load_matrix3_json(post_matrix_path) if post_matrix_path else None
    return BevCorrection(
        post_matrix,
        values["offset_x"],
        values["offset_y"],
        values["scale_x"],
        values["scale_y"],
        values["pivot_x"],
        values["pivot_y"],
    )


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
    return [{"x": int(round(float(x_val))), "y": int(round(float(y_val)))} for x_val, y_val in zip(sample_xs, sample_ys)]


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
        fitted_set[lane_idx] = fit_lane_curve(lane_points, degree, sample_count, min_points, top_y, bottom_y)
    return fitted_set


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
    output: List[Dict[str, int]] = []
    for x, y in dst:
        ix, iy = correction.apply_xy(float(x), float(y)) if correction is not None else (int(round(x)), int(round(y)))
        output.append({"x": ix, "y": iy})
    return output


def transform_object_foot(
    obj: Dict[str, Any],
    homography: np.ndarray,
    src_w: int,
    src_h: int,
    dst_w: int,
    dst_h: int,
    correction: Optional[BevCorrection] = None,
) -> Dict[str, Any]:
    output = dict(obj)
    x_min = float(obj.get("x_min", 0.0))
    x_max = float(obj.get("x_max", 0.0))
    y_max = float(obj.get("y_max", 0.0))
    foot = [{"x": 0.5 * (x_min + x_max), "y": y_max}]
    point = transform_points(foot, homography, src_w, src_h, dst_w, dst_h, correction)[0]
    output["bev_x"] = point["x"]
    output["bev_y"] = point["y"]
    return output


def build_centerline(
    left_lane: List[Dict[str, int]],
    right_lane: List[Dict[str, int]],
) -> List[Dict[str, int]]:
    if not left_lane or not right_lane:
        return []
    point_count = min(len(left_lane), len(right_lane))
    return [
        {
            "x": int(round((float(left_lane[i]["x"]) + float(right_lane[i]["x"])) * 0.5)),
            "y": int(round((float(left_lane[i]["y"]) + float(right_lane[i]["y"])) * 0.5)),
        }
        for i in range(point_count)
    ]


def choose_lookahead_point(centerline: List[Dict[str, int]], target_y: float) -> Optional[Dict[str, int]]:
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

