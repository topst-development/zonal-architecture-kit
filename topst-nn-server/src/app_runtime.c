#include "app_runtime.h"

#include <pthread.h>
#include <stdio.h>

#include "app_inference.h"
#include "app_input.h"
#include "app_json.h"
#include "app_monitor.h"
#include "app_tracker.h"

int app_runtime_init(app_context_t *app)
{
    int i;

    /* 런타임 시작 전에 모델별 tracker 상태를 초기화한다. */
    for (i = 0; i < APP_MAX_MODELS; ++i) {
        app_tracker_init(&app->trackers[i]);
    }

    /* 출력 장치와 입력/출력 리사이즈에 필요한 scaler를 먼저 연다. */
    if (display_create(&app->display) != 0) {
        fprintf(stderr, "display_create failed\n");
        return -1;
    }

    if (display_open_device(app->display, app->display_device) != 0) {
        fprintf(stderr, "display_open_device failed: %s\n", app->display_device);
        return -1;
    }

    if (scaler_create(&app->scaler) != 0) {
        fprintf(stderr, "scaler_create failed\n");
        return -1;
    }

    if (scaler_open_device(app->scaler, app->scaler_device[SCALER_INDEX_0],
                           SCALER_INDEX_0) != 0) {
        fprintf(stderr, "scaler_open_device failed: %s\n",
                app->scaler_device[SCALER_INDEX_0]);
        return -1;
    }

    if (scaler_open_device(app->scaler, app->scaler_device[SCALER_INDEX_1],
                           SCALER_INDEX_1) != 0) {
        fprintf(stderr, "scaler_open_device failed: %s\n",
                app->scaler_device[SCALER_INDEX_1]);
        return -1;
    }

    /* 카메라 모드일 때만 카메라 장치와 캡처 버퍼를 준비한다. */
    if (app->input_mode == APP_INPUT_CAMERA) {
        if (camera_create(&app->camera) != 0) {
            fprintf(stderr, "camera_create failed\n");
            return -1;
        }
        if (camera_open_device(app->camera, app->camera_device) != 0) {
            fprintf(stderr, "camera_open_device failed: %s\n", app->camera_device);
            return -1;
        }
        if (camera_set_config(app->camera, app->camera_width, app->camera_height) != 0) {
            fprintf(stderr, "camera_set_config failed: %s %ux%u\n",
                    app->camera_device, app->camera_width, app->camera_height);
            return -1;
        }
    }

    /* 두 모델의 NPU 네트워크와 입출력 버퍼를 각각 준비한다. */
    if (init_model(&app->models[0]) != 0) {
        fprintf(stderr, "init_model failed for model0: %s\n", app->models[0].path);
        return -1;
    }
    if (init_model(&app->models[1]) != 0) {
        fprintf(stderr, "init_model failed for model1: %s\n", app->models[1].path);
        return -1;
    }

    /* 입력 경로(TCP 수신 버퍼 등)를 모드에 따라 초기화한다. */
    if (app_input_init(app) != 0) {
        if (app->input_mode == APP_INPUT_TCP) {
            fprintf(stderr, "tcp input init failed on port %d\n", app->tcp_input.port);
        } else {
            fprintf(stderr, "app_input_init failed\n");
        }
        return -1;
    }

    /* 디스플레이용 출력 버퍼를 PMAP 메모리로 확보한다. */
    if (app_memory_init(&app->memory, app->camera_device,
                        app->display_width, app->display_height) != 0) {
        fprintf(stderr, "app_memory_init failed\n");
        return -1;
    }

    /* JSON 송신이 켜져 있으면 결과 스트리밍용 소켓 서버를 연다. */
    if (app->json_enabled) {
        if (app_json_init(app) != 0) {
            fprintf(stderr, "app_json_init failed on port %d\n", app->json_output.port);
            return -1;
        }
    }

    if (app_monitor_start(app) != 0) {
        fprintf(stderr, "monitor thread start failed\n");
    }

    return 0;
}

void app_runtime_cleanup(app_context_t *app)
{
    int i;

    if (app == NULL) {
        return;
    }

    /* 런타임 종료 시에는 입력/JSON/모니터 스레드부터 먼저 정리한다. */
    app_input_deinit(app);
    app_json_deinit(app);
    app_monitor_stop(app);

    /* 모델별 NPU 자원과 tracker 상태를 정리한다. */
    for (i = 0; i < APP_MAX_MODELS; ++i) {
        cleanup_model(&app->models[i]);
        app_tracker_reset(&app->trackers[i]);
    }

    (void)app_memory_deinit(&app->memory, app->display_width, app->display_height);

    /* 하드웨어 장치는 생성의 반대 순서로 닫아 자원 해제 흐름을 단순하게 유지한다. */
    if (app->scaler != NULL) {
        scaler_close_device(app->scaler, SCALER_INDEX_0);
        scaler_close_device(app->scaler, SCALER_INDEX_1);
        scaler_destroy(app->scaler);
        app->scaler = NULL;
    }
    if (app->display != NULL) {
        display_close_device(app->display);
        display_destroy(app->display);
        app->display = NULL;
    }
    if (app->camera != NULL) {
        camera_close_device(app->camera);
        camera_destroy(app->camera);
        app->camera = NULL;
    }
    if (app->tcp_stage_buf != NULL) {
        buffer_close(app->tcp_stage_buf);
        app->tcp_stage_buf = NULL;
    }
}
