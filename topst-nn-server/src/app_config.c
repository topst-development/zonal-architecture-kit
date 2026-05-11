#include "app_config.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void app_config_print_usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s -n <model0_dir> -N <model1_dir> [options]\n"
            "  -n <dir>   Cluster 0 model directory\n"
            "  -N <dir>   Cluster 1 model directory\n"
            "  -i <mode>  Input mode: camera|tcp, default: camera\n"
            "  -c <path>  Camera device, default: %s\n"
            "  -d <path>  Display device, default: %s\n"
            "  -p <num>   TCP port for -i tcp, default: %d\n"
            "  -w <num>   Camera width, default: %d\n"
            "  -h <num>   Camera height, default: %d\n"
            "  -W <num>   Display width, default: %d\n"
            "  -H <num>   Display height, default: %d\n"
            "  -x <num>   Display x position, default: 0\n"
            "  -y <num>   Display y position, default: 0\n"
            "  -t <num>   Timeout ms, default: 1000\n"
            "  -j         Enable JSON output server (always enabled in tcp mode)\n"
            "  -v         Verbose perf logging\n"
            "\n"
            "TCP mode expects raw RGB888 frames of size width*height*3 bytes.\n"
            "Press 'x' in the terminal to stop the app cleanly.\n",
            prog, DEFAULT_CAMERA_DEVICE, DEFAULT_DISPLAY_DEVICE,
            DEFAULT_TCP_PORT,
            DEFAULT_CAMERA_WIDTH, DEFAULT_CAMERA_HEIGHT,
            DEFAULT_DISPLAY_WIDTH, DEFAULT_DISPLAY_HEIGHT);
}

void app_config_set_defaults(app_context_t *app)
{
    /* 실행 전 기본 상태를 먼저 비우고, 장치/해상도/포트 기본값을 채움 */
    memset(app, 0, sizeof(*app));

    (void)snprintf(app->camera_device, sizeof(app->camera_device), "%s",
                   DEFAULT_CAMERA_DEVICE);
    (void)snprintf(app->display_device, sizeof(app->display_device), "%s",
                   DEFAULT_DISPLAY_DEVICE);
    (void)snprintf(app->scaler_device[SCALER_INDEX_0],
                   sizeof(app->scaler_device[SCALER_INDEX_0]), "%s",
                   DEFAULT_SCALER0_DEVICE);
    (void)snprintf(app->scaler_device[SCALER_INDEX_1],
                   sizeof(app->scaler_device[SCALER_INDEX_1]), "%s",
                   DEFAULT_SCALER1_DEVICE);

    app->camera_width = DEFAULT_CAMERA_WIDTH;
    app->camera_height = DEFAULT_CAMERA_HEIGHT;
    app->display_width = DEFAULT_DISPLAY_WIDTH;
    app->display_height = DEFAULT_DISPLAY_HEIGHT;
    app->timeout_ms = 1000;
    app->input_mode = APP_INPUT_CAMERA;
    app->json_enabled = 0;
    app->tcp_input.server_fd = -1;
    app->tcp_input.client_fd = -1;
    app->tcp_input.port = DEFAULT_TCP_PORT;
    app->json_output.server_fd = -1;
    app->json_output.client_fd = -1;
    app->json_output.port = DEFAULT_JSON_PORT;

    /* 현재 구현은 모델 2개를 고정으로 사용하므로 index/cluster도 함께 지정 */
    app->models[0].index = 0;
    app->models[0].cluster = 0;
    app->models[1].index = 1;
    app->models[1].cluster = 1;
}

int app_config_parse_args(app_context_t *app, int argc, char **argv)
{
    int opt;
    static const struct option long_options[] = {
        {"json", no_argument, NULL, 'j'},
        {0, 0, 0, 0},
    };

    /* CLI 옵션 - 기본 설정 */
    while ((opt = getopt_long(argc, argv, "n:N:i:c:d:p:w:h:W:H:x:y:t:jv",
                              long_options, NULL)) != -1) {
        switch (opt) {
            case 'n':
                (void)snprintf(app->models[0].path, sizeof(app->models[0].path), "%s", optarg);
                break;
            case 'N':
                (void)snprintf(app->models[1].path, sizeof(app->models[1].path), "%s", optarg);
                break;
            case 'i':
                if (strcmp(optarg, "camera") == 0) {
                    app->input_mode = APP_INPUT_CAMERA;
                } else if (strcmp(optarg, "tcp") == 0) {
                    app->input_mode = APP_INPUT_TCP;
                } else {
                    return -1;
                }
                break;
            case 'c':
                (void)snprintf(app->camera_device, sizeof(app->camera_device), "%s", optarg);
                break;
            case 'd':
                (void)snprintf(app->display_device, sizeof(app->display_device), "%s", optarg);
                break;
            case 'p':
                app->tcp_input.port = atoi(optarg);
                break;
            case 'w':
                app->camera_width = (uint32_t)atoi(optarg);
                break;
            case 'h':
                app->camera_height = (uint32_t)atoi(optarg);
                break;
            case 'W':
                app->display_width = (uint32_t)atoi(optarg);
                break;
            case 'H':
                app->display_height = (uint32_t)atoi(optarg);
                break;
            case 'x':
                app->display_x = (uint32_t)atoi(optarg);
                break;
            case 'y':
                app->display_y = (uint32_t)atoi(optarg);
                break;
            case 't':
                app->timeout_ms = atoi(optarg);
                break;
            case 'j':
                app->json_enabled = 1;
                break;
            case 'v':
                app->verbose = 1;
                break;
            default:
                return -1;
        }
    }

    /* 두 모델 경로는 필수 입력이므로 비어 있으면 fail */
    if (app->models[0].path[0] == '\0' || app->models[1].path[0] == '\0') {
        return -1;
    }

    /* 공통 timeout/verbose 값은 각 모델 설정에도 복사해 사용 */
    app->models[0].timeout_ms = app->timeout_ms;
    app->models[1].timeout_ms = app->timeout_ms;
    app->models[0].verbose = app->verbose;
    app->models[1].verbose = app->verbose;

    /* TCP 입력은 외부 브리지와 연동되므로 JSON 출력도 활성화 */
    if (app->input_mode == APP_INPUT_TCP) {
        app->json_enabled = 1;
    }

    /* 실제 적용된 핵심 입력 설정을 시작 로그에 남김 */
    printf("[input] mode=%s source=%ux%u port=%d json=%s\n",
           input_mode_to_string(app->input_mode),
           app->camera_width, app->camera_height,
           app->tcp_input.port,
           app->json_enabled ? "on" : "off");

    return 0;
}
