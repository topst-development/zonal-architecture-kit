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
            "  -i <mode>  Input mode: camera|tcp|raw-tcp|vision, default: camera\n"
            "               tcp: Vision Protocol compatibility alias\n"
            "               raw-tcp: legacy raw RGB socket transport\n"
            "  -c <path>  Camera device, default: %s\n"
            "  -d <path>  Display device, default: %s\n"
            "  -p <num>   TCP port for -i raw-tcp, default: %d\n"
            "  --vision-target <ip>  PC/RTPM Vision Protocol server IP, default: %s\n"
            "  --stream-port <num>   Vision stream port, default: %d\n"
            "  --message-port <num>  Vision message port, default: %d\n"
            "  -w <num>   Camera/frame width, default: %d\n"
            "  -h <num>   Camera/frame height, default: %d\n"
            "  -W <num>   Display width, default: %d\n"
            "  -H <num>   Display height, default: %d\n"
            "  -x <num>   Display x position, default: 0\n"
            "  -y <num>   Display y position, default: 0\n"
            "  -t <num>   Timeout ms, default: 1000\n"
            "  -j         Enable JSON output server/result messages\n"
            "  --no-render Disable display/render output for debugging\n"
            "  -v         Verbose perf logging\n"
            "\n"
            "TCP and Vision modes expect raw RGB888 frames of size width*height*3 bytes.\n"
            "Type 'x' then Enter, or press Ctrl+C to stop the app cleanly.\n",
            prog, DEFAULT_CAMERA_DEVICE, DEFAULT_DISPLAY_DEVICE,
            DEFAULT_TCP_PORT, DEFAULT_VISION_TARGET_IP,
            DEFAULT_VISION_STREAM_PORT, DEFAULT_VISION_MESSAGE_PORT,
            DEFAULT_CAMERA_WIDTH, DEFAULT_CAMERA_HEIGHT,
            DEFAULT_DISPLAY_WIDTH, DEFAULT_DISPLAY_HEIGHT);
}

void app_config_set_defaults(app_context_t *app)
{
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
    (void)snprintf(app->vision.target_ip, sizeof(app->vision.target_ip), "%s",
                   DEFAULT_VISION_TARGET_IP);

    app->camera_width = DEFAULT_CAMERA_WIDTH;
    app->camera_height = DEFAULT_CAMERA_HEIGHT;
    app->display_width = DEFAULT_DISPLAY_WIDTH;
    app->display_height = DEFAULT_DISPLAY_HEIGHT;
    app->timeout_ms = 1000;
    app->input_mode = APP_INPUT_CAMERA;
    app->json_enabled = 0;
    app->render_enabled = 1;
    app->tcp_input.server_fd = -1;
    app->tcp_input.client_fd = -1;
    app->tcp_input.port = DEFAULT_TCP_PORT;
    app->json_output.server_fd = -1;
    app->json_output.client_fd = -1;
    app->json_output.port = DEFAULT_JSON_PORT;
    app->vision.stream_port = DEFAULT_VISION_STREAM_PORT;
    app->vision.message_port = DEFAULT_VISION_MESSAGE_PORT;

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
        {"no-render", no_argument, NULL, 1000},
        {"vision-target", required_argument, NULL, 1001},
        {"stream-port", required_argument, NULL, 1002},
        {"message-port", required_argument, NULL, 1003},
        {0, 0, 0, 0},
    };

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
                    /* Keep the legacy command line while using the bridge's
                     * default Vision Protocol transport internally. */
                    app->input_mode = APP_INPUT_VISION;
                    printf("[input] -i tcp compatibility mode: using Vision Protocol\n");
                } else if (strcmp(optarg, "raw-tcp") == 0) {
                    app->input_mode = APP_INPUT_TCP;
                } else if (strcmp(optarg, "vision") == 0) {
                    app->input_mode = APP_INPUT_VISION;
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
            case 1000:
                app->render_enabled = 0;
                break;
            case 1001:
                (void)snprintf(app->vision.target_ip, sizeof(app->vision.target_ip), "%s", optarg);
                break;
            case 1002:
                app->vision.stream_port = atoi(optarg);
                break;
            case 1003:
                app->vision.message_port = atoi(optarg);
                break;
            default:
                return -1;
        }
    }

    if (app->models[0].path[0] == '\0' || app->models[1].path[0] == '\0') {
        return -1;
    }

    app->models[0].timeout_ms = app->timeout_ms;
    app->models[1].timeout_ms = app->timeout_ms;
    app->models[0].verbose = app->verbose;
    app->models[1].verbose = app->verbose;

    if (app->input_mode == APP_INPUT_TCP || app->input_mode == APP_INPUT_VISION) {
        app->json_enabled = 1;
    }

    if (app->input_mode == APP_INPUT_VISION) {
        printf("[input] mode=%s source=%ux%u target=%s stream=%d message=%d json=%s render=%s\n",
               input_mode_to_string(app->input_mode),
               app->camera_width, app->camera_height,
               app->vision.target_ip, app->vision.stream_port, app->vision.message_port,
               app->json_enabled ? "on" : "off",
               app->render_enabled ? "on" : "off");
    } else {
        printf("[input] mode=%s source=%ux%u port=%d json=%s render=%s\n",
               input_mode_to_string(app->input_mode),
               app->camera_width, app->camera_height,
               app->tcp_input.port,
               app->json_enabled ? "on" : "off",
               app->render_enabled ? "on" : "off");
    }

    return 0;
}
