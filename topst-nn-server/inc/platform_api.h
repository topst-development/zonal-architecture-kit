#ifndef PLATFORM_API_H
#define PLATFORM_API_H

#include <stdint.h>

typedef enum {
    CAMERA_STATUS_IDLE = 0,
    CAMERA_STATUS_OPENED,
    CAMERA_STATUS_STREAMING,
} camera_status_t;

typedef enum {
    SCALER_INDEX_0 = 0,
    SCALER_INDEX_1,
    SCALER_INDEX_MAX,
} scaler_index_t;

typedef enum {
    SCALER_FORMAT_ARGB8888 = 12,
    SCALER_FORMAT_RGB888 = 14,
} scaler_format_t;

typedef struct {
    uint64_t paddr;
    uint32_t width;
    uint32_t height;
    scaler_format_t format;
} scaler_image_t;

typedef struct camera_context *camera_handle_t;
typedef struct display_context *display_handle_t;
typedef struct scaler_context *scaler_handle_t;

int camera_create(camera_handle_t *handle);
int camera_destroy(camera_handle_t handle);
int camera_open_device(camera_handle_t handle, const char *device_path);
int camera_close_device(camera_handle_t handle);
int camera_set_config(camera_handle_t handle, uint32_t width, uint32_t height);
int camera_get_buffer(camera_handle_t handle, uint8_t **virtual_addr,
                      uint64_t *physical_addr);
int camera_release_buffer(camera_handle_t handle);

int display_create(display_handle_t *handle);
int display_destroy(display_handle_t handle);
int display_open_device(display_handle_t handle, const char *device_path);
int display_close_device(display_handle_t handle);
int display_show(display_handle_t handle, uint64_t base_addr, uint32_t x,
                 uint32_t y, uint32_t width, uint32_t height);

int scaler_create(scaler_handle_t *handle);
int scaler_destroy(scaler_handle_t handle);
int scaler_open_device(scaler_handle_t handle, const char *device_path,
                       scaler_index_t scaler_index);
int scaler_close_device(scaler_handle_t handle, scaler_index_t scaler_index);
int scaler_resize(scaler_handle_t handle, scaler_index_t scaler_index,
                  scaler_image_t src, scaler_image_t dst);
int scaler_poll(scaler_handle_t handle, scaler_index_t scaler_index);

#endif
