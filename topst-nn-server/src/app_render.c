#include "app_render.h"

#include <sys/mman.h>

#include "opencv_api.h"

static void overlay_results(app_context_t *app, uint8_t *output_map_base)
{
    static const Color_t colors[APP_MAX_MODELS] = {
        RGB(80, 255, 120),
        RGB(80, 255, 120),
    };
    const int compact_ui = (app->display_width <= 800 || app->display_height <= 480);
    const int left_overlay_margin = compact_ui ? 28 : 36;
    const int right_overlay_margin = compact_ui ? -2 : 6;
    const int perf_line_step = compact_ui ? 22 : 18;
    const int perf_group_gap = compact_ui ? 6 : 2;
    const int perf_column_width = compact_ui ? 125 : 200;
    const double perf_font_size = compact_ui ? 0.4 : 0.8;
    const double box_font_size = compact_ui ? 0.5 : 0.8;
    const int box_label_offset = compact_ui ? 8 : 5;
    const double cls_font_size = compact_ui ? 0.4 : 0.8;
    const int fps_y = compact_ui ? 40 : 36;
    const int cls_start_y = fps_y + perf_line_step + (compact_ui ? 8 : 6);
    const int cls_line_step = compact_ui ? 26 : 30;
    int i;
    int info_x;
    int info_y = fps_y;

    if (output_map_base == NULL || output_map_base == MAP_FAILED) {
        return;
    }

    /* 네트워크별 결과를 한 화면에 겹쳐 그리기 위해 공통 오버레이 좌표를 계산한다. */
    info_x = (int)app->display_width - perf_column_width - right_overlay_margin;
    if (info_x < left_overlay_margin) {
        info_x = left_overlay_margin;
    }

    for (i = 0; i < APP_MAX_MODELS; ++i) {
        const model_context_t *model = &app->models[i];
        Color_t color = colors[i];

        if (model->post_type == TELECHIPS_NPU_POST_DETECTOR) {
            /* detector는 tracker가 보정한 박스를 사용해 화면에 그린다. */
            int j;
            Box_t boxes[256];
            int box_count = model->tracked_result.count;

            if (box_count > 256) {
                box_count = 256;
            }
            for (j = 0; j < box_count; ++j) {
                const tracked_object_t *obj = &model->tracked_result.objects[j];
                boxes[j].cls = obj->cls;
                boxes[j].track_id = obj->track_id;
                boxes[j].score = obj->score;
                boxes[j].xmin = (int)(obj->x_min + 0.5f);
                boxes[j].ymin = (int)(obj->y_min + 0.5f);
                boxes[j].xmax = (int)(obj->x_max + 0.5f);
                boxes[j].ymax = (int)(obj->y_max + 0.5f);
            }
            if (box_count > 0) {
                int box_image_width = (int)app->camera_width;
                int box_image_height = (int)app->camera_height;

                if (box_image_width <= 0) {
                    box_image_width = (int)app->camera_width;
                }
                if (box_image_height <= 0) {
                    box_image_height = (int)app->camera_height;
                }

                cvDrawBoxes(output_map_base, boxes, box_count,
                            app->display_width, app->display_height,
                            (uint32_t)box_image_width,
                            (uint32_t)box_image_height,
                            color, color, box_font_size, box_label_offset);
            }
        } else if (model->post_type == TELECHIPS_NPU_POST_CUSTOM) {
            /* lane 결과는 모델 좌표계를 디스플레이 좌표계로 변환해 선분으로 그린다. */
            static const Color_t lane_colors[6] = {
                RGB(255, 0, 0),
                RGB(0, 255, 0),
                RGB(0, 255, 255),
                RGB(255, 255, 0),
                RGB(255, 0, 255),
                RGB(255, 255, 255),
            };
            const laneaf_result_t *res = model->lane_data;
            int lane_idx;

            if (res == NULL) {
                continue;
            }

            for (lane_idx = 0; lane_idx < res->num_lanes && lane_idx < MAX_LANES; ++lane_idx) {
                const lane_polyline_t *ln = &res->lane[lane_idx];
                Point_t line_start[MAX_POINTS - 1];
                Point_t line_end[MAX_POINTS - 1];
                Color_t lane_color = lane_colors[lane_idx % 6];
                int line_count = 0;
                int prev_valid = 0;
                int prev_x = 0;
                int prev_y = 0;
                int p;

                if (ln->n < 2) {
                    continue;
                }

                for (p = 0; p < ln->n && p < MAX_POINTS; ++p) {
                    int x = (int)((ln->x[p] / (float)res->img_w) * (float)app->display_width + 0.5f);
                    int y = (int)((ln->y[p] / (float)res->img_h) * (float)app->display_height + 0.5f);
                    int clamped_x = x < 0 ? 0 : x;
                    int clamped_y = y < 0 ? 0 : y;

                    if (prev_valid) {
                        line_start[line_count].x = prev_x;
                        line_start[line_count].y = prev_y;
                        line_end[line_count].x = clamped_x;
                        line_end[line_count].y = clamped_y;
                        ++line_count;
                    }
                    prev_x = clamped_x;
                    prev_y = clamped_y;
                    prev_valid = 1;
                }

                if (line_count > 0) {
                    cvDrawLines(output_map_base, app->display_width, app->display_height,
                                line_start, line_end, line_count, lane_color, 3);
                }
            }
        } else if (model->post_type == TELECHIPS_NPU_POST_CLASSIFIER) {
            cvDrawCls(output_map_base, app->display_width, app->display_height,
                      model->cls_result.class_ids[0], left_overlay_margin, cls_start_y + i * cls_line_step,
                      color, cls_font_size);
        }

        cvDrawInfo(output_map_base, app->display_width, app->display_height,
                   DRAW_INFO_NETWORK, model->perf.elapsed_in_us / 1000.0,
                   model->index, info_x, info_y, perf_font_size, color);
        info_y += perf_line_step;
        cvDrawInfo(output_map_base, app->display_width, app->display_height,
                   DRAW_INFO_NPU, model->npuUtilization,
                   model->index, info_x, info_y, perf_font_size, color);
        info_y += perf_line_step + perf_group_gap;
    }

    /* 마지막에 시스템 공통 정보(FPS/CPU/MEM)를 별도 영역에 표시한다. */
    cvDrawInfo(output_map_base, app->display_width, app->display_height,
               DRAW_INFO_FPS, app->perf.fps,
               0, left_overlay_margin, fps_y, perf_font_size, RGB(255, 255, 255));
    cvDrawInfo(output_map_base, app->display_width, app->display_height,
               DRAW_INFO_CPU, app->perf.cpuUtil[0],
               0, info_x, info_y, perf_font_size, RGB(255, 255, 255));
    info_y += perf_line_step;
    cvDrawInfo(output_map_base, app->display_width, app->display_height,
               DRAW_INFO_MEMORY, app->perf.memUsage,
               0, info_x, info_y, perf_font_size, RGB(255, 255, 255));
}

