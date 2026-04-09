#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "app_types.h"

void app_config_set_defaults(app_context_t *app);
int app_config_parse_args(app_context_t *app, int argc, char **argv);
void app_config_print_usage(const char *prog);

#endif
