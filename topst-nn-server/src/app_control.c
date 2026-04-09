#define _DEFAULT_SOURCE

#include "app_control.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static app_context_t *g_app_ctx;
static struct termios g_saved_termios;
static int g_termios_valid;
static int g_keyboard_enabled;
static pthread_t g_keyboard_thread;

static void app_control_on_signal(int sig)
{
    (void)sig;
    if (g_app_ctx != NULL) {
        g_app_ctx->stop = 1;
    }
}

static void *app_control_keyboard_thread(void *arg)
{
    app_context_t *app = (app_context_t *)arg;

    while (!app->stop) {
        char ch;
        ssize_t ret = read(STDIN_FILENO, &ch, 1);

        if (ret <= 0) {
            continue;
        }
        if (ch == 'x' || ch == 'X') {
            app->stop = 1;
            raise(SIGINT);
            break;
        }
    }

    return NULL;
}

void app_control_install_signal_handlers(app_context_t *app)
{
    g_app_ctx = app;
    signal(SIGINT, app_control_on_signal);
    signal(SIGTERM, app_control_on_signal);
}

void app_control_uninstall_signal_handlers(void)
{
    g_app_ctx = NULL;
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
}

int app_control_start_keyboard(app_context_t *app)
{
    struct termios raw_termios;

    if (!isatty(STDIN_FILENO)) {
        return 0;
    }
    if (tcgetattr(STDIN_FILENO, &g_saved_termios) != 0) {
        return -1;
    }

    g_termios_valid = 1;
    raw_termios = g_saved_termios;
    raw_termios.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw_termios.c_cc[VMIN] = 0;
    raw_termios.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw_termios) != 0) {
        g_termios_valid = 0;
        return -1;
    }

    if (pthread_create(&g_keyboard_thread, NULL, app_control_keyboard_thread, app) != 0) {
        if (g_termios_valid) {
            (void)tcsetattr(STDIN_FILENO, TCSANOW, &g_saved_termios);
            g_termios_valid = 0;
        }
        return -1;
    }

    g_keyboard_enabled = 1;
    return 0;
}

void app_control_stop_keyboard(void)
{
    if (g_keyboard_enabled && g_app_ctx != NULL) {
        g_app_ctx->stop = 1;
        (void)pthread_join(g_keyboard_thread, NULL);
        g_keyboard_enabled = 0;
    }

    if (g_termios_valid) {
        (void)tcsetattr(STDIN_FILENO, TCSANOW, &g_saved_termios);
        g_termios_valid = 0;
    }
}
