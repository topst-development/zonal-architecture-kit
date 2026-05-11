#define _DEFAULT_SOURCE

#include "app_inference.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int build_network_paths(const char *dir, network_files_t *files)
{
    int written;

    /* 모델 디렉터리 아래의 고정 파일 이름을 실제 로드 경로로 조합 */
    if (dir == NULL || files == NULL) {
        return -1;
    }

    written = snprintf(files->so_path, sizeof(files->so_path), "%s/net.so", dir);
    if (written < 0 || (size_t)written >= sizeof(files->so_path)) {
        return -1;
    }
    written = snprintf(files->cmd_path, sizeof(files->cmd_path), "%s/npu_cmd.bin", dir);
    if (written < 0 || (size_t)written >= sizeof(files->cmd_path)) {
        return -1;
    }
    written = snprintf(files->param_path, sizeof(files->param_path),
                       "%s/quantized_network.bin", dir);
    if (written < 0 || (size_t)written >= sizeof(files->param_path)) {
        return -1;
    }

    if (access(files->so_path, R_OK) != 0 || access(files->cmd_path, R_OK) != 0 ||
        access(files->param_path, R_OK) != 0) {
        return -1;
    }

    return 0;
}

uint32_t app_align_width(uint32_t width, uint32_t multiple)
{
    /* NPU 입력 버퍼는 폭 정렬 제약있으므로 상수 배수  */
    if ((width % multiple) == 0u) {
        return width;
    }
    return ((width / multiple) + 1u) * multiple;
}

void cleanup_model(model_context_t *model)
{
    /* 모델별 NPU 자원은 입력 버퍼 -> 출력 버퍼 -> 네트워크 -> NPU 순으로 정리 */
    if (model->input_buf != NULL) {
        buffer_close(model->input_buf);
        model->input_buf = NULL;
    }
    if (model->output_buf != NULL) {
        buffer_close(model->output_buf);
        model->output_buf = NULL;
    }
    if (model->net != NULL) {
        network_close(model->net);
        model->net = NULL;
    }
    if (model->npu != NULL) {
        npu_close(model->npu);
        model->npu = NULL;
    }
}

int init_model(model_context_t *model)
{
    network_files_t files;
    uint32_t aligned_width;

    /* net.so / npu_cmd.bin / quantized_network.bin 3종 파일을 먼저 검사 */
    if (build_network_paths(model->path, &files) != 0) {
        fprintf(stderr, "invalid model directory: %s\n", model->path);
        return -1;
    }

    /* 각 모델은 지정된 cluster 번호의 NPU를 열고 그 위에 네트워크를 적재 */
    model->npu = npu_open(model->cluster);
    if (model->npu == NULL) {
        fprintf(stderr, "failed to open NPU cluster %d\n", model->cluster);
        return -1;
    }

    model->net = network_load_from_file(model->npu, files.so_path,
                                        files.cmd_path, files.param_path);
    if (model->net == NULL) {
        fprintf(stderr, "failed to load network on cluster %d\n", model->cluster);
        return -1;
    }

    model->input_width = network_get_input_width(model->net);
    model->input_height = network_get_input_height(model->net);
    model->output_size = network_get_output_size(model->net);
    model->post_type = network_get_type(model->net);

    /* RGB 입력 기준으로 정렬된 폭을 사용해 실제 입력 버퍼 크기를 계산 */
    aligned_width = app_align_width((uint32_t)model->input_width, 16u);
    model->input_size = (int)(aligned_width * (uint32_t)model->input_height * 3u);

    /* 입력/출력 버퍼는 모두 해당 NPU가 직접 접근할 수 있는 버퍼로 할당 */
    model->input_buf = buffer_alloc(model->npu, model->input_size);
    model->output_buf = buffer_alloc(model->npu, model->output_size);
    if (model->input_buf == NULL || model->output_buf == NULL) {
        fprintf(stderr, "failed to allocate buffers for cluster %d\n", model->cluster);
        return -1;
    }

    memset(&model->perf, 0, sizeof(model->perf));
    memset(&model->err_status, 0, sizeof(model->err_status));
    model->npuUtilization = 0.0;

    printf("[model%d] cluster=%d name=%s input=%dx%d type=%d\n",
           model->index, model->cluster,
           model->net->methods->get_network_name(),
           model->input_width, model->input_height, model->post_type);

    return 0;
}

void *run_inference_thread(void *arg)
{
    inference_task_t *task = (inference_task_t *)arg;
    unsigned long long freq;

    /* 매 프레임 추론 전에 이전 perf/error 상태를 지우고 새 측정을 시작 */
    memset(&task->model->perf, 0, sizeof(task->model->perf));
    memset(&task->model->err_status, 0, sizeof(task->model->err_status));

    /* 준비된 input/output 버퍼를 사용해 동기 방식으로 NPU 추론을 실행 */
    task->run_status = network_run(task->model->net,
                                   task->model->input_buf,
                                   task->model->output_buf,
                                   &task->model->err_status,
                                   NPU_RUN_SYNC,
                                   &task->model->perf,
                                   task->model->timeout_ms);

    if (task->run_status == 0 && task->model->verbose) {
        printf("[model%d] elapsed=%.2f ms\n",
               task->model->index,
               task->model->perf.elapsed_in_us / 1000.0);
    }

    /* conv MAC 수와 실행 시간을 이용해 대략적인 NPU 사용률을 계산 */
    if (task->run_status == 0 && task->model->perf.elapsed_in_us > 0) {
        freq = (unsigned long long)task->model->perf.elapsed_in_us * (NPU_CORE_CLOCK / 1000000ULL);
        freq = freq * NPU_ALPHA * NPU_CORE_NUM;
        if (freq > 0ULL) {
            task->model->npuUtilization =
                100.0 * ((double)task->model->net->methods->conv_mac_num / (double)freq);
        } else {
            task->model->npuUtilization = 0.0;
        }
    } else {
        task->model->npuUtilization = 0.0;
    }

    return NULL;
}

int postprocess_model(model_context_t *model)
{
    /* 네트워크 타입에 맞춰 후처리 호출 */
    if (model->post_type == TELECHIPS_NPU_POST_DETECTOR) {
        memset(&model->det_result, 0, sizeof(model->det_result));
        return network_run_postprocess(model->net, model->output_buf,
                                       &model->det_result);
    }
    if (model->post_type == TELECHIPS_NPU_POST_CLASSIFIER) {
        memset(&model->cls_result, 0, sizeof(model->cls_result));
        return network_run_postprocess(model->net, model->output_buf,
                                       &model->cls_result);
    }
    if (model->post_type == TELECHIPS_NPU_POST_CUSTOM) {
        model->lane_data = NULL;
        return network_run_postprocess(model->net, model->output_buf,
                                       &model->lane_data);
    }

    return 0;
}
