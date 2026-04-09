#ifndef APP_CONTROL_H
#define APP_CONTROL_H

#include "app_types.h"

void app_control_install_signal_handlers(app_context_t *app);
void app_control_uninstall_signal_handlers(void);
int app_control_start_keyboard(app_context_t *app);
void app_control_stop_keyboard(void);

#endif
