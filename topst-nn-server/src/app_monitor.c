#define _DEFAULT_SOURCE

#include "app_monitor.h"

#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static void app_monitor_get_cpu_usage(system_perf_t *perf)
{
    static int32_t prev_jiffies[CPU_CORE_NUM + 1][CPU_STAT_MAX];
    int32_t cur_jiffies[CPU_CORE_NUM + 1][CPU_STAT_MAX] = {{0}};
    int32_t diff_jiffies[CPU_CORE_NUM + 1][CPU_STAT_MAX];
    int32_t total_jiffies[CPU_CORE_NUM + 1] = {0};
    int32_t tmp;
    int core_idx;
    int stat_idx;
    char cpu_id[16];
    FILE *fp = fopen("/proc/stat", "r");

    if (fp == NULL) {
        return;
    }

    for (core_idx = 0; core_idx < CPU_CORE_NUM + 1; ++core_idx) {
        if (fscanf(fp, "%15s %d %d %d %d %d %d %d %d %d %d",
                   cpu_id,
                   &cur_jiffies[core_idx][0],
                   &cur_jiffies[core_idx][1],
                   &cur_jiffies[core_idx][2],
                   &cur_jiffies[core_idx][3],
                   &tmp, &tmp, &tmp, &tmp, &tmp, &tmp) != 11) {
            break;
        }

        for (stat_idx = 0; stat_idx < CPU_STAT_MAX; ++stat_idx) {
            diff_jiffies[core_idx][stat_idx] =
                cur_jiffies[core_idx][stat_idx] - prev_jiffies[core_idx][stat_idx];
            total_jiffies[core_idx] += diff_jiffies[core_idx][stat_idx];
        }

        if (total_jiffies[core_idx] > 0) {
            perf->cpuUtil[core_idx] = (uint32_t)llround(
                100.0 * (1.0 - (diff_jiffies[core_idx][3] / (double)total_jiffies[core_idx])));
        }

        memcpy(prev_jiffies[core_idx], cur_jiffies[core_idx], sizeof(int32_t) * CPU_STAT_MAX);
    }

    fclose(fp);
}

static void app_monitor_get_memory_usage(system_perf_t *perf)
{
    char line[256];
    int mem_total = 0;
    int mem_free = 0;
    FILE *fp = fopen("/proc/meminfo", "r");

    if (fp == NULL) {
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            mem_total = atoi(line + 9);
        } else if (strncmp(line, "MemFree:", 8) == 0) {
            mem_free = atoi(line + 8);
            break;
        }
    }
    fclose(fp);

    if (mem_total > 0) {
        perf->memUsage = (uint32_t)llround(100.0 * (1.0 - (mem_free / (double)mem_total)));
    }
}

static void *app_monitor_thread(void *arg)
{
    app_context_t *app = (app_context_t *)arg;

    while (!app->stop) {
        app_monitor_get_cpu_usage(&app->perf);
        app_monitor_get_memory_usage(&app->perf);
        usleep(500 * 1000);
    }

    return NULL;
}

int app_monitor_start(app_context_t *app)
{
    app_monitor_get_cpu_usage(&app->perf);
    app_monitor_get_memory_usage(&app->perf);
    if (pthread_create(&app->perf.thread, NULL, app_monitor_thread, app) != 0) {
        return -1;
    }
    app->perf.thread_started = 1;
    return 0;
}

void app_monitor_stop(app_context_t *app)
{
    if (!app->perf.thread_started) {
        return;
    }

    app->stop = 1;
    (void)pthread_join(app->perf.thread, NULL);
    app->perf.thread_started = 0;
}

void app_monitor_update_fps(app_context_t *app)
{
    static struct timeval begin;
    struct timeval end;

    gettimeofday(&end, NULL);
    if (begin.tv_sec != 0 || begin.tv_usec != 0) {
        double elapsed_ms = ((end.tv_sec - begin.tv_sec) * 1000.0) +
                            ((end.tv_usec - begin.tv_usec) / 1000.0);
        if (elapsed_ms > 0.0) {
            app->perf.fps = 1000.0 / elapsed_ms;
        }
    }
    begin = end;
}
