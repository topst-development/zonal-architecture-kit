#ifndef APP_TRACKER_H
#define APP_TRACKER_H

#include "app_types.h"

void app_tracker_init(sort_tracker_t *tracker);
void app_tracker_reset(sort_tracker_t *tracker);
void app_tracker_update(app_context_t *app);

#endif
