#include "platform_api.h"

#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <tcc_overlay_ioctl.h>
#include <tcc_scaler_ioctrl.h>

#define CAMERA_BUFFER_COUNT 4

struct camera_buffer {
    uint8_t *virt_addr;
    uint64_t phys_addr;
    uint64_t length;
    int mapped;
};

struct camera_context {
    camera_status_t status;
    int fd;
    struct camera_buffer buffers[CAMERA_BUFFER_COUNT];
    struct v4l2_buffer current_buffer;
    struct v4l2_plane current_planes[3];
};

struct display_context {
    int fd;
};

struct scaler_context {
    int fd[SCALER_INDEX_MAX];
};

static void camera_release_mappings(struct camera_context *ctx)
{
    int i;

    for (i = 0; i < CAMERA_BUFFER_COUNT; ++i) {
        if (ctx->buffers[i].mapped) {
            (void)munmap(ctx->buffers[i].virt_addr, ctx->buffers[i].length);
            ctx->buffers[i].mapped = 0;
        }
    }
}

int camera_create(camera_handle_t *handle)
{
    struct camera_context *ctx;

    if (handle == NULL) {
        return -1;
    }

    ctx = (struct camera_context *)calloc(1, sizeof(*ctx));
    if (ctx == NULL) {
        return -1;
    }

    ctx->fd = -1;
    *handle = ctx;
    return 0;
}

int camera_destroy(camera_handle_t handle)
{
    if (handle != NULL) {
        free(handle);
    }
    return 0;
}

int camera_open_device(camera_handle_t handle, const char *device_path)
{
    struct camera_context *ctx = handle;
    struct v4l2_capability capability;

    if (ctx == NULL || device_path == NULL) {
        return -1;
    }

    ctx->fd = open(device_path, O_RDWR);
    if (ctx->fd < 0) {
        return -1;
    }

    if (ioctl(ctx->fd, VIDIOC_QUERYCAP, &capability) < 0) {
        (void)close(ctx->fd);
        ctx->fd = -1;
        return -1;
    }

    if ((capability.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) == 0) {
        (void)close(ctx->fd);
        ctx->fd = -1;
        return -1;
    }

    ctx->status = CAMERA_STATUS_OPENED;
    return 0;
}

int camera_close_device(camera_handle_t handle)
{
    struct camera_context *ctx = handle;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    if (ctx == NULL || ctx->fd < 0) {
        return -1;
    }

    if (ctx->status == CAMERA_STATUS_STREAMING) {
        (void)ioctl(ctx->fd, VIDIOC_STREAMOFF, &type);
    }

    camera_release_mappings(ctx);
    (void)close(ctx->fd);
    ctx->fd = -1;
    ctx->status = CAMERA_STATUS_IDLE;
    return 0;
}

int camera_set_config(camera_handle_t handle, uint32_t width, uint32_t height)
{
    struct camera_context *ctx = handle;
    struct v4l2_format format;
    struct v4l2_requestbuffers req;
    unsigned int i;

    if (ctx == NULL || ctx->fd < 0) {
        return -1;
    }

    (void)memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    format.fmt.pix_mp.width = width;
    format.fmt.pix_mp.height = height;
    format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_RGB32;
    format.fmt.pix_mp.num_planes = 1;
    format.fmt.pix_mp.plane_fmt[0].sizeimage = width * height * 4u;

    if (ioctl(ctx->fd, VIDIOC_S_FMT, &format) < 0) {
        return -1;
    }

    (void)memset(&req, 0, sizeof(req));
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    req.memory = V4L2_MEMORY_MMAP;
    req.count = CAMERA_BUFFER_COUNT;
    if (ioctl(ctx->fd, VIDIOC_REQBUFS, &req) < 0) {
        return -1;
    }

    for (i = 0; i < req.count; ++i) {
        struct v4l2_buffer buffer;
        struct v4l2_plane planes[3];

        (void)memset(&buffer, 0, sizeof(buffer));
        (void)memset(planes, 0, sizeof(planes));
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;
        buffer.length = 1;
        buffer.m.planes = planes;

        if (ioctl(ctx->fd, VIDIOC_QUERYBUF, &buffer) < 0) {
            return -1;
        }

        ctx->buffers[i].length = planes[0].length;
        ctx->buffers[i].phys_addr = planes[0].reserved[0];
        ctx->buffers[i].virt_addr =
            (uint8_t *)mmap(NULL, planes[0].length, PROT_READ | PROT_WRITE,
                            MAP_SHARED, ctx->fd, (off_t)planes[0].m.mem_offset);
        if (ctx->buffers[i].virt_addr == MAP_FAILED) {
            return -1;
        }
        ctx->buffers[i].mapped = 1;

        if (ioctl(ctx->fd, VIDIOC_QBUF, &buffer) < 0) {
            return -1;
        }
    }

    {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        if (ioctl(ctx->fd, VIDIOC_STREAMON, &type) < 0) {
            return -1;
        }
    }

    ctx->status = CAMERA_STATUS_STREAMING;
    return 0;
}

int camera_get_buffer(camera_handle_t handle, uint8_t **virtual_addr,
                      uint64_t *physical_addr)
{
    struct camera_context *ctx = handle;

    if (ctx == NULL || ctx->fd < 0 || virtual_addr == NULL ||
        physical_addr == NULL) {
        return -1;
    }

    (void)memset(&ctx->current_buffer, 0, sizeof(ctx->current_buffer));
    (void)memset(ctx->current_planes, 0, sizeof(ctx->current_planes));
    ctx->current_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    ctx->current_buffer.memory = V4L2_MEMORY_MMAP;
    ctx->current_buffer.length = 1;
    ctx->current_buffer.m.planes = ctx->current_planes;

    if (ioctl(ctx->fd, VIDIOC_DQBUF, &ctx->current_buffer) < 0) {
        return -1;
    }

    *virtual_addr = ctx->buffers[ctx->current_buffer.index].virt_addr;
    *physical_addr = ctx->buffers[ctx->current_buffer.index].phys_addr;
    return (int)ctx->current_planes[0].bytesused;
}

