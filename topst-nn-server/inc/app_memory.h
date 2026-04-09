#ifndef APP_MEMORY_H
#define APP_MEMORY_H

#include <stdint.h>

#define APP_PMAP_DEVICE_PATH "/proc/reserved_mem"
#define APP_PMAP_NAME_0 "pmap_visionprotocol"
#define APP_PMAP_NAME_1 "pmap_visionprotocol1"
#define APP_PMAP_NAME_2 "pmap_visionprotocol2"
#define APP_PMAP_NAME_3 "pmap_visionprotocol3"
#define APP_PMAP_MAX_NUMBER 4
#define APP_PMAP_SPLIT_NUMBER 4
#define APP_PMAP_SIZE (1920 * 1080 * 4)

typedef struct {
    int32_t display_memory_fd;
    uint64_t phy_base_output[2];
    uint8_t *map_base_output[2];
    uint64_t reserved_memory[APP_PMAP_MAX_NUMBER][APP_PMAP_SPLIT_NUMBER];
} app_memory_context_t;

int app_memory_init(app_memory_context_t *ctx, const char *input_path,
                    uint32_t output_width, uint32_t output_height);
int app_memory_deinit(app_memory_context_t *ctx, uint32_t output_width,
                      uint32_t output_height);

#endif
