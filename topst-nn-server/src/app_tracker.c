#include "app_tracker.h"

#include <float.h>
#include <math.h>
#include <string.h>

#define SORT_STATE_DIM 7
#define SORT_MEAS_DIM 4
#define TRACK_ID_MIN 1
#define TRACK_ID_MAX 10000

typedef struct {
    int cls;
    float score;
    float x1;
    float y1;
    float x2;
    float y2;
    float cx;
    float cy;
    float w;
    float h;
    float area;
    float ratio;
} tracker_detection_t;

typedef struct {
    int det_idx;
    int trk_idx;
} tracker_match_t;

static int tracker_track_id_in_use(const sort_tracker_t *tracker, int track_id)
{
    int i;

    for (i = 0; i < APP_MAX_TRACKS; ++i) {
        const sort_track_t *track = &tracker->tracks[i];

        if (track->active && track->track_id == track_id) {
            return 1;
        }
    }

    return 0;
}

static int tracker_allocate_track_id(sort_tracker_t *tracker)
{
    int attempts;
    int candidate = tracker->next_track_id;

    if (candidate < TRACK_ID_MIN || candidate > TRACK_ID_MAX) {
        candidate = TRACK_ID_MIN;
    }

    for (attempts = 0; attempts < TRACK_ID_MAX; ++attempts) {
        if (!tracker_track_id_in_use(tracker, candidate)) {
            tracker->next_track_id = candidate + 1;
            if (tracker->next_track_id > TRACK_ID_MAX) {
                tracker->next_track_id = TRACK_ID_MIN;
            }
            return candidate;
        }

        candidate++;
        if (candidate > TRACK_ID_MAX) {
            candidate = TRACK_ID_MIN;
        }
    }

    return 0;
}

static float tracker_maxf(float a, float b)
{
    return (a > b) ? a : b;
}

static float tracker_minf(float a, float b)
{
    return (a < b) ? a : b;
}

static void tracker_bbox_from_track(const sort_track_t *track,
                                    float *x1, float *y1, float *x2, float *y2)
{
    float area = track->area;
    float ratio = track->ratio;
    float w;
    float h;

    if (area < 1.0f) {
        area = 1.0f;
    }
    if (ratio < 0.05f) {
        ratio = 0.05f;
    }

    w = sqrtf(area * ratio);
    if (!isfinite(w) || w < 1.0f) {
        w = 1.0f;
    }
    h = area / w;
    if (!isfinite(h) || h < 1.0f) {
        h = 1.0f;
    }

    *x1 = track->cx - w * 0.5f;
    *y1 = track->cy - h * 0.5f;
    *x2 = track->cx + w * 0.5f;
    *y2 = track->cy + h * 0.5f;
}

static float tracker_iou_xyxy(float ax1, float ay1, float ax2, float ay2,
                              float bx1, float by1, float bx2, float by2)
{
    float xx1 = tracker_maxf(ax1, bx1);
    float yy1 = tracker_maxf(ay1, by1);
    float xx2 = tracker_minf(ax2, bx2);
    float yy2 = tracker_minf(ay2, by2);
    float w = tracker_maxf(0.0f, xx2 - xx1);
    float h = tracker_maxf(0.0f, yy2 - yy1);
    float inter = w * h;
    float area_a = tracker_maxf(0.0f, ax2 - ax1) * tracker_maxf(0.0f, ay2 - ay1);
    float area_b = tracker_maxf(0.0f, bx2 - bx1) * tracker_maxf(0.0f, by2 - by1);
    float denom = area_a + area_b - inter;

    if (denom <= 0.0f) {
        return 0.0f;
    }
    return inter / denom;
}

