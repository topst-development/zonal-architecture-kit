#define _DEFAULT_SOURCE

#include "app_control.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

static app_context_t *g_app_ctx;
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
        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 200 * 1000;

        if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) <= 0) {
            continue;
        }
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char line[32];
            ssize_t ret = read(STDIN_FILENO, line, sizeof(line));

            if (ret <= 0) {
                continue;
            }
            if (line[0] == 'x' || line[0] == 'X') {
                app->stop = 1;
                break;
            }
        }
    }

    return NULL;
}

void app_control_install_signal_handlers(app_context_t *app)
{
    struct sigaction action;

    g_app_ctx = app;
    memset(&action, 0, sizeof(action));
    action.sa_handler = app_control_on_signal;
    sigemptyset(&action.sa_mask);
    (void)sigaction(SIGINT, &action, NULL);
    (void)sigaction(SIGTERM, &action, NULL);
    (void)sigaction(SIGTSTP, &action, NULL);
    (void)sigaction(SIGQUIT, &action, NULL);
    (void)sigaction(SIGHUP, &action, NULL);
}

void app_control_uninstall_signal_handlers(void)
{
    g_app_ctx = NULL;
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
}

int app_control_start_keyboard(app_context_t *app)
{
    if (!isatty(STDIN_FILENO)) {
        return 0;
    }

    if (pthread_create(&g_keyboard_thread, NULL, app_control_keyboard_thread, app) != 0) {
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
}
