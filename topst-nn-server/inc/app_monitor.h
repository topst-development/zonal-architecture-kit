#ifndef APP_MONITOR_H
#define APP_MONITOR_H

#include "app_types.h"

int app_monitor_start(app_context_t *app);
void app_monitor_stop(app_context_t *app);
void app_monitor_update_fps(app_context_t *app);

#endif
