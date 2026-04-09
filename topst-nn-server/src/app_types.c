#include "app_types.h"

const char *input_mode_to_string(app_input_mode_t mode)
{
    switch (mode) {
        case APP_INPUT_CAMERA:
            return "camera";
        case APP_INPUT_TCP:
            return "tcp";
        default:
            return "unknown";
    }
}
