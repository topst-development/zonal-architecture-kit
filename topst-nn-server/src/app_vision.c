#include "app_vision.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_memory.h"
#include "message_api.h"

#define VISION_PROTOCOL_RESULT_MAX_SIZE VISION_PROTOCOL_CTRL_DATA_SIZE

static int get_reserved_memory(const char *pmap_name, uint64_t *base,
                               uint64_t *size)
{
    FILE *file = fopen(APP_PMAP_DEVICE_PATH, "r");
    char line[128];
    char read_name[64];
    unsigned long long start;
    unsigned long long end;

    if (file == NULL) {
        perror("open " APP_PMAP_DEVICE_PATH);
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%llx-%llx %*s %63s", &start, &end, read_name) == 3) {
            if (strcmp(read_name, pmap_name) == 0) {
                *base = (uint64_t)start;
                *size = (uint64_t)(end - start + 1ULL);
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return -1;
}

static int configure_recv_buffers(app_context_t *app)
{
    uint64_t base = 0;
    uint64_t size = 0;
    int i;

    if (app->vision.frame_bytes > (size_t)APP_PMAP_SIZE) {
        fprintf(stderr, "[vision] frame too large for pmap slot: frame=%zu slot=%u\n",
                app->vision.frame_bytes, (unsigned int)APP_PMAP_SIZE);
        return -1;
    }

    if (get_reserved_memory(APP_PMAP_NAME_0, &base, &size) != 0) {
        fprintf(stderr, "[vision] failed to find reserved memory %s\n", APP_PMAP_NAME_0);
        return -1;
    }

    if (size < ((uint64_t)APP_PMAP_SIZE * (uint64_t)APP_PMAP_SPLIT_NUMBER)) {
        fprintf(stderr, "[vision] reserved memory %s is smaller than expected: %llu\n",
                APP_PMAP_NAME_0, (unsigned long long)size);
        return -1;
    }

    for (i = 0; i < APP_PMAP_SPLIT_NUMBER; ++i) {
        app->vision.recv_phys[i] = base + ((uint64_t)APP_PMAP_SIZE * (uint64_t)i);
    }

    return 0;
}

int app_vision_init(app_context_t *app)
{
    int ret;

    app->vision.frame_bytes = (size_t)app->camera_width * (size_t)app->camera_height * 3u;
    app->vision.active_frame = NULL;
    app->vision.active_phys = 0;
    app->vision.active_sync = 0;
    app->vision.result_seq = 0;

    if (configure_recv_buffers(app) != 0) {
        return -1;
    }

    printf("[vision] opening tc-nn-app message transport target=%s stream=%d message=%d frame=%zubytes\n",
           app->vision.target_ip, app->vision.stream_port, app->vision.message_port,
           app->vision.frame_bytes);
    printf("[vision] recv pmap slots: 0x%llx 0x%llx 0x%llx 0x%llx\n",
           (unsigned long long)app->vision.recv_phys[0],
           (unsigned long long)app->vision.recv_phys[1],
           (unsigned long long)app->vision.recv_phys[2],
           (unsigned long long)app->vision.recv_phys[3]);

    ret = MessageCreate((MessageHandle *)&app->vision.handle);
    if (ret != 0 || app->vision.handle == NULL) {
        fprintf(stderr, "[vision] MessageCreate failed: %d\n", ret);
        return -1;
    }

    ret = MessageOpen((MessageHandle)app->vision.handle,
                      MESSAGE_STREAM_RECV,
                      app->vision.recv_phys,
                      APP_PMAP_SPLIT_NUMBER,
                      (uint32_t)app->vision.frame_bytes,
                      app->vision.target_ip,
                      (uint16_t)app->vision.stream_port,
                      (uint16_t)app->vision.message_port);
    if (ret != 0) {
        fprintf(stderr, "[vision] MessageOpen failed: %d\n", ret);
        app_vision_deinit(app);
        return -1;
    }

    return 0;
}

void app_vision_deinit(app_context_t *app)
{
    app_vision_release_frame(app);

    if (app->vision.handle != NULL) {
        (void)MessageClose((MessageHandle)app->vision.handle);
        (void)MessageDestroy((MessageHandle)app->vision.handle);
        app->vision.handle = NULL;
    }
}

int app_vision_recv_frame(app_context_t *app)
{
    uint8_t *virtual_addr = NULL;
    uint64_t base_offset = 0;
    uint64_t sync_stamp = 0;
    int ret;

    app_vision_release_frame(app);

    ret = MessagePopReceiveBuffer((MessageHandle)app->vision.handle,
                                  &virtual_addr,
                                  &base_offset,
                                  &sync_stamp);
    if (ret == TIMEOUT_ERROR || ret == QUEUE_UNDERFLOW) {
        return 0;
    }
    if (ret != 0) {
        if (app->stop) {
            return 0;
        }
        fprintf(stderr, "[vision] MessagePopReceiveBuffer failed: %d\n", ret);
        return -1;
    }
    if (virtual_addr == NULL || base_offset == 0) {
        fprintf(stderr, "[vision] invalid receive buffer\n");
        return 0;
    }

    app->vision.active_frame = virtual_addr;
    app->vision.active_phys = base_offset;
    app->vision.active_sync = sync_stamp;
    return 1;
}

void app_vision_release_frame(app_context_t *app)
{
    if (app->vision.handle != NULL && app->vision.active_frame != NULL) {
        (void)MessagePushReceiveBuffer((MessageHandle)app->vision.handle,
                                       app->vision.active_phys);
        app->vision.active_frame = NULL;
        app->vision.active_phys = 0;
        app->vision.active_sync = 0;
    }
}

const uint8_t *app_vision_frame_data(const app_context_t *app)
{
    return app->vision.active_frame;
}

uint64_t app_vision_frame_phys(const app_context_t *app)
{
    return app->vision.active_phys;
}

int app_vision_send_result_json(app_context_t *app, const char *json, size_t length)
{
    message_context_t *context;
    vision_message_header_t header;
    uint8_t *packet;
    size_t total;
    int ret;

    if (app->vision.handle == NULL || json == NULL || length == 0u) {
        return 0;
    }
    if (length > VISION_PROTOCOL_RESULT_MAX_SIZE) {
        fprintf(stderr, "[vision] result JSON too large: %zu\n", length);
        return -1;
    }

    context = (message_context_t *)app->vision.handle;
    if (context->messageHandle == NULL) {
        return 0;
    }

    memset(&header, 0, sizeof(header));
    header.id = VISION_MSG_EVENT_RESULT_DATA_JSON;
    header.length = (uint32_t)length;
    header.seqNum = app->vision.result_seq++;
    header.timestamp = 0;

    total = sizeof(header) + length;
    packet = (uint8_t *)malloc(total);
    if (packet == NULL) {
        return -1;
    }

    memcpy(packet, &header, sizeof(header));
    memcpy(packet + sizeof(header), json, length);

    ret = Vision_API_SendMessage(context->messageHandle, packet, (uint32_t)total, BLOCKING);
    free(packet);

    return (ret == VISION_SUCCESS) ? 0 : -1;
}
