#include "app_memory.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static int get_reserved_memory(const char *pmap_name, unsigned long *base,
                               unsigned long *size)
{
    FILE *file = fopen(APP_PMAP_DEVICE_PATH, "r");
    char line[128];
    char read_name[32];
    unsigned long long start;
    unsigned long long end;

    if (file == NULL) {
        perror("Failed open reserved memory device");
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%llx-%llx %*s %31s", &start, &end, read_name) == 3) {
            if (strcmp(read_name, pmap_name) == 0) {
                *size = (unsigned long)(end - start + 1ULL);
                *base = (unsigned long)start;
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return -1;
}

static void align_buffer_size(uint32_t output_width, uint32_t output_height,
                              uint64_t *aligned_size)
{
    uint64_t raw_size = (uint64_t)output_width * (uint64_t)output_height * 3ULL;
    uint64_t quotient = raw_size / 4096ULL;

    if ((raw_size % 4096ULL) == 0ULL) {
        *aligned_size = quotient * 4096ULL;
    } else {
        *aligned_size = (quotient + 1ULL) * 4096ULL;
    }
}

static int reserved_memory_init(app_memory_context_t *ctx)
{
    const char *pmap_names[APP_PMAP_MAX_NUMBER] = {
        APP_PMAP_NAME_0,
        APP_PMAP_NAME_1,
        APP_PMAP_NAME_2,
        APP_PMAP_NAME_3,
    };
    int i;

    for (i = 0; i < APP_PMAP_MAX_NUMBER; ++i) {
        unsigned long base = 0;
        unsigned long size = 0;
        int j;

        if (get_reserved_memory(pmap_names[i], &base, &size) != 0) {
            fprintf(stderr, "Fail to find reserved memory %s\n", pmap_names[i]);
            return -1;
        }

        for (j = 0; j < APP_PMAP_SPLIT_NUMBER; ++j) {
            ctx->reserved_memory[i][j] = base + ((uint64_t)APP_PMAP_SIZE * (uint64_t)j);
        }
    }

    return 0;
}

static int application_memory_init(app_memory_context_t *ctx, const char *input_path,
                                   uint32_t output_width, uint32_t output_height)
{
    uint64_t aligned_size = 0;
    size_t map_size = (size_t)output_width * (size_t)output_height * 3u;

    ctx->display_memory_fd = open("/dev/mem", O_RDWR | O_NDELAY);
    if (ctx->display_memory_fd < 0) {
        perror("open /dev/mem failed");
        return -1;
    }

    align_buffer_size(output_width, output_height, &aligned_size);
    if (strcmp(input_path, "/dev/video0") == 0) {
        ctx->phy_base_output[0] = ctx->reserved_memory[2][0];
        ctx->phy_base_output[1] = ctx->reserved_memory[2][0] + aligned_size;
    } else if (strcmp(input_path, "/dev/video1") == 0) {
        ctx->phy_base_output[0] = ctx->reserved_memory[2][1];
        ctx->phy_base_output[1] = ctx->reserved_memory[2][1] + aligned_size;
    } else if (strcmp(input_path, "/dev/video2") == 0) {
        ctx->phy_base_output[0] = ctx->reserved_memory[2][2];
        ctx->phy_base_output[1] = ctx->reserved_memory[2][2] + aligned_size;
    } else if (strcmp(input_path, "/dev/video3") == 0) {
        ctx->phy_base_output[0] = ctx->reserved_memory[2][3];
        ctx->phy_base_output[1] = ctx->reserved_memory[2][3] + aligned_size;
    } else {
        ctx->phy_base_output[0] = ctx->reserved_memory[2][0];
        ctx->phy_base_output[1] = ctx->reserved_memory[2][1] + aligned_size;
    }

    ctx->map_base_output[0] = (uint8_t *)mmap(NULL, map_size, PROT_READ | PROT_WRITE,
                                              MAP_SHARED, ctx->display_memory_fd,
                                              ctx->phy_base_output[0]);
    if (ctx->map_base_output[0] == MAP_FAILED) {
        perror("mmap map_base_output[0] failed");
        return -1;
    }

    ctx->map_base_output[1] = (uint8_t *)mmap(NULL, map_size, PROT_READ | PROT_WRITE,
                                              MAP_SHARED, ctx->display_memory_fd,
                                              ctx->phy_base_output[1]);
    if (ctx->map_base_output[1] == MAP_FAILED) {
        perror("mmap map_base_output[1] failed");
        return -1;
    }

    return 0;
}

int app_memory_init(app_memory_context_t *ctx, const char *input_path,
                    uint32_t output_width, uint32_t output_height)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->display_memory_fd = -1;

    if (reserved_memory_init(ctx) != 0) {
        return -1;
    }

    if (application_memory_init(ctx, input_path, output_width, output_height) != 0) {
        return -1;
    }

    return 0;
}

int app_memory_deinit(app_memory_context_t *ctx, uint32_t output_width,
                      uint32_t output_height)
{
    size_t map_size = (size_t)output_width * (size_t)output_height * 3u;

    if (ctx->map_base_output[0] != NULL && ctx->map_base_output[0] != MAP_FAILED) {
        (void)munmap(ctx->map_base_output[0], map_size);
        ctx->map_base_output[0] = NULL;
    }
    if (ctx->map_base_output[1] != NULL && ctx->map_base_output[1] != MAP_FAILED) {
        (void)munmap(ctx->map_base_output[1], map_size);
        ctx->map_base_output[1] = NULL;
    }
    if (ctx->display_memory_fd >= 0) {
        (void)close(ctx->display_memory_fd);
        ctx->display_memory_fd = -1;
    }

    return 0;
}
