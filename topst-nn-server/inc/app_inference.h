#ifndef APP_INFERENCE_H
#define APP_INFERENCE_H

#include "app_types.h"

uint32_t app_align_width(uint32_t width, uint32_t multiple);
void cleanup_model(model_context_t *model);
int init_model(model_context_t *model);
void *run_inference_thread(void *arg);
int postprocess_model(model_context_t *model);

#endif
