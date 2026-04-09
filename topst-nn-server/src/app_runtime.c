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

    for (i = 0; i < APP_MAX_MODELS; ++i) {
        app_tracker_init(&app->trackers[i]);
    }

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

    if (init_model(&app->models[0]) != 0) {
        fprintf(stderr, "init_model failed for model0: %s\n", app->models[0].path);
        return -1;
    }
    if (init_model(&app->models[1]) != 0) {
        fprintf(stderr, "init_model failed for model1: %s\n", app->models[1].path);
        return -1;
    }

    if (app_input_init(app) != 0) {
        if (app->input_mode == APP_INPUT_TCP) {
            fprintf(stderr, "tcp input init failed on port %d\n", app->tcp_input.port);
        } else {
            fprintf(stderr, "app_input_init failed\n");
        }
        return -1;
    }

    if (app_memory_init(&app->memory, app->camera_device,
                        app->display_width, app->display_height) != 0) {
        fprintf(stderr, "app_memory_init failed\n");
        return -1;
    }

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

    app_input_deinit(app);
    app_json_deinit(app);
    app_monitor_stop(app);

    for (i = 0; i < APP_MAX_MODELS; ++i) {
        cleanup_model(&app->models[i]);
        app_tracker_reset(&app->trackers[i]);
    }

    (void)app_memory_deinit(&app->memory, app->display_width, app->display_height);

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