static void tracker_kalman_init(sort_track_t *track)
{
    int r;
    int c;

    memset(track->covariance, 0, sizeof(track->covariance));
    for (r = 0; r < SORT_STATE_DIM; ++r) {
        for (c = 0; c < SORT_STATE_DIM; ++c) {
            track->covariance[r][c] = 0.0f;
        }
    }

    track->covariance[0][0] = 10.0f;
    track->covariance[1][1] = 10.0f;
    track->covariance[2][2] = 10.0f;
    track->covariance[3][3] = 10.0f;
    track->covariance[4][4] = 1000.0f;
    track->covariance[5][5] = 1000.0f;
    track->covariance[6][6] = 1000.0f;
}

static void tracker_kalman_predict(sort_track_t *track)
{
    static const float F[SORT_STATE_DIM][SORT_STATE_DIM] = {
        {1, 0, 0, 0, 1, 0, 0},
        {0, 1, 0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 0, 1},
        {0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 1},
    };
    static const float Q[SORT_STATE_DIM] = {1.0f, 1.0f, 10.0f, 0.01f, 0.01f, 0.01f, 0.001f};
    float p_tmp[SORT_STATE_DIM][SORT_STATE_DIM];
    float p_new[SORT_STATE_DIM][SORT_STATE_DIM];
    int r;
    int c;
    int k;

    /* 현재 detection과 매칭하기 전에 다음 박스 상태를 먼저 예측한다. */
    if ((track->area + track->v_area) <= 1.0f) {
        track->v_area = 0.0f;
    }

    track->cx += track->v_cx;
    track->cy += track->v_cy;
    track->area += track->v_area;
    if (track->area < 1.0f) {
        track->area = 1.0f;
    }

    for (r = 0; r < SORT_STATE_DIM; ++r) {
        for (c = 0; c < SORT_STATE_DIM; ++c) {
            p_tmp[r][c] = 0.0f;
            for (k = 0; k < SORT_STATE_DIM; ++k) {
                p_tmp[r][c] += F[r][k] * track->covariance[k][c];
            }
        }
    }

    for (r = 0; r < SORT_STATE_DIM; ++r) {
        for (c = 0; c < SORT_STATE_DIM; ++c) {
            p_new[r][c] = 0.0f;
            for (k = 0; k < SORT_STATE_DIM; ++k) {
                p_new[r][c] += p_tmp[r][k] * F[c][k];
            }
            if (r == c) {
                p_new[r][c] += Q[r];
            }
        }
    }

    memcpy(track->covariance, p_new, sizeof(p_new));
}

static void tracker_invert_4x4(const float in[4][4], float out[4][4])
{
    float aug[4][8];
    int r;
    int c;
    int k;

    for (r = 0; r < 4; ++r) {
        for (c = 0; c < 4; ++c) {
            aug[r][c] = in[r][c];
            aug[r][c + 4] = (r == c) ? 1.0f : 0.0f;
        }
    }

    for (c = 0; c < 4; ++c) {
        int pivot = c;
        float best = fabsf(aug[c][c]);

        for (r = c + 1; r < 4; ++r) {
            float v = fabsf(aug[r][c]);
            if (v > best) {
                best = v;
                pivot = r;
            }
        }

        if (best < 1e-6f) {
            for (r = 0; r < 4; ++r) {
                for (k = 0; k < 4; ++k) {
                    out[r][k] = (r == k) ? 1.0f : 0.0f;
                }
            }
            return;
        }

        if (pivot != c) {
            for (k = 0; k < 8; ++k) {
                float tmp = aug[c][k];
                aug[c][k] = aug[pivot][k];
                aug[pivot][k] = tmp;
            }
        }

        {
            float div = aug[c][c];
            for (k = 0; k < 8; ++k) {
                aug[c][k] /= div;
            }
        }

        for (r = 0; r < 4; ++r) {
            if (r == c) {
                continue;
            }
            {
                float factor = aug[r][c];
                for (k = 0; k < 8; ++k) {
                    aug[r][k] -= factor * aug[c][k];
                }
            }
        }
    }

    for (r = 0; r < 4; ++r) {
        for (c = 0; c < 4; ++c) {
            out[r][c] = aug[r][c + 4];
        }
    }
}

