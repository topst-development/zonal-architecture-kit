#ifndef APP_JSON_H
#define APP_JSON_H

#include "app_types.h"

int app_json_init(app_context_t *app);
void app_json_deinit(app_context_t *app);
void app_json_poll_accept(app_context_t *app);
int app_json_send_results(app_context_t *app);

#endif
