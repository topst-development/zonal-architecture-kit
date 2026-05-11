#include "app_pipeline.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "app_inference.h"
#include "app_input.h"
#include "app_json.h"
#include "app_monitor.h"
#include "app_render.h"
#include "app_tracker.h"

int app_pipeline_run(app_context_t *app)
{
    while (!app->stop) {
        inference_task_t tasks[APP_MAX_MODELS];
        pthread_t threads[APP_MAX_MODELS];
        int i;

        /* 프레임 1장을 입력받은 뒤, 그 프레임에 대해 전체 인지 파이프라인을 수행한다. */
        if (acquire_input_frame(app) <= 0) {
            app_json_poll_accept(app);
            continue;
        }
        app_json_poll_accept(app);

        /* 공용 입력 프레임을 각 모델의 NPU 입력 버퍼 크기에 맞게 리사이즈한다. */
        if (prepare_model_input(app, &app->models[0], SCALER_INDEX_0) != 0 ||
            prepare_model_input(app, &app->models[1], SCALER_INDEX_1) != 0) {
            fprintf(stderr, "failed to prepare input frame\n");
            release_input_frame(app);
            return -1;
        }

        /* 추론이 가장 무거운 단계이므로 두 모델은 병렬 스레드로 실행한다. */
        for (i = 0; i < APP_MAX_MODELS; ++i) {
            memset(&tasks[i], 0, sizeof(tasks[i]));
            tasks[i].model = &app->models[i];
            if (pthread_create(&threads[i], NULL, run_inference_thread, &tasks[i]) != 0) {
                fprintf(stderr, "failed to create inference thread for model%d\n", i);
                app->stop = 1;
                release_input_frame(app);
                return -1;
            }
        }

        for (i = 0; i < APP_MAX_MODELS; ++i) {
            (void)pthread_join(threads[i], NULL);
            if (tasks[i].run_status != 0) {
                fprintf(stderr, "inference failed for model%d\n", i);
                app->stop = 1;
            }
        }

        /* 후처리는 raw NPU 출력을 detector/lane/classifier 결과 구조체로 변환한다. */
        for (i = 0; i < APP_MAX_MODELS; ++i) {
            if (postprocess_model(&app->models[i]) != 0) {
                fprintf(stderr, "postprocess failed for model%d\n", i);
                app->stop = 1;
            }
        }
        if (app->stop) {
            release_input_frame(app);
            return -1;
        }

        /* 트래커는 detector 결과를 받아 안정적인 track ID를 생성해 렌더링/JSON에 넘긴다. */
        app_tracker_update(app);
        app_monitor_update_fps(app);
        app->frame_index++;
        (void)app_json_send_results(app);

        if (render_output_frame(app) != 0) {
            fprintf(stderr, "failed to display frame\n");
            release_input_frame(app);
            return -1;
        }

        release_input_frame(app);
    }

    return 0;
}