static void tracker_kalman_update(sort_track_t *track, const tracker_detection_t *det)
{
    float x[SORT_STATE_DIM] = {
        track->cx, track->cy, track->area, track->ratio,
        track->v_cx, track->v_cy, track->v_area
    };
    float z[SORT_MEAS_DIM] = {det->cx, det->cy, det->area, det->ratio};
    float y[SORT_MEAS_DIM];
    float s[4][4];
    float s_inv[4][4];
    float k_gain[SORT_STATE_DIM][4];
    float i_kh[SORT_STATE_DIM][SORT_STATE_DIM];
    float p_new[SORT_STATE_DIM][SORT_STATE_DIM];
    static const float R[SORT_MEAS_DIM] = {1.0f, 1.0f, 10.0f, 0.1f};
    int r;
    int c;
    int k;

    for (r = 0; r < SORT_MEAS_DIM; ++r) {
        y[r] = z[r] - x[r];
    }

    for (r = 0; r < SORT_MEAS_DIM; ++r) {
        for (c = 0; c < SORT_MEAS_DIM; ++c) {
            s[r][c] = track->covariance[r][c];
            if (r == c) {
                s[r][c] += R[r];
            }
        }
    }

    tracker_invert_4x4(s, s_inv);

    for (r = 0; r < SORT_STATE_DIM; ++r) {
        for (c = 0; c < SORT_MEAS_DIM; ++c) {
            k_gain[r][c] = 0.0f;
            for (k = 0; k < SORT_MEAS_DIM; ++k) {
                k_gain[r][c] += track->covariance[r][k] * s_inv[k][c];
            }
        }
    }

    for (r = 0; r < SORT_STATE_DIM; ++r) {
        for (k = 0; k < SORT_MEAS_DIM; ++k) {
            x[r] += k_gain[r][k] * y[k];
        }
    }

    for (r = 0; r < SORT_STATE_DIM; ++r) {
        for (c = 0; c < SORT_STATE_DIM; ++c) {
            float kh = 0.0f;

            if (c < SORT_MEAS_DIM) {
                kh = k_gain[r][c];
            }
            i_kh[r][c] = ((r == c) ? 1.0f : 0.0f) - kh;
        }
    }

    for (r = 0; r < SORT_STATE_DIM; ++r) {
        for (c = 0; c < SORT_STATE_DIM; ++c) {
            p_new[r][c] = 0.0f;
            for (k = 0; k < SORT_STATE_DIM; ++k) {
                p_new[r][c] += i_kh[r][k] * track->covariance[k][c];
            }
        }
    }

    track->cx = x[0];
    track->cy = x[1];
    track->area = (x[2] < 1.0f) ? 1.0f : x[2];
    track->ratio = (x[3] < 0.05f) ? 0.05f : x[3];
    track->v_cx = x[4];
    track->v_cy = x[5];
    track->v_area = x[6];
    memcpy(track->covariance, p_new, sizeof(p_new));
}

static void tracker_spawn(sort_tracker_t *tracker, const tracker_detection_t *det)
{
    int i;
    int track_id = tracker_allocate_track_id(tracker);

    if (track_id < TRACK_ID_MIN) {
        return;
    }

    for (i = 0; i < APP_MAX_TRACKS; ++i) {
        sort_track_t *track = &tracker->tracks[i];

        if (track->active) {
            continue;
        }

        memset(track, 0, sizeof(*track));
        track->active = 1;
        track->track_id = track_id;
        track->cls = det->cls;
        track->score = det->score;
        track->cx = det->cx;
        track->cy = det->cy;
        track->area = det->area;
        track->ratio = det->ratio;
        track->hits = 0;
        track->hit_streak = 0;
        track->age = 0;
        track->time_since_update = 0;
        tracker_kalman_init(track);
        return;
    }
}

