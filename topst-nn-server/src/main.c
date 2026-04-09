#include <stdio.h>

#include "app_config.h"
#include "app_control.h"
#include "app_pipeline.h"
#include "app_runtime.h"
#include "app_types.h"

int main(int argc, char **argv)
{
    static app_context_t app;
    int status = 0;

    app_config_set_defaults(&app);
    if (app_config_parse_args(&app, argc, argv) != 0) {
        app_config_print_usage(argv[0]);
        return 1;
    }

    app_control_install_signal_handlers(&app);
    (void)app_control_start_keyboard(&app);

    if (app_runtime_init(&app) != 0) {
        fprintf(stderr, "runtime initialization failed\n");
        status = 1;
        goto done;
    }

    if (app_pipeline_run(&app) != 0 && !app.stop) {
        status = 1;
    }

done:
    app_runtime_cleanup(&app);
    app_control_stop_keyboard();
    app_control_uninstall_signal_handlers();
    return status;
}