int render_output_frame(app_context_t *app)
{
    int output_idx = app->display_buffer_index % APP_DISPLAY_BUFFER_COUNT;

    if (app->memory.map_base_output[output_idx] == NULL ||
        app->memory.map_base_output[output_idx] == MAP_FAILED) {
        fprintf(stderr, "invalid output buffer mapping: %d\n", output_idx);
        return -1;
    }

    if (app->input_mode == APP_INPUT_CAMERA) {
        /* 카메라 입력은 현재 캡처 버퍼를 그대로 디스플레이 해상도로 축소한다. */
        scaler_image_t src;
        scaler_image_t dst;

        src.paddr = app->camera_phys_addr;
        src.width = app->camera_width;
        src.height = app->camera_height;
        src.format = SCALER_FORMAT_ARGB8888;

        dst.paddr = app->memory.phy_base_output[output_idx];
        dst.width = app->display_width;
        dst.height = app->display_height;
        dst.format = SCALER_FORMAT_RGB888;

        if (scaler_resize(app->scaler, SCALER_INDEX_0, src, dst) != 0) {
            return -1;
        }
        if (scaler_poll(app->scaler, SCALER_INDEX_0) != 0) {
            return -1;
        }
    } else {
        /* TCP 입력은 staging buffer를 기준으로 디스플레이용 프레임을 만든다. */
        scaler_image_t src;
        scaler_image_t dst;

        if (app->tcp_stage_buf == NULL) {
            return -1;
        }

        src.paddr = app->tcp_stage_buf->paddr;
        src.width = app->camera_width;
        src.height = app->camera_height;
        src.format = SCALER_FORMAT_RGB888;

        dst.paddr = app->memory.phy_base_output[output_idx];
        dst.width = app->display_width;
        dst.height = app->display_height;
        dst.format = SCALER_FORMAT_RGB888;

        if (scaler_resize(app->scaler, SCALER_INDEX_0, src, dst) != 0) {
            return -1;
        }
        if (scaler_poll(app->scaler, SCALER_INDEX_0) != 0) {
            return -1;
        }
    }

    /* 축소된 배경 영상 위에 detector/lane/perf 정보를 덧그린다. */
    overlay_results(app, app->memory.map_base_output[output_idx]);

    /* 완성된 출력 버퍼를 overlay/display 장치에 실제로 보여준다. */
    if (display_show(app->display, app->memory.phy_base_output[output_idx], app->display_x,
                     app->display_y, app->display_width, app->display_height) != 0) {
        return -1;
    }

    app->display_buffer_index++;
    return 0;
}