static int tracker_collect_detections(app_context_t *app, const model_context_t *model,
                                      tracker_detection_t *detections, int max_detections)
{
    int count = 0;
    int i;
    int det_count = model->det_result.cnt;

    if (det_count < 0) {
        det_count = 0;
    }
    if (det_count > APP_MAX_TRACKS) {
        det_count = APP_MAX_TRACKS;
    }

    for (i = 0; i < det_count && count < max_detections; ++i) {
        const enlight_obj_t *obj = &model->det_result.obj[i];
        float src_w = (obj->img_w > 0) ? (float)obj->img_w : (float)app->camera_width;
        float src_h = (obj->img_h > 0) ? (float)obj->img_h : (float)app->camera_height;
        float scale_x = (float)app->camera_width / src_w;
        float scale_y = (float)app->camera_height / src_h;
        float x1 = obj->x_min * scale_x;
        float y1 = obj->y_min * scale_y;
        float x2 = obj->x_max * scale_x;
        float y2 = obj->y_max * scale_y;
        float w = x2 - x1;
        float h = y2 - y1;

        if (w < 2.0f || h < 2.0f) {
            continue;
        }

        detections[count].cls = obj->cls;
        detections[count].score = obj->score;
        detections[count].x1 = x1;
        detections[count].y1 = y1;
        detections[count].x2 = x2;
        detections[count].y2 = y2;
        detections[count].cx = x1 + w * 0.5f;
        detections[count].cy = y1 + h * 0.5f;
        detections[count].w = w;
        detections[count].h = h;
        detections[count].area = w * h;
        detections[count].ratio = w / tracker_maxf(h, 1.0f);
        count++;
    }

    return count;
}

static void tracker_hungarian(const float cost[APP_MAX_TRACKS][APP_MAX_TRACKS],
                              int rows, int cols, int *assignment)
{
    static float u[APP_MAX_TRACKS + 1];
    static float v[APP_MAX_TRACKS + 1];
    static int p[APP_MAX_TRACKS + 1];
    static int way[APP_MAX_TRACKS + 1];
    int i;

    for (i = 0; i <= cols; ++i) {
        u[i] = 0.0f;
        v[i] = 0.0f;
        p[i] = 0;
        way[i] = 0;
    }
    for (i = 0; i < rows; ++i) {
        assignment[i] = -1;
    }

    for (i = 1; i <= rows; ++i) {
        static float minv[APP_MAX_TRACKS + 1];
        static unsigned char used[APP_MAX_TRACKS + 1];
        int j;
        int j0 = 0;

        p[0] = i;
        for (j = 0; j <= cols; ++j) {
            minv[j] = FLT_MAX;
            used[j] = 0;
        }

        do {
            int i0 = p[j0];
            int j1 = 0;
            float delta = FLT_MAX;

            used[j0] = 1;
            for (j = 1; j <= cols; ++j) {
                float cur;

                if (used[j]) {
                    continue;
                }

                cur = cost[i0 - 1][j - 1] - u[i0] - v[j];
                if (cur < minv[j]) {
                    minv[j] = cur;
                    way[j] = j0;
                }
                if (minv[j] < delta) {
                    delta = minv[j];
                    j1 = j;
                }
            }

            for (j = 0; j <= cols; ++j) {
                if (used[j]) {
                    u[p[j]] += delta;
                    v[j] -= delta;
                } else {
                    minv[j] -= delta;
                }
            }
            j0 = j1;
        } while (p[j0] != 0);

        do {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0 != 0);
    }

    for (i = 1; i <= cols; ++i) {
        if (p[i] != 0 && p[i] <= rows) {
            assignment[p[i] - 1] = i - 1;
        }
    }
}