int camera_release_buffer(camera_handle_t handle)
{
    struct camera_context *ctx = handle;

    if (ctx == NULL || ctx->fd < 0) {
        return -1;
    }

    return ioctl(ctx->fd, VIDIOC_QBUF, &ctx->current_buffer);
}

int display_create(display_handle_t *handle)
{
    struct display_context *ctx;

    if (handle == NULL) {
        return -1;
    }

    ctx = (struct display_context *)calloc(1, sizeof(*ctx));
    if (ctx == NULL) {
        return -1;
    }

    ctx->fd = -1;
    *handle = ctx;
    return 0;
}

int display_destroy(display_handle_t handle)
{
    if (handle != NULL) {
        free(handle);
    }
    return 0;
}

int display_open_device(display_handle_t handle, const char *device_path)
{
    if (handle == NULL || device_path == NULL) {
        return -1;
    }

    handle->fd = open(device_path, O_RDWR);
    return handle->fd < 0 ? -1 : 0;
}

int display_close_device(display_handle_t handle)
{
    if (handle == NULL || handle->fd < 0) {
        return -1;
    }

    (void)close(handle->fd);
    handle->fd = -1;
    return 0;
}

int display_show(display_handle_t handle, uint64_t base_addr, uint32_t x,
                 uint32_t y, uint32_t width, uint32_t height)
{
    overlay_video_buffer_t buffer;

    if (handle == NULL || handle->fd < 0) {
        return -1;
    }

    (void)memset(&buffer, 0, sizeof(buffer));
    buffer.cfg.sx = x;
    buffer.cfg.sy = y;
    buffer.cfg.width = width;
    buffer.cfg.height = height;
    buffer.cfg.format = 14;
    buffer.addr = base_addr;

    return ioctl(handle->fd, OVERLAY_PUSH_VIDEO_BUFFER, (unsigned long)&buffer);
}

int scaler_create(scaler_handle_t *handle)
{
    struct scaler_context *ctx;

    if (handle == NULL) {
        return -1;
    }

    ctx = (struct scaler_context *)calloc(1, sizeof(*ctx));
    if (ctx == NULL) {
        return -1;
    }

    ctx->fd[SCALER_INDEX_0] = -1;
    ctx->fd[SCALER_INDEX_1] = -1;
    *handle = ctx;
    return 0;
}

int scaler_destroy(scaler_handle_t handle)
{
    if (handle != NULL) {
        free(handle);
    }
    return 0;
}

int scaler_open_device(scaler_handle_t handle, const char *device_path,
                       scaler_index_t scaler_index)
{
    if (handle == NULL || device_path == NULL || scaler_index >= SCALER_INDEX_MAX) {
        return -1;
    }

    handle->fd[scaler_index] = open(device_path, O_RDWR | O_NDELAY);
    return handle->fd[scaler_index] < 0 ? -1 : 0;
}

int scaler_close_device(scaler_handle_t handle, scaler_index_t scaler_index)
{
    if (handle == NULL || scaler_index >= SCALER_INDEX_MAX ||
        handle->fd[scaler_index] < 0) {
        return -1;
    }

    (void)close(handle->fd[scaler_index]);
    handle->fd[scaler_index] = -1;
    return 0;
}

int scaler_resize(scaler_handle_t handle, scaler_index_t scaler_index,
                  scaler_image_t src, scaler_image_t dst)
{
    struct SCALER_TYPE scaler_info;

    if (handle == NULL || scaler_index >= SCALER_INDEX_MAX ||
        handle->fd[scaler_index] < 0) {
        return -1;
    }

    (void)memset(&scaler_info, 0, sizeof(scaler_info));
    scaler_info.responsetype = SCALER_INTERRUPT;
    scaler_info.src_Yaddr = src.paddr;
    scaler_info.src_Uaddr = src.paddr;
    scaler_info.src_Vaddr = src.paddr;
    scaler_info.src_winRight = src.width;
    scaler_info.src_winBottom = src.height;
    scaler_info.src_ImgWidth = src.width;
    scaler_info.src_ImgHeight = src.height;
    scaler_info.src_fmt = src.format;
    scaler_info.dest_Yaddr = dst.paddr;
    scaler_info.dest_Uaddr = dst.paddr;
    scaler_info.dest_Vaddr = dst.paddr;
    scaler_info.dest_winRight = dst.width;
    scaler_info.dest_winBottom = dst.height;
    scaler_info.dest_ImgWidth = dst.width;
    scaler_info.dest_ImgHeight = dst.height;
    scaler_info.dest_fmt = dst.format;

    return ioctl(handle->fd[scaler_index], TCC_SCALER_IOCTRL, (uint64_t)&scaler_info);
}

int scaler_poll(scaler_handle_t handle, scaler_index_t scaler_index)
{
    struct pollfd event;
    int ret;

    if (handle == NULL || scaler_index >= SCALER_INDEX_MAX ||
        handle->fd[scaler_index] < 0) {
        return -1;
    }

    (void)memset(&event, 0, sizeof(event));
    event.fd = handle->fd[scaler_index];
    event.events = POLLIN;
    ret = poll(&event, 1, 400);
    if (ret <= 0 || (event.revents & POLLERR) != 0) {
        return -1;
    }

    return 0;
}
