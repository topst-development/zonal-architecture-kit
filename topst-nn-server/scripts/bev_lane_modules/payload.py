from __future__ import annotations

from typing import Any, Dict, List, Optional

import numpy as np

from .geometry import (
    BevCorrection,
    compute_pure_pursuit_steering,
    fit_lane_set,
    transform_object_foot,
    transform_points,
)
from .state import BevLaneState


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
    payload_width = int(result.get("width", 800))
    payload_height = int(result.get("height", 480))
    lanes_in = result.get("lanes", []) or []

    bev_by_id: List[List[Dict[str, int]]] = [[], [], [], []]
    for lane in lanes_in:
        lane_id = int(lane.get("lane_id", -1))
        if 0 <= lane_id < 4:
            bev_by_id[lane_id] = transform_points(
                lane.get("points", []) or [],
                homography,
                payload_width,
                payload_height,
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
        objects: List[Dict[str, Any]] = list(result.get("objects", []) or [])
        if max_objects > 0:
            objects = objects[:max_objects]
        bev_objects = [
            transform_object_foot(obj, homography, payload_width, payload_height, source_width, source_height, correction)
            for obj in objects
        ]
        if lane_state is not None:
            bev_objects = lane_state.update_object_tracks(
                bev_objects,
                object_track_distance,
                object_track_confirm,
                object_track_miss,
                object_track_alpha,
                max_objects,
            )
        payload["objects"] = objects
        payload["bev_objects"] = bev_objects
        lane_status = [False, False, False, False, False]
        for obj in bev_objects:
            center_x = float(obj.get("bev_x", 0.0))
            lane_index = int(center_x * 5.0 / max(bev_width, 1))
            lane_status[max(0, min(4, lane_index))] = True
        payload["lane_status"] = lane_status
    else:
        payload["objects"] = []
        payload["bev_objects"] = []
        payload["lane_status"] = [False, False, False, False, False]

    return payload
