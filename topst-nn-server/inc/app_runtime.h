#ifndef APP_RUNTIME_H
#define APP_RUNTIME_H

#include "app_types.h"

int app_runtime_init(app_context_t *app);
void app_runtime_cleanup(app_context_t *app);

#endif