static void tracker_associate(const tracker_detection_t *detections, int detection_count,
                              const float tracker_boxes[APP_MAX_TRACKS][4], const int *tracker_classes,
                              int tracker_count, float iou_threshold,
                              tracker_match_t *matches, int *match_count,
                              int *unmatched_dets, int *unmatched_det_count,
                              int *unmatched_trks, int *unmatched_trk_count)
{
    static float cost[APP_MAX_TRACKS][APP_MAX_TRACKS];
    static float iou_matrix[APP_MAX_TRACKS][APP_MAX_TRACKS];
    static float square_cost[APP_MAX_TRACKS][APP_MAX_TRACKS];
    static int assignment[APP_MAX_TRACKS];
    int matched_dets[APP_MAX_TRACKS];
    int matched_trks[APP_MAX_TRACKS];
    int r;
    int c;

    *match_count = 0;
    *unmatched_det_count = 0;
    *unmatched_trk_count = 0;
    memset(matched_dets, 0, sizeof(matched_dets));
    memset(matched_trks, 0, sizeof(matched_trks));

    if (tracker_count == 0) {
        for (r = 0; r < detection_count; ++r) {
            unmatched_dets[(*unmatched_det_count)++] = r;
        }
        return;
    }

    /* 클래스가 같은 경우에만 IoU 비용 행렬을 만들고, 그 위에서 1:1 매칭을 푼다. */
    for (r = 0; r < detection_count; ++r) {
        for (c = 0; c < tracker_count; ++c) {
            float iou = 0.0f;

            if (detections[r].cls == tracker_classes[c]) {
                iou = tracker_iou_xyxy(detections[r].x1, detections[r].y1,
                                       detections[r].x2, detections[r].y2,
                                       tracker_boxes[c][0], tracker_boxes[c][1],
                                       tracker_boxes[c][2], tracker_boxes[c][3]);
            }
            iou_matrix[r][c] = iou;
            cost[r][c] = -iou;
        }
    }

    {
        int n = (detection_count > tracker_count) ? detection_count : tracker_count;

        for (r = 0; r < n; ++r) {
            for (c = 0; c < n; ++c) {
                square_cost[r][c] = 0.0f;
                if (r < detection_count && c < tracker_count) {
                    square_cost[r][c] = cost[r][c];
                }
            }
        }

        tracker_hungarian(square_cost, n, n, assignment);
    }

    for (r = 0; r < detection_count; ++r) {
        int trk_idx = assignment[r];

        if (trk_idx >= 0 && trk_idx < tracker_count &&
            iou_matrix[r][trk_idx] >= iou_threshold) {
            matches[*match_count].det_idx = r;
            matches[*match_count].trk_idx = trk_idx;
            matched_dets[r] = 1;
            matched_trks[trk_idx] = 1;
            (*match_count)++;
        }
    }

    for (r = 0; r < detection_count; ++r) {
        if (!matched_dets[r]) {
            unmatched_dets[(*unmatched_det_count)++] = r;
        }
    }
    for (c = 0; c < tracker_count; ++c) {
        if (!matched_trks[c]) {
            unmatched_trks[(*unmatched_trk_count)++] = c;
        }
    }
}

static void tracker_publish_results(model_context_t *model, const sort_tracker_t *tracker)
{
    int i;

    memset(&model->tracked_result, 0, sizeof(model->tracked_result));
    for (i = APP_MAX_TRACKS - 1; i >= 0; --i) {
        const sort_track_t *track = &tracker->tracks[i];
        tracked_object_t *out;
        float x1;
        float y1;
        float x2;
        float y2;

        if (!track->active) {
            continue;
        }
        if (!(track->time_since_update < 1 &&
              (track->hit_streak >= tracker->min_hits ||
               tracker->frame_count <= tracker->min_hits))) {
            continue;
        }
        if (model->tracked_result.count >= APP_MAX_TRACKS) {
            break;
        }

        tracker_bbox_from_track(track, &x1, &y1, &x2, &y2);
        out = &model->tracked_result.objects[model->tracked_result.count++];
        out->track_id = track->track_id;
        out->cls = track->cls;
        out->score = track->score;
        out->x_min = x1;
        out->y_min = y1;
        out->x_max = x2;
        out->y_max = y2;
    }
}

