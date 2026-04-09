#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>

#include "app_memory.h"
#include "npu_api.h"
#include "platform_api.h"

#define APP_MAX_MODELS 2
#define APP_DISPLAY_BUFFER_COUNT 2

#define DEFAULT_CAMERA_DEVICE "/dev/video2"
#define DEFAULT_DISPLAY_DEVICE "/dev/overlay"
#define DEFAULT_SCALER0_DEVICE "/dev/scaler1"
#define DEFAULT_SCALER1_DEVICE "/dev/scaler3"
#define DEFAULT_CAMERA_WIDTH 1280
#define DEFAULT_CAMERA_HEIGHT 720
#define DEFAULT_DISPLAY_WIDTH 800
#define DEFAULT_DISPLAY_HEIGHT 480
#define DEFAULT_TCP_PORT 9999
#define DEFAULT_JSON_PORT 9998
#define CPU_CORE_NUM 4
#define CPU_CORE_NUM_MAX 5
#define CPU_STAT_MAX 4
#define NPU_CORE_CLOCK 1000000000ULL
#define NPU_CORE_NUM 2ULL
#define NPU_ALPHA (32ULL * 32ULL)
#define APP_MAX_TRACKS 256

typedef struct {
    char so_path[1024];
    char cmd_path[1024];
    char param_path[1024];
} network_files_t;

#define MAX_LANES 8
#define MAX_POINTS 80

typedef struct {
    float x[MAX_POINTS];
    float y[MAX_POINTS];
    float conf[MAX_POINTS];
    int n;
    float lane_score;
} lane_polyline_t;

typedef struct {
    uint64_t ts_ms;
    int img_w;
    int img_h;
    int num_lanes;
    lane_polyline_t lane[MAX_LANES];
} laneaf_result_t;

typedef struct {
    int track_id;
    int cls;
    float score;
    float x_min;
    float y_min;
    float x_max;
    float y_max;
} tracked_object_t;

typedef struct {
    int count;
    tracked_object_t objects[APP_MAX_TRACKS];
} tracked_objects_t;

typedef struct {
    int active;
    int track_id;
    int cls;
    int age;
    int hits;
    int time_since_update;
    int hit_streak;
    float score;
    float cx;
    float cy;
    float area;
    float ratio;
    float v_cx;
    float v_cy;
    float v_area;
    float covariance[7][7];
} sort_track_t;

typedef struct {
    int next_track_id;
    int frame_count;
    int max_age;
    int min_hits;
    float iou_threshold;
    sort_track_t tracks[APP_MAX_TRACKS];
} sort_tracker_t;

typedef enum {
    APP_INPUT_CAMERA = 0,
    APP_INPUT_TCP,
} app_input_mode_t;

typedef struct {
    int server_fd;
    int client_fd;
    int port;
    uint8_t *frame_buffer;
    size_t frame_bytes;
} tcp_input_context_t;

typedef struct {
    int server_fd;
    int client_fd;
    int port;
} json_output_context_t;

typedef struct {
    int index;
    int cluster;
    char path[1024];
    int timeout_ms;
    int verbose;
    npu_t *npu;
    npu_net_t *net;
    npu_buf_t *input_buf;
    npu_buf_t *output_buf;
    int input_width;
    int input_height;
    int input_size;
    int output_size;
    int post_type;
    npu_perf_t perf;
    npu_err_bits_t err_status;
    double npuUtilization;
    enlight_objs_t det_result;
    tracked_objects_t tracked_result;
    enlight_batch_cls_t cls_result;
    laneaf_result_t *lane_data;
} model_context_t;

typedef struct {
    uint32_t cpuUtil[CPU_CORE_NUM_MAX];
    uint32_t memUsage;
    double fps;
    int thread_started;
    pthread_t thread;
} system_perf_t;

typedef struct {
    app_input_mode_t input_mode;
    char camera_device[256];
    char display_device[256];
    char scaler_device[SCALER_INDEX_MAX][256];
    uint32_t camera_width;
    uint32_t camera_height;
    uint32_t display_width;
    uint32_t display_height;
    uint32_t display_x;
    uint32_t display_y;
    int timeout_ms;
    int verbose;
    int json_enabled;
    volatile sig_atomic_t stop;

    camera_handle_t camera;
    display_handle_t display;
    scaler_handle_t scaler;
    uint8_t *camera_virt_addr;
    uint64_t camera_phys_addr;
    npu_buf_t *tcp_stage_buf;
    int display_buffer_index;
    uint64_t frame_index;
    tcp_input_context_t tcp_input;
    json_output_context_t json_output;

    model_context_t models[APP_MAX_MODELS];
    sort_tracker_t trackers[APP_MAX_MODELS];
    app_memory_context_t memory;
    system_perf_t perf;
} app_context_t;

typedef struct {
    model_context_t *model;
    int run_status;
} inference_task_t;

const char *input_mode_to_string(app_input_mode_t mode);

#endif
