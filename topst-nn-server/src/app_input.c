#define _DEFAULT_SOURCE

#include "app_input.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "app_inference.h"
#include "opencv_api.h"

static int recv_all(int fd, uint8_t *buf, size_t size)
{
    size_t received = 0;

    /* TCP는 바이트 스트림이므로 프레임 1장이 여러 recv() 조각으로 나뉘어 들어올 수 있음. */
    while (received < size) {
        ssize_t ret = recv(fd, buf + received, size - received, 0);
        if (ret == 0) {
            return 0;
        }
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        received += (size_t)ret;
    }

    return 1;
}

static int tcp_input_init(tcp_input_context_t *ctx, int port, size_t frame_bytes)
{
    struct sockaddr_in serv_addr;
    int opt = 1;

    memset(ctx, 0, sizeof(*ctx));
    ctx->server_fd = -1;
    ctx->client_fd = -1;
    ctx->port = port;
    ctx->frame_bytes = frame_bytes;

    ctx->frame_buffer = (uint8_t *)malloc(frame_bytes);
    if (ctx->frame_buffer == NULL) {
        fprintf(stderr, "failed to allocate TCP frame buffer (%zu bytes)\n", frame_bytes);
        return -1;
    }

    ctx->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->server_fd < 0) {
        perror("socket");
        return -1;
    }

    (void)setsockopt(ctx->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons((uint16_t)port);

    if (bind(ctx->server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        return -1;
    }
    if (listen(ctx->server_fd, 1) < 0) {
        perror("listen");
        return -1;
    }

    printf("[tcp] listening on port %d for %zubyte RGB frames\n", port, frame_bytes);
    return 0;
}

static int tcp_accept_client(tcp_input_context_t *ctx)
{
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    ctx->client_fd = accept(ctx->server_fd, (struct sockaddr *)&cli_addr, &cli_len);
    if (ctx->client_fd < 0) {
        if (errno == EINTR) {
            return 0;
        }
        perror("accept");
        return -1;
    }

    printf("[tcp] client connected: %s:%d\n",
           inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    return 1;
}

int app_input_init(app_context_t *app)
{
    if (app->input_mode != APP_INPUT_TCP) {
        return 0;
    }

    if (tcp_input_init(&app->tcp_input, app->tcp_input.port,
                       (size_t)app->camera_width * (size_t)app->camera_height * 3u) != 0) {
        return -1;
    }

    app->tcp_stage_buf = buffer_alloc(app->models[0].npu,
                                      (int)((size_t)app->camera_width *
                                            (size_t)app->camera_height * 3u));
    if (app->tcp_stage_buf == NULL) {
        fprintf(stderr, "failed to allocate tcp staging buffer\n");
        return -1;
    }

    return 0;
}

void app_input_deinit(app_context_t *app)
{
    if (app->tcp_input.client_fd >= 0) {
        (void)close(app->tcp_input.client_fd);
        app->tcp_input.client_fd = -1;
    }
    if (app->tcp_input.server_fd >= 0) {
        (void)close(app->tcp_input.server_fd);
        app->tcp_input.server_fd = -1;
    }
    free(app->tcp_input.frame_buffer);
    app->tcp_input.frame_buffer = NULL;
    app->tcp_input.frame_bytes = 0;
}

int acquire_input_frame(app_context_t *app)
{
    if (app->input_mode == APP_INPUT_CAMERA) {
        if (camera_get_buffer(app->camera, &app->camera_virt_addr,
                              &app->camera_phys_addr) <= 0) {
            usleep(1000);
            return 0;
        }
        return 1;
    }

    /* TCP 모드에서는 frame_buffer에 RGB 프레임 1장이 모두 채워질 때까지 대기 */
    for (;;) {
        int ret;

        if (app->stop) {
            return -1;
        }
        if (app->tcp_input.client_fd < 0) {
            ret = tcp_accept_client(&app->tcp_input);
            if (ret < 0) {
                return -1;
            }
            if (ret == 0) {
                continue;
            }
        }

        ret = recv_all(app->tcp_input.client_fd, app->tcp_input.frame_buffer,
                       app->tcp_input.frame_bytes);
        if (ret > 0) {
            return 1;
        }

        (void)close(app->tcp_input.client_fd);
        app->tcp_input.client_fd = -1;
        if (ret < 0) {
            perror("recv");
        } else {
            printf("[tcp] client disconnected\n");
        }
    }
}

void release_input_frame(app_context_t *app)
{
    if (app->input_mode == APP_INPUT_CAMERA && app->camera != NULL) {
        (void)camera_release_buffer(app->camera);
    }
}

int prepare_model_input(app_context_t *app, const model_context_t *model,
                        scaler_index_t scaler_index)
{
    if (app->input_mode == APP_INPUT_CAMERA) {
        scaler_image_t src;
        scaler_image_t dst;

        /* 카메라 입력은 캡처된 물리 버퍼에서 바로 리사이즈할 수 있다. */
        src.paddr = app->camera_phys_addr;
        src.width = app->camera_width;
        src.height = app->camera_height;
        src.format = SCALER_FORMAT_ARGB8888;

        dst.paddr = model->input_buf->paddr;
        dst.width = app_align_width((uint32_t)model->input_width, 16u);
        dst.height = (uint32_t)model->input_height;
        dst.format = SCALER_FORMAT_RGB888;

        if (scaler_resize(app->scaler, scaler_index, src, dst) != 0) {
            return -1;
        }

        return scaler_poll(app->scaler, scaler_index);
    }

    {
        scaler_image_t src;
        scaler_image_t dst;
        unsigned char *stage_addr;

        /*
         * TCP 입력은 일반 호스트 메모리이므로, 하드웨어 스케일러가 읽기 전에
         * NPU가 소유한 staging buffer로 한 번 복사
         */
        if (app->tcp_stage_buf == NULL) {
            return -1;
        }

        stage_addr = (unsigned char *)buffer_get_addr(app->tcp_stage_buf);
        if (stage_addr == NULL) {
            return -1;
        }

        memcpy(stage_addr, app->tcp_input.frame_buffer, app->tcp_input.frame_bytes);

        src.paddr = app->tcp_stage_buf->paddr;
        src.width = app->camera_width;
        src.height = app->camera_height;
        src.format = SCALER_FORMAT_RGB888;

        dst.paddr = model->input_buf->paddr;
        dst.width = app_align_width((uint32_t)model->input_width, 16u);
        dst.height = (uint32_t)model->input_height;
        dst.format = SCALER_FORMAT_RGB888;

        if (scaler_resize(app->scaler, scaler_index, src, dst) != 0) {
            return -1;
        }

        return scaler_poll(app->scaler, scaler_index);
    }
}
