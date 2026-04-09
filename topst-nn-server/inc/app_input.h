#ifndef APP_INPUT_H
#define APP_INPUT_H

#include "app_types.h"

int app_input_init(app_context_t *app);
void app_input_deinit(app_context_t *app);
int acquire_input_frame(app_context_t *app);
void release_input_frame(app_context_t *app);
int prepare_model_input(app_context_t *app, const model_context_t *model,
                        scaler_index_t scaler_index);

#endif