void app_tracker_init(sort_tracker_t *tracker)
{
    if (tracker == NULL) {
        return;
    }

    memset(tracker, 0, sizeof(*tracker));
    tracker->next_track_id = TRACK_ID_MIN;
    tracker->frame_count = 0;
    tracker->max_age = 15;
    tracker->min_hits = 2;
    tracker->iou_threshold = 0.3f;
}

void app_tracker_reset(sort_tracker_t *tracker)
{
    app_tracker_init(tracker);
}

void app_tracker_update(app_context_t *app)
{
    int model_idx;

    for (model_idx = 0; model_idx < APP_MAX_MODELS; ++model_idx) {
        model_context_t *model = &app->models[model_idx];
        sort_tracker_t *tracker = &app->trackers[model_idx];
        tracker_detection_t detections[APP_MAX_TRACKS];
        float tracker_boxes[APP_MAX_TRACKS][4];
        int tracker_indices[APP_MAX_TRACKS];
        int tracker_classes[APP_MAX_TRACKS];
        tracker_match_t matches[APP_MAX_TRACKS];
        int unmatched_dets[APP_MAX_TRACKS];
        int unmatched_trks[APP_MAX_TRACKS];
        int detection_count;
        int tracker_count = 0;
        int match_count = 0;
        int unmatched_det_count = 0;
        int unmatched_trk_count = 0;
        int i;

        if (model->post_type != TELECHIPS_NPU_POST_DETECTOR) {
            memset(&model->tracked_result, 0, sizeof(model->tracked_result));
            continue;
        }

        /* 트래킹은 detector 모델에만 적용되며, lane/classifier 결과는 이 단계를 건너뛴다. */
        tracker->frame_count++;
        detection_count = tracker_collect_detections(app, model, detections, APP_MAX_TRACKS);

        for (i = 0; i < APP_MAX_TRACKS; ++i) {
            sort_track_t *track = &tracker->tracks[i];
            float x1;
            float y1;
            float x2;
            float y2;

            if (!track->active) {
                continue;
            }

            tracker_kalman_predict(track);
            track->age++;
            if (track->time_since_update > 0) {
                track->hit_streak = 0;
            }
            track->time_since_update++;

            tracker_bbox_from_track(track, &x1, &y1, &x2, &y2);
            if (!isfinite(x1) || !isfinite(y1) || !isfinite(x2) || !isfinite(y2)) {
                memset(track, 0, sizeof(*track));
                continue;
            }

            tracker_boxes[tracker_count][0] = x1;
            tracker_boxes[tracker_count][1] = y1;
            tracker_boxes[tracker_count][2] = x2;
            tracker_boxes[tracker_count][3] = y2;
            tracker_classes[tracker_count] = track->cls;
            tracker_indices[tracker_count] = i;
            tracker_count++;
        }

        tracker_associate(detections, detection_count,
                          tracker_boxes, tracker_classes, tracker_count,
                          tracker->iou_threshold,
                          matches, &match_count,
                          unmatched_dets, &unmatched_det_count,
                          unmatched_trks, &unmatched_trk_count);

        for (i = 0; i < match_count; ++i) {
            sort_track_t *track = &tracker->tracks[tracker_indices[matches[i].trk_idx]];

            tracker_kalman_update(track, &detections[matches[i].det_idx]);
            track->time_since_update = 0;
            track->hits++;
            track->hit_streak++;
            track->score = detections[matches[i].det_idx].score;
            track->cls = detections[matches[i].det_idx].cls;
        }

        for (i = 0; i < unmatched_det_count; ++i) {
            tracker_spawn(tracker, &detections[unmatched_dets[i]]);
        }

        for (i = 0; i < tracker_count; ++i) {
            sort_track_t *track = &tracker->tracks[tracker_indices[i]];

            if (track->active && track->time_since_update > tracker->max_age) {
                memset(track, 0, sizeof(*track));
            }
        }

        tracker_publish_results(model, tracker);
    }
}
