from __future__ import annotations

import threading
from typing import Any, Dict, List, Tuple


def smooth_lane_points(
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


def smooth_lane_set(
    previous_set: List[List[Dict[str, int]]],
    current_set: List[List[Dict[str, int]]],
    alpha: float,
) -> List[List[Dict[str, int]]]:
    smoothed_set: List[List[Dict[str, int]]] = [[], [], [], []]
    for lane_idx in range(4):
        previous = previous_set[lane_idx] if lane_idx < len(previous_set) else []
        current = current_set[lane_idx] if lane_idx < len(current_set) else []
        smoothed_set[lane_idx] = smooth_lane_points(previous, current, alpha)
    return smoothed_set


class BevLaneState:
    def __init__(self):
        self.lock = threading.Lock()
        self.smoothed_lanes: List[List[Dict[str, int]]] = [[], [], [], []]
        self.object_tracks: List[Dict[str, Any]] = []
        self.next_track_id = 1
        self.smoothed_steering_deg = 0.0

    def smooth(self, lanes: List[List[Dict[str, int]]], alpha: float) -> List[List[Dict[str, int]]]:
        with self.lock:
            self.smoothed_lanes = smooth_lane_set(self.smoothed_lanes, lanes, alpha)
            return [[dict(point) for point in lane] for lane in self.smoothed_lanes]

    def smooth_steering(self, steering_deg: float, alpha: float) -> float:
        with self.lock:
            self.smoothed_steering_deg = alpha * self.smoothed_steering_deg + (1.0 - alpha) * steering_deg
            return self.smoothed_steering_deg

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
            matches: List[Tuple[int, int, float]] = []

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

