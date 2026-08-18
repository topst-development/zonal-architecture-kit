#ifndef APP_VISION_H
#define APP_VISION_H

#include <stddef.h>
#include <stdint.h>

#include "app_types.h"

int app_vision_init(app_context_t *app);
void app_vision_deinit(app_context_t *app);
int app_vision_recv_frame(app_context_t *app);
void app_vision_release_frame(app_context_t *app);
const uint8_t *app_vision_frame_data(const app_context_t *app);
uint64_t app_vision_frame_phys(const app_context_t *app);
int app_vision_send_result_json(app_context_t *app, const char *json, size_t length);

#endif

