#define _DEFAULT_SOURCE

#include "app_json.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

static int append_json(char *buf, size_t buf_size, int *pos, const char *fmt, ...)
{
    va_list ap;
    int written;

    if (buf == NULL || pos == NULL || fmt == NULL) {
        return -1;
    }
    if ((size_t)(*pos) >= buf_size) {
        return -1;
    }

    va_start(ap, fmt);
    written = vsnprintf(buf + *pos, buf_size - (size_t)(*pos), fmt, ap);
    va_end(ap);

    if (written < 0) {
        return -1;
    }

    *pos += written;
    if ((size_t)(*pos) >= buf_size) {
        return -1;
    }

    return 0;
}

static int json_output_start(json_output_context_t *ctx, int port)
{
    struct sockaddr_in serv_addr;
    int opt = 1;
    int flags;

    memset(ctx, 0, sizeof(*ctx));
    ctx->server_fd = -1;
    ctx->client_fd = -1;
    ctx->port = port;

    ctx->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->server_fd < 0) {
        perror("json socket");
        return -1;
    }

    (void)setsockopt(ctx->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    flags = fcntl(ctx->server_fd, F_GETFL, 0);
    if (flags >= 0) {
        (void)fcntl(ctx->server_fd, F_SETFL, flags | O_NONBLOCK);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons((uint16_t)port);

    if (bind(ctx->server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("json bind");
        return -1;
    }
    if (listen(ctx->server_fd, 1) < 0) {
        perror("json listen");
        return -1;
    }

    printf("[json] listening on port %d\n", port);
    return 0;
}

int app_json_init(app_context_t *app)
{
    return json_output_start(&app->json_output, app->json_output.port);
}

void app_json_deinit(app_context_t *app)
{
    if (app->json_output.client_fd >= 0) {
        (void)close(app->json_output.client_fd);
        app->json_output.client_fd = -1;
    }
    if (app->json_output.server_fd >= 0) {
        (void)close(app->json_output.server_fd);
        app->json_output.server_fd = -1;
    }
}

void app_json_poll_accept(app_context_t *app)
{
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int cfd;

    if (app->json_output.server_fd < 0) {
        return;
    }

    cfd = accept(app->json_output.server_fd, (struct sockaddr *)&cli_addr, &cli_len);
    if (cfd < 0) {
        return;
    }

    if (app->json_output.client_fd >= 0) {
        (void)close(app->json_output.client_fd);
    }
    app->json_output.client_fd = cfd;
    printf("[json] client connected: %s:%d\n",
           inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
}

int app_json_send_results(app_context_t *app)
{
    char json_buf[32768];
    int i;
    int pos = 0;
    int first = 1;
    ssize_t sent;

    if (app->json_output.client_fd < 0) {
        return 0;
    }

    /* 프레임 1장의 결과를 줄바꿈(\n)으로 끝나는 JSON 한 줄로 직렬화 */
    if (append_json(json_buf, sizeof(json_buf), &pos,
                    "{\"frame_index\":%llu,\"width\":%u,\"height\":%u,",
                    (unsigned long long)app->frame_index,
                    app->camera_width, app->camera_height) < 0) {
        return -1;
    }

    if (append_json(json_buf, sizeof(json_buf), &pos, "\"objects\":[") < 0) {
        return -1;
    }
    for (i = 0; i < APP_MAX_MODELS; ++i) {
        const model_context_t *model = &app->models[i];
        int j;

        if (model->post_type != TELECHIPS_NPU_POST_DETECTOR) {
            continue;
        }

        for (j = 0; j < model->tracked_result.count; ++j) {
            const tracked_object_t *obj = &model->tracked_result.objects[j];

            if (!first && append_json(json_buf, sizeof(json_buf), &pos, ",") < 0) {
                return -1;
            }
            first = 0;
            if (append_json(json_buf, sizeof(json_buf), &pos,
                            "{\"model\":%d,\"track_id\":%d,\"cls\":%d,\"score\":%.3f,"
                            "\"x_min\":%.1f,\"y_min\":%.1f,\"x_max\":%.1f,\"y_max\":%.1f}",
                            model->index, obj->track_id, obj->cls, obj->score,
                            obj->x_min, obj->y_min, obj->x_max, obj->y_max) < 0) {
                return -1;
            }
        }
    }

    if (append_json(json_buf, sizeof(json_buf), &pos, "],\"lanes\":[") < 0) {
        return -1;
    }

    first = 1;
    for (i = 0; i < APP_MAX_MODELS; ++i) {
        const model_context_t *model = &app->models[i];
        int lane_idx;

        if (model->post_type != TELECHIPS_NPU_POST_CUSTOM) {
            continue;
        }
        if (model->lane_data == NULL) {
            continue;
        }

        for (lane_idx = 0; lane_idx < model->lane_data->num_lanes && lane_idx < MAX_LANES; ++lane_idx) {
            int p;
            int lane_open = 0;
            const lane_polyline_t *ln = &model->lane_data->lane[lane_idx];
            const int lane_src_w = (model->lane_data->img_w > 0) ? model->lane_data->img_w : (int)app->camera_width;
            const int lane_src_h = (model->lane_data->img_h > 0) ? model->lane_data->img_h : (int)app->camera_height;
            /* 차선 후처리 결과는 모델 고유 해상도일 수 있으므로 여기서 카메라 픽셀 기준으로 다시 맞춘다. */
            const float lane_sx = (float)app->camera_width / (float)((lane_src_w > 0) ? lane_src_w : 1);
            const float lane_sy = (float)app->camera_height / (float)((lane_src_h > 0) ? lane_src_h : 1);

            if (!first && append_json(json_buf, sizeof(json_buf), &pos, ",") < 0) {
                return -1;
            }
            first = 0;
            if (append_json(json_buf, sizeof(json_buf), &pos,
                            "{\"model\":%d,\"lane_id\":%d,\"score\":%.3f,\"n\":%d,\"points\":[",
                            model->index, lane_idx, ln->lane_score, ln->n) < 0) {
                return -1;
            }

            for (p = 0; p < ln->n && p < MAX_POINTS; ++p) {
                if (ln->conf[p] >= 0.0f) {
                    int x = (int)(ln->x[p] * lane_sx + 0.5f);
                    int y = (int)(ln->y[p] * lane_sy + 0.5f);
                    if (lane_open && append_json(json_buf, sizeof(json_buf), &pos, ",") < 0) {
                        return -1;
                    }
                    lane_open = 1;
                    if (append_json(json_buf, sizeof(json_buf), &pos,
                                    "{\"x\":%d,\"y\":%d,\"conf\":%.3f}", x, y, ln->conf[p]) < 0) {
                        return -1;
                    }
                }
            }

            if (append_json(json_buf, sizeof(json_buf), &pos, "]}") < 0) {
                return -1;
            }
        }
    }

    if (append_json(json_buf, sizeof(json_buf), &pos,
                    "],\"perf\":{\"fps\":%.2f,\"cpu\":%u,\"mem\":%u}}\n",
                    app->perf.fps, app->perf.cpuUtil[0], app->perf.memUsage) < 0) {
        return -1;
    }

    sent = send(app->json_output.client_fd, json_buf, (size_t)pos, MSG_NOSIGNAL);
    if (sent < 0) {
        perror("json send");
        (void)close(app->json_output.client_fd);
        app->json_output.client_fd = -1;
        return -1;
    }

    return 0;
}
