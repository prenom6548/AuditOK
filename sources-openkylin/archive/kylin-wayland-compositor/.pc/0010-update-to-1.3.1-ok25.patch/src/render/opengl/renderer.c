// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <drm_fourcc.h>
#include <kywc/log.h>
#include <wlr/types/wlr_matrix.h>

#include "render/opengl.h"
#include "render/pixel_format.h"
#include "render/renderer.h"

#include "linear_gradient_frag_str.h"
#include "linear_gradient_stroke_frag_str.h"
#include "linear_gradient_vert_str.h"
#include "quad_ex_frag_str.h"
#include "quad_ex_vert_str.h"
#include "quad_frag_str.h"
#include "quad_stroke_frag_str.h"
#include "quad_vert_str.h"
#include "tex_common_vert_str.h"
#include "tex_ex_vert_str.h"
#include "tex_external_ex_frag_str.h"
#include "tex_external_frag_str.h"
#include "tex_mix_frag_str.h"
#include "tex_mix_vert_str.h"
#include "tex_rgba_ex_frag_str.h"
#include "tex_rgba_frag_str.h"
#include "tex_rgbx_ex_frag_str.h"
#include "tex_rgbx_frag_str.h"

static const struct wlr_renderer_impl renderer_impl;
static const struct wlr_render_timer_impl render_timer_impl;

bool wlr_renderer_is_opengl(struct wlr_renderer *wlr_renderer)
{
    return wlr_renderer->impl == &renderer_impl;
}

struct opengl_renderer *opengl_renderer_from_wlr_renderer(struct wlr_renderer *wlr_renderer)
{
    assert(wlr_renderer->impl == &renderer_impl);
    struct opengl_renderer *renderer = wl_container_of(wlr_renderer, renderer, wlr_renderer);
    return renderer;
}

static struct opengl_renderer *gl_get_renderer_in_context(struct wlr_renderer *wlr_renderer)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);
    assert(renderer->current_buffer != NULL);
    return renderer;
}

static struct opengl_render_timer *gl_get_render_timer(struct wlr_render_timer *wlr_timer)
{
    assert(wlr_timer->impl == &render_timer_impl);
    struct opengl_render_timer *timer = wl_container_of(wlr_timer, timer, base);
    return timer;
}

static void destroy_buffer(struct opengl_buffer *buffer)
{
    wl_list_remove(&buffer->link);
    wlr_addon_finish(&buffer->addon);

    struct egl_context prev_ctx;
    egl_make_current(buffer->renderer->egl, &prev_ctx);

    opengl_push_debug(buffer->renderer);

    glDeleteFramebuffers(1, &buffer->fbo);
    glDeleteRenderbuffers(1, &buffer->rbo);

    opengl_pop_debug(buffer->renderer);

    egl_destroy_image(buffer->renderer->egl, buffer->image);

    egl_restore_context(&prev_ctx);

    free(buffer);
}

static void handle_buffer_destroy(struct wlr_addon *addon)
{
    struct opengl_buffer *buffer = wl_container_of(addon, buffer, addon);
    destroy_buffer(buffer);
}

static const struct wlr_addon_interface buffer_addon_impl = {
    .name = "opengl_buffer",
    .destroy = handle_buffer_destroy,
};

static struct opengl_buffer *get_or_create_buffer(struct opengl_renderer *renderer,
                                                  struct wlr_buffer *wlr_buffer)
{
    struct wlr_addon *addon = wlr_addon_find(&wlr_buffer->addons, renderer, &buffer_addon_impl);
    if (addon) {
        struct opengl_buffer *buffer = wl_container_of(addon, buffer, addon);
        return buffer;
    }

    struct opengl_buffer *buffer = calloc(1, sizeof(*buffer));
    if (buffer == NULL) {
        kywc_log_errno(KYWC_ERROR, "Allocation failed");
        return NULL;
    }
    buffer->buffer = wlr_buffer;
    buffer->renderer = renderer;

    struct wlr_dmabuf_attributes dmabuf = { 0 };
    if (!wlr_buffer_get_dmabuf(wlr_buffer, &dmabuf)) {
        goto error_buffer;
    }

    bool external_only = renderer->exts.OES_egl_image_external;
    buffer->image = egl_create_image_from_dmabuf(renderer->egl, &dmabuf, &external_only);
    if (buffer->image == EGL_NO_IMAGE_KHR) {
        goto error_buffer;
    }

    opengl_push_debug(renderer);

    glGenRenderbuffers(1, &buffer->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer->rbo);
    glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, buffer->image);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &buffer->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer->rbo);
    GLenum fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    opengl_pop_debug(renderer);

    if (fb_status != GL_FRAMEBUFFER_COMPLETE) {
        kywc_log(KYWC_ERROR, "Failed to create FBO");
        goto error_image;
    }

    buffer->has_alpha = !render_buffer_is_opaque(wlr_buffer);
    wlr_addon_init(&buffer->addon, &wlr_buffer->addons, renderer, &buffer_addon_impl);

    wl_list_insert(&renderer->buffers, &buffer->link);

    kywc_log(KYWC_DEBUG, "Created GL FBO for buffer %dx%d", wlr_buffer->width, wlr_buffer->height);

    return buffer;

error_image:
    egl_destroy_image(renderer->egl, buffer->image);
error_buffer:
    free(buffer);
    return NULL;
}

static bool gl_bind_buffer(struct wlr_renderer *wlr_renderer, struct wlr_buffer *wlr_buffer)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);

    if (renderer->current_buffer != NULL) {
        opengl_push_debug(renderer);
        glFlush();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        opengl_pop_debug(renderer);

        wlr_buffer_unlock(renderer->current_buffer->buffer);
        renderer->current_buffer = NULL;
    }

    if (wlr_buffer == NULL) {
        egl_unset_current(renderer->egl);
        return true;
    }

    egl_make_current(renderer->egl, NULL);

    struct opengl_buffer *buffer = get_or_create_buffer(renderer, wlr_buffer);
    if (buffer == NULL) {
        return false;
    }

    wlr_buffer_lock(wlr_buffer);
    renderer->current_buffer = buffer;

    opengl_push_debug(renderer);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->current_buffer->fbo);
    opengl_pop_debug(renderer);

    return true;
}

static const char *reset_status_str(GLenum status)
{
    switch (status) {
    case GL_GUILTY_CONTEXT_RESET_KHR:
        return "guilty";
    case GL_INNOCENT_CONTEXT_RESET_KHR:
        return "innocent";
    case GL_UNKNOWN_CONTEXT_RESET_KHR:
        return "unknown";
    default:
        return "<invalid>";
    }
}

static bool gl_begin(struct wlr_renderer *wlr_renderer, uint32_t width, uint32_t height)
{
    struct opengl_renderer *renderer = gl_get_renderer_in_context(wlr_renderer);

    opengl_push_debug(renderer);

    if (renderer->exts.KHR_robustness) {
        GLenum status = glGetGraphicsResetStatusKHR();
        if (status != GL_NO_ERROR) {
            kywc_log(KYWC_ERROR, "GPU reset (%s)", reset_status_str(status));
            wl_signal_emit_mutable(&wlr_renderer->events.lost, NULL);
            return false;
        }
    }

    glViewport(0, 0, width, height);

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    opengl_pop_debug(renderer);

    return true;
}

static void gl_end(struct wlr_renderer *wlr_renderer)
{
    gl_get_renderer_in_context(wlr_renderer);
    // no-op
}

static const uint32_t *gl_get_shm_texture_formats(struct wlr_renderer *wlr_renderer, size_t *len)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);
    return pixel_format_get_drm_formats_for_gl(len, renderer);
}

static const struct wlr_drm_format_set *
gl_get_dmabuf_texture_formats(struct wlr_renderer *wlr_renderer)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);
    return &renderer->egl->dmabuf_texture_formats;
}

static const struct wlr_drm_format_set *gl_get_render_formats(struct wlr_renderer *wlr_renderer)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);
    return &renderer->egl->dmabuf_render_formats;
}

static uint32_t gl_preferred_read_format(struct wlr_renderer *wlr_renderer)
{
    struct opengl_renderer *renderer = gl_get_renderer_in_context(wlr_renderer);

    opengl_push_debug(renderer);

    GLint gl_format = -1, gl_type = -1, alpha_size = -1;
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &gl_format);
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &gl_type);
    glGetIntegerv(GL_ALPHA_BITS, &alpha_size);

    opengl_pop_debug(renderer);

    const struct pixel_format *fmt = pixel_format_from_gl(gl_format, gl_type, alpha_size > 0);
    if (fmt != NULL) {
        return fmt->drm_format;
    }

    if (renderer->exts.EXT_read_format_bgra) {
        return DRM_FORMAT_XRGB8888;
    }
    return DRM_FORMAT_XBGR8888;
}

static bool gl_read_pixels(struct wlr_renderer *wlr_renderer, uint32_t drm_format, uint32_t stride,
                           uint32_t width, uint32_t height, uint32_t src_x, uint32_t src_y,
                           uint32_t dst_x, uint32_t dst_y, void *data)
{
    struct opengl_renderer *renderer = gl_get_renderer_in_context(wlr_renderer);

    const struct pixel_format *fmt = pixel_format_from_drm(drm_format);
    if (fmt == NULL || !pixel_format_is_support_by_gl(fmt, renderer)) {
        kywc_log(KYWC_ERROR, "Cannot read pixels: unsupported pixel format 0x%" PRIX32, drm_format);
        return false;
    }

    if (fmt->gl_format == GL_BGRA_EXT && !renderer->exts.EXT_read_format_bgra) {
        kywc_log(KYWC_ERROR, "Cannot read pixels: missing GL_EXT_read_format_bgra extension");
        return false;
    }

    if (pixel_format_pixels_per_block(fmt) != 1) {
        kywc_log(KYWC_ERROR, "Cannot read pixels: block formats are not supported");
        return false;
    }

    opengl_push_debug(renderer);

    // Make sure any pending drawing is finished before we try to read it
    glFinish();

    glGetError(); // Clear the error flag

    unsigned char *p = (unsigned char *)data + dst_y * stride;
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    uint32_t pack_stride = pixel_format_min_stride(fmt, width);
    if (pack_stride == stride && dst_x == 0) {
        // Under these particular conditions, we can read the pixels with only
        // one glReadPixels call
        glReadPixels(src_x, src_y, width, height, fmt->gl_format, fmt->gl_type, p);
    } else {
        // Unfortunately GLES2 doesn't support GL_PACK_ROW_LENGTH, so we have to read
        // the lines out row by row
        for (size_t i = 0; i < height; ++i) {
            uint32_t y = src_y + i;
            glReadPixels(src_x, y, width, 1, fmt->gl_format, fmt->gl_type,
                         p + i * stride + dst_x * fmt->bytes_per_block);
        }
    }

    opengl_pop_debug(renderer);

    return glGetError() == GL_NO_ERROR;
}

static int gl_get_drm_fd(struct wlr_renderer *wlr_renderer)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);

    if (renderer->drm_fd < 0) {
        renderer->drm_fd = egl_dup_drm_fd(renderer->egl);
    }

    return renderer->drm_fd;
}

static uint32_t gl_get_render_buffer_caps(struct wlr_renderer *wlr_renderer)
{
    return WLR_BUFFER_CAP_DMABUF;
}

struct egl *opengl_renderer_get_egl(struct wlr_renderer *wlr_renderer)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);
    return renderer->egl;
}

static void gl_destroy(struct wlr_renderer *wlr_renderer)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);

    egl_make_current(renderer->egl, NULL);

    struct opengl_buffer *buffer, *buffer_tmp;
    wl_list_for_each_safe(buffer, buffer_tmp, &renderer->buffers, link) {
        destroy_buffer(buffer);
    }

    struct opengl_texture *tex, *tex_tmp;
    wl_list_for_each_safe(tex, tex_tmp, &renderer->textures, link) {
        opengl_texture_destroy(tex);
    }

    opengl_push_debug(renderer);
    glDeleteProgram(renderer->shaders.quad.program);
    glDeleteProgram(renderer->shaders.tex_rgba.program);
    glDeleteProgram(renderer->shaders.tex_rgbx.program);
    glDeleteProgram(renderer->shaders.tex_ext.program);
    opengl_pop_debug(renderer);

    if (renderer->exts.KHR_debug) {
        glDisable(GL_DEBUG_OUTPUT_KHR);
        glDebugMessageCallbackKHR(NULL, NULL);
    }

    egl_unset_current(renderer->egl);
    egl_destroy(renderer->egl);

    if (renderer->drm_fd >= 0) {
        close(renderer->drm_fd);
    }

    free(renderer);
}

static struct wlr_render_pass *gl_begin_buffer_pass(struct wlr_renderer *wlr_renderer,
                                                    struct wlr_buffer *wlr_buffer,
                                                    const struct wlr_buffer_pass_options *options)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);

    struct egl_context prev_ctx = { 0 };
    if (!egl_make_current(renderer->egl, &prev_ctx)) {
        return NULL;
    }

    struct opengl_render_timer *timer = NULL;
    if (options->timer) {
        timer = gl_get_render_timer(options->timer);
        clock_gettime(CLOCK_MONOTONIC, &timer->cpu_start);
    }

    struct opengl_buffer *buffer = get_or_create_buffer(renderer, wlr_buffer);
    if (!buffer) {
        return NULL;
    }

    struct opengl_render_pass *pass = opengl_begin_buffer_pass(buffer, &prev_ctx, timer);
    if (!pass) {
        return NULL;
    }
    return &pass->base;
}

static struct wlr_render_timer *gl_render_timer_create(struct wlr_renderer *wlr_renderer)
{
    struct opengl_renderer *renderer = opengl_renderer_from_wlr_renderer(wlr_renderer);
    if (!renderer->exts.EXT_disjoint_timer_query) {
        kywc_log(KYWC_ERROR, "Can't create timer, EXT_disjoint_timer_query not available");
        return NULL;
    }

    struct opengl_render_timer *timer = calloc(1, sizeof(*timer));
    if (!timer) {
        return NULL;
    }
    timer->base.impl = &render_timer_impl;
    timer->renderer = renderer;

    struct egl_context prev_ctx;
    egl_make_current(renderer->egl, &prev_ctx);
    glGenQueriesEXT(1, &timer->id);
    egl_restore_context(&prev_ctx);

    return &timer->base;
}

static int64_t timespec_to_nsec(const struct timespec *a)
{
    return (int64_t)a->tv_sec * 1000000000 + a->tv_nsec;
}

static int gl_get_render_time(struct wlr_render_timer *wlr_timer)
{
    struct opengl_render_timer *timer = gl_get_render_timer(wlr_timer);
    struct opengl_renderer *renderer = timer->renderer;

    struct egl_context prev_ctx;
    egl_make_current(renderer->egl, &prev_ctx);

    GLint64 disjoint;
    glGetInteger64v(GL_GPU_DISJOINT_EXT, &disjoint);
    if (disjoint) {
        kywc_log(KYWC_ERROR, "A disjoint operation occurred and the render timer is invalid");
        egl_restore_context(&prev_ctx);
        return -1;
    }

    GLint available;
    glGetQueryObjectivEXT(timer->id, GL_QUERY_RESULT_AVAILABLE_EXT, &available);
    if (!available) {
        kywc_log(KYWC_ERROR, "Timer was read too early, gpu isn't done!");
        egl_restore_context(&prev_ctx);
        return -1;
    }

    GLuint64 gl_render_end;
    glGetQueryObjectui64vEXT(timer->id, GL_QUERY_RESULT_EXT, &gl_render_end);

    int64_t cpu_nsec_total =
        timespec_to_nsec(&timer->cpu_end) - timespec_to_nsec(&timer->cpu_start);

    egl_restore_context(&prev_ctx);
    return gl_render_end - timer->gl_cpu_end + cpu_nsec_total;
}

static void gl_render_timer_destroy(struct wlr_render_timer *wlr_timer)
{
    struct opengl_render_timer *timer = wl_container_of(wlr_timer, timer, base);
    struct opengl_renderer *renderer = timer->renderer;

    struct egl_context prev_ctx;
    egl_make_current(renderer->egl, &prev_ctx);
    glDeleteQueriesEXT(1, &timer->id);
    egl_restore_context(&prev_ctx);
    free(timer);
}

static const struct wlr_renderer_impl renderer_impl = {
    .destroy = gl_destroy,
    .bind_buffer = gl_bind_buffer,
    .begin = gl_begin,
    .end = gl_end,
    .get_shm_texture_formats = gl_get_shm_texture_formats,
    .get_dmabuf_texture_formats = gl_get_dmabuf_texture_formats,
    .get_render_formats = gl_get_render_formats,
    .preferred_read_format = gl_preferred_read_format,
    .read_pixels = gl_read_pixels,
    .get_drm_fd = gl_get_drm_fd,
    .get_render_buffer_caps = gl_get_render_buffer_caps,
    .texture_from_buffer = opengl_texture_from_buffer,
    .begin_buffer_pass = gl_begin_buffer_pass,
    .render_timer_create = gl_render_timer_create,
};

static const struct wlr_render_timer_impl render_timer_impl = {
    .get_duration_ns = gl_get_render_time,
    .destroy = gl_render_timer_destroy,
};

// https://www.khronos.org/opengl/wiki/Debug_Output
void opengl_push_debug_(struct opengl_renderer *renderer, const char *file, const char *func)
{
    if (!renderer->exts.KHR_debug) {
        return;
    }

    int len = snprintf(NULL, 0, "%s:%s", file, func) + 1;
    char str[len];
    snprintf(str, len, "%s:%s", file, func);
    glPushDebugGroupKHR(GL_DEBUG_SOURCE_APPLICATION_KHR, 1, -1, str);
}

void opengl_pop_debug(struct opengl_renderer *renderer)
{
    if (renderer->exts.KHR_debug) {
        glPopDebugGroupKHR();
    }
}

static enum kywc_log_level gl_log_level_to_kywc(GLenum type)
{
    switch (type) {
    case GL_DEBUG_TYPE_ERROR_KHR:
        return KYWC_ERROR;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR:
        return KYWC_DEBUG;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR:
        return KYWC_ERROR;
    case GL_DEBUG_TYPE_PORTABILITY_KHR:
        return KYWC_DEBUG;
    case GL_DEBUG_TYPE_PERFORMANCE_KHR:
        return KYWC_DEBUG;
    case GL_DEBUG_TYPE_OTHER_KHR:
        return KYWC_DEBUG;
    case GL_DEBUG_TYPE_MARKER_KHR:
        return KYWC_DEBUG;
    case GL_DEBUG_TYPE_PUSH_GROUP_KHR:
        return KYWC_DEBUG;
    case GL_DEBUG_TYPE_POP_GROUP_KHR:
        return KYWC_DEBUG;
    default:
        return KYWC_DEBUG;
    }
}

static void gl_log(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei len,
                   const GLchar *msg, const void *user)
{
    kywc_log(gl_log_level_to_kywc(type), "[GL] %s", msg);
}

static GLuint compile_shader(struct opengl_renderer *renderer, GLenum type, const GLchar *src)
{
    opengl_push_debug(renderer);

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == GL_FALSE) {
        kywc_log(KYWC_ERROR, "Failed to compile shader");
        glDeleteShader(shader);
        shader = 0;
    }

    opengl_pop_debug(renderer);
    return shader;
}

GLuint opengl_create_program(struct opengl_renderer *renderer, const GLchar *vert_src,
                             const GLchar *frag_src)
{
    opengl_push_debug(renderer);

    GLuint vert = compile_shader(renderer, GL_VERTEX_SHADER, vert_src);
    if (!vert) {
        goto error;
    }

    GLuint frag = compile_shader(renderer, GL_FRAGMENT_SHADER, frag_src);
    if (!frag) {
        glDeleteShader(vert);
        goto error;
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    glDetachShader(prog, vert);
    glDetachShader(prog, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (ok == GL_FALSE) {
        kywc_log(KYWC_ERROR, "Failed to link shader: \n%s\n%s", vert_src, frag_src);
        glDeleteProgram(prog);
        goto error;
    }

    opengl_pop_debug(renderer);
    return prog;

error:
    opengl_pop_debug(renderer);
    return 0;
}

static struct wlr_renderer *opengl_renderer_create(struct egl *egl)
{
    if (!egl_make_current(egl, NULL)) {
        return NULL;
    }

    const char *gl_renderer = (const char *)glGetString(GL_RENDERER);
    /* reject software renderer */
    if (!egl->is_software && (strstr(gl_renderer, "softpipe") ||
                              strncmp("llvmpipe", gl_renderer, strlen("llvmpipe")) == 0)) {
        kywc_log(KYWC_ERROR, "Refusing to use renderer: %s", gl_renderer);
        return NULL;
    }

    struct opengl_renderer *renderer = calloc(1, sizeof(*renderer));
    if (!renderer) {
        return NULL;
    }
    wlr_renderer_init(&renderer->wlr_renderer, &renderer_impl);

    wl_list_init(&renderer->buffers);
    wl_list_init(&renderer->textures);

    renderer->egl = egl;
    renderer->drm_fd = -1;

    kywc_log(KYWC_INFO, "Creating OpenGL renderer");
    kywc_log(KYWC_INFO, "Using %s", glGetString(GL_VERSION));
    kywc_log(KYWC_INFO, "GL vendor: %s", glGetString(GL_VENDOR));
    kywc_log(KYWC_INFO, "GL renderer: %s", gl_renderer);

    if (!renderer->egl->exts.EXT_image_dma_buf_import) {
        kywc_log(KYWC_ERROR, "EGL_EXT_image_dma_buf_import not supported");
        free(renderer);
        return NULL;
    }
    if (egl->is_gles && !epoxy_has_gl_extension("GL_EXT_texture_format_BGRA8888")) {
        kywc_log(KYWC_ERROR, "BGRA8888 format not supported by GLES");
        free(renderer);
        return NULL;
    }
    if (egl->is_gles && epoxy_gl_version() < 30 &&
        !epoxy_has_gl_extension("GL_EXT_unpack_subimage")) {
        kywc_log(KYWC_ERROR, "GL_EXT_unpack_subimage not supported");
        free(renderer);
        return NULL;
    }

    renderer->exts.EXT_read_format_bgra =
        !egl->is_gles || epoxy_has_gl_extension("GL_EXT_read_format_bgra");

    renderer->exts.EXT_texture_type_2_10_10_10_REV =
        epoxy_has_gl_extension("GL_EXT_texture_type_2_10_10_10_REV");

    renderer->exts.OES_texture_half_float_linear =
        epoxy_has_gl_extension("GL_OES_texture_half_float_linear");

    renderer->exts.EXT_texture_norm16 = epoxy_has_gl_extension("GL_EXT_texture_norm16");

    renderer->exts.OES_egl_image_external = epoxy_has_gl_extension("GL_OES_EGL_image_external");

    renderer->exts.OES_egl_image = epoxy_has_gl_extension("GL_OES_EGL_image");

    renderer->exts.KHR_robustness = epoxy_has_gl_extension("GL_KHR_robustness");
    if (renderer->exts.KHR_robustness) {
        GLint notif_strategy = 0;
        glGetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_KHR, &notif_strategy);
        switch (notif_strategy) {
        case GL_LOSE_CONTEXT_ON_RESET_KHR:
            kywc_log(KYWC_DEBUG, "GPU reset notifications are enabled");
            break;
        case GL_NO_RESET_NOTIFICATION_KHR:
            kywc_log(KYWC_DEBUG, "GPU reset notifications are disabled");
            break;
        }
    }

    renderer->exts.EXT_disjoint_timer_query = epoxy_has_gl_extension("GL_EXT_disjoint_timer_query");

    renderer->exts.KHR_debug = epoxy_has_gl_extension("GL_KHR_debug");
    if (renderer->exts.KHR_debug) {
        glEnable(GL_DEBUG_OUTPUT_KHR);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR);
        glDebugMessageCallbackKHR(gl_log, NULL);
        // Silence unwanted message types
        glDebugMessageControlKHR(GL_DONT_CARE, GL_DEBUG_TYPE_POP_GROUP_KHR, GL_DONT_CARE, 0, NULL,
                                 GL_FALSE);
        glDebugMessageControlKHR(GL_DONT_CARE, GL_DEBUG_TYPE_PUSH_GROUP_KHR, GL_DONT_CARE, 0, NULL,
                                 GL_FALSE);
    }

    opengl_push_debug(renderer);

    GLuint prog;
    renderer->shaders.quad.program = prog =
        opengl_create_program(renderer, quad_vert_str, quad_frag_str);
    if (!renderer->shaders.quad.program) {
        goto error;
    }
    renderer->shaders.quad.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
    renderer->shaders.quad.color = glGetUniformLocation(prog, "color");
    renderer->shaders.quad.uv_attrib = glGetAttribLocation(prog, "inUV");
    renderer->shaders.quad.uv_rotation = -1;
    renderer->shaders.quad.aspect = -1;
    renderer->shaders.quad.anti_aliasing = -1;
    renderer->shaders.quad.round_corner_radius = -1;

    renderer->shaders.tex_rgba.program = prog =
        opengl_create_program(renderer, tex_common_vert_str, tex_rgba_frag_str);
    if (!renderer->shaders.tex_rgba.program) {
        goto error;
    }
    renderer->shaders.tex_rgba.uv2texcoord = glGetUniformLocation(prog, "uv2texcoord");
    renderer->shaders.tex_rgba.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
    renderer->shaders.tex_rgba.tex = glGetUniformLocation(prog, "tex");
    renderer->shaders.tex_rgba.alpha = glGetUniformLocation(prog, "alpha");
    renderer->shaders.tex_rgba.uv_attrib = glGetAttribLocation(prog, "inUV");
    renderer->shaders.tex_rgba.uv_rotation = -1;
    renderer->shaders.tex_rgba.aspect = -1;
    renderer->shaders.tex_rgba.anti_aliasing = -1;
    renderer->shaders.tex_rgba.round_corner_radius = -1;

    renderer->shaders.tex_rgbx.program = prog =
        opengl_create_program(renderer, tex_common_vert_str, tex_rgbx_frag_str);
    if (!renderer->shaders.tex_rgbx.program) {
        goto error;
    }
    renderer->shaders.tex_rgbx.uv2texcoord = glGetUniformLocation(prog, "uv2texcoord");
    renderer->shaders.tex_rgbx.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
    renderer->shaders.tex_rgbx.tex = glGetUniformLocation(prog, "tex");
    renderer->shaders.tex_rgbx.alpha = glGetUniformLocation(prog, "alpha");
    renderer->shaders.tex_rgbx.uv_attrib = glGetAttribLocation(prog, "inUV");
    renderer->shaders.tex_rgbx.uv_rotation = -1;
    renderer->shaders.tex_rgbx.aspect = -1;
    renderer->shaders.tex_rgbx.anti_aliasing = -1;
    renderer->shaders.tex_rgbx.round_corner_radius = -1;

    if (renderer->exts.OES_egl_image_external) {
        renderer->shaders.tex_ext.program = prog =
            opengl_create_program(renderer, tex_common_vert_str, tex_external_frag_str);
        if (!renderer->shaders.tex_ext.program) {
            goto error;
        }
        renderer->shaders.tex_ext.uv2texcoord = glGetUniformLocation(prog, "uv2texcoord");
        renderer->shaders.tex_ext.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
        renderer->shaders.tex_ext.tex = glGetUniformLocation(prog, "tex");
        renderer->shaders.tex_ext.alpha = glGetUniformLocation(prog, "alpha");
        renderer->shaders.tex_ext.uv_attrib = glGetAttribLocation(prog, "inUV");
        renderer->shaders.tex_ext.uv_rotation = -1;
        renderer->shaders.tex_ext.aspect = -1;
        renderer->shaders.tex_ext.anti_aliasing = -1;
        renderer->shaders.tex_ext.round_corner_radius = -1;
    }

    // round corner clip shader
    renderer->shaders.quad_ex.program = prog =
        opengl_create_program(renderer, quad_ex_vert_str, quad_ex_frag_str);
    if (!renderer->shaders.quad_ex.program) {
        goto error;
    }
    renderer->shaders.quad_ex.uv_rotation = glGetUniformLocation(prog, "uvRotation");
    renderer->shaders.quad_ex.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
    renderer->shaders.quad_ex.color = glGetUniformLocation(prog, "color");
    renderer->shaders.quad_ex.anti_aliasing = glGetUniformLocation(prog, "antiAliasing");
    renderer->shaders.quad_ex.aspect = glGetUniformLocation(prog, "aspect");
    renderer->shaders.quad_ex.round_corner_radius = glGetUniformLocation(prog, "roundCornerRadius");
    renderer->shaders.quad_ex.uv_attrib = glGetAttribLocation(prog, "inUV");

    renderer->shaders.tex_rgba_ex.program = prog =
        opengl_create_program(renderer, tex_ex_vert_str, tex_rgba_ex_frag_str);
    if (!renderer->shaders.tex_rgba_ex.program) {
        goto error;
    }
    renderer->shaders.tex_rgba_ex.uv_rotation = glGetUniformLocation(prog, "uvRotation");
    renderer->shaders.tex_rgba_ex.uv2texcoord = glGetUniformLocation(prog, "uv2texcoord");
    renderer->shaders.tex_rgba_ex.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
    renderer->shaders.tex_rgba_ex.tex = glGetUniformLocation(prog, "tex");
    renderer->shaders.tex_rgba_ex.alpha = glGetUniformLocation(prog, "alpha");
    renderer->shaders.tex_rgba_ex.force_opaque = glGetUniformLocation(prog, "forceOpaque");
    renderer->shaders.tex_rgba_ex.anti_aliasing = glGetUniformLocation(prog, "antiAliasing");
    renderer->shaders.tex_rgba_ex.aspect = glGetUniformLocation(prog, "aspect");
    renderer->shaders.tex_rgba_ex.round_corner_radius =
        glGetUniformLocation(prog, "roundCornerRadius");
    renderer->shaders.tex_rgba_ex.uv_attrib = glGetAttribLocation(prog, "inUV");

    renderer->shaders.tex_rgbx_ex.program = prog =
        opengl_create_program(renderer, tex_ex_vert_str, tex_rgbx_ex_frag_str);
    if (!renderer->shaders.tex_rgbx_ex.program) {
        goto error;
    }
    renderer->shaders.tex_rgbx_ex.uv_rotation = glGetUniformLocation(prog, "uvRotation");
    renderer->shaders.tex_rgbx_ex.uv2texcoord = glGetUniformLocation(prog, "uv2texcoord");
    renderer->shaders.tex_rgbx_ex.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
    renderer->shaders.tex_rgbx_ex.tex = glGetUniformLocation(prog, "tex");
    renderer->shaders.tex_rgbx_ex.alpha = glGetUniformLocation(prog, "alpha");
    renderer->shaders.tex_rgbx_ex.force_opaque = glGetUniformLocation(prog, "forceOpaque");
    renderer->shaders.tex_rgbx_ex.anti_aliasing = glGetUniformLocation(prog, "antiAliasing");
    renderer->shaders.tex_rgbx_ex.aspect = glGetUniformLocation(prog, "aspect");
    renderer->shaders.tex_rgbx_ex.round_corner_radius =
        glGetUniformLocation(prog, "roundCornerRadius");
    renderer->shaders.tex_rgbx_ex.uv_attrib = glGetAttribLocation(prog, "inUV");

    if (renderer->exts.OES_egl_image_external) {
        renderer->shaders.tex_ext_ex.program = prog =
            opengl_create_program(renderer, tex_ex_vert_str, tex_external_ex_frag_str);
        if (!renderer->shaders.tex_ext_ex.program) {
            goto error;
        }
        renderer->shaders.tex_ext_ex.uv_rotation = glGetUniformLocation(prog, "uvRotation");
        renderer->shaders.tex_ext_ex.uv2texcoord = glGetUniformLocation(prog, "uv2texcoord");
        renderer->shaders.tex_ext_ex.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
        renderer->shaders.tex_ext_ex.tex = glGetUniformLocation(prog, "tex");
        renderer->shaders.tex_ext_ex.alpha = glGetUniformLocation(prog, "alpha");
        renderer->shaders.tex_ext_ex.force_opaque = glGetUniformLocation(prog, "forceOpaque");
        renderer->shaders.tex_ext_ex.anti_aliasing = glGetUniformLocation(prog, "antiAliasing");
        renderer->shaders.tex_ext_ex.aspect = glGetUniformLocation(prog, "aspect");
        renderer->shaders.tex_ext_ex.round_corner_radius =
            glGetUniformLocation(prog, "roundCornerRadius");
        renderer->shaders.tex_ext_ex.uv_attrib = glGetAttribLocation(prog, "inUV");
    }

    renderer->shaders.tex_mix.program = prog =
        opengl_create_program(renderer, tex_mix_vert_str, tex_mix_frag_str);
    if (!renderer->shaders.tex_mix.program) {
        goto error;
    }

    renderer->shaders.tex_mix.uv2texcoord = glGetUniformLocation(prog, "uv2texcoord");
    renderer->shaders.tex_mix.uv2texcoord2 = glGetUniformLocation(prog, "uv2texcoord2");
    renderer->shaders.tex_mix.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
    renderer->shaders.tex_mix.tex_a = glGetUniformLocation(prog, "textureA");
    renderer->shaders.tex_mix.tex_b = glGetUniformLocation(prog, "textureB");
    renderer->shaders.tex_mix.alpha = glGetUniformLocation(prog, "alpha");
    renderer->shaders.tex_mix.uvpos_attrib = glGetAttribLocation(prog, "inUV");
    renderer->shaders.tex_mix.blend_factor = glGetUniformLocation(prog, "blendFactor");

    // quad_stroke
    renderer->shaders.quad_stroke.program = prog =
        opengl_create_program(renderer, quad_ex_vert_str, quad_stroke_frag_str);
    if (!renderer->shaders.quad_stroke.program) {
        goto error;
    }
    renderer->shaders.quad_stroke.uv_rotation = glGetUniformLocation(prog, "uvRotation");
    renderer->shaders.quad_stroke.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
    renderer->shaders.quad_stroke.color = glGetUniformLocation(prog, "color");
    renderer->shaders.quad_stroke.stroke_width = glGetUniformLocation(prog, "strokeWidth");
    renderer->shaders.quad_stroke.anti_aliasing = glGetUniformLocation(prog, "antiAliasing");
    renderer->shaders.quad_stroke.aspect = glGetUniformLocation(prog, "aspect");
    renderer->shaders.quad_stroke.round_corner_radius =
        glGetUniformLocation(prog, "roundCornerRadius");
    renderer->shaders.quad_stroke.uv_attrib = glGetAttribLocation(prog, "inUV");

    // linear_gradient
    renderer->shaders.linear_gradient.program = prog =
        opengl_create_program(renderer, linear_gradient_vert_str, linear_gradient_frag_str);
    if (!renderer->shaders.linear_gradient.program) {
        goto error;
    }

    renderer->shaders.linear_gradient.in_uv = glGetAttribLocation(prog, "inUV");
    renderer->shaders.linear_gradient.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
    renderer->shaders.linear_gradient.uv_rotation = glGetUniformLocation(prog, "uvRotation");
    renderer->shaders.linear_gradient.start = glGetUniformLocation(prog, "start");
    renderer->shaders.linear_gradient.end = glGetUniformLocation(prog, "end");
    renderer->shaders.linear_gradient.color_stop0_offset =
        glGetUniformLocation(prog, "colorStops[0].offset");
    renderer->shaders.linear_gradient.color_stop0_color =
        glGetUniformLocation(prog, "colorStops[0].color");
    renderer->shaders.linear_gradient.color_stop1_offset =
        glGetUniformLocation(prog, "colorStops[1].offset");
    renderer->shaders.linear_gradient.color_stop1_color =
        glGetUniformLocation(prog, "colorStops[1].color");
    renderer->shaders.linear_gradient.anti_aliasing = glGetUniformLocation(prog, "antiAliasing");
    renderer->shaders.linear_gradient.aspect = glGetUniformLocation(prog, "aspect");
    renderer->shaders.linear_gradient.round_corner_radius =
        glGetUniformLocation(prog, "roundCornerRadius");

    // linear_gradient_stroke
    renderer->shaders.linear_gradient_stroke.program = prog =
        opengl_create_program(renderer, linear_gradient_vert_str, linear_gradient_stroke_frag_str);
    if (!renderer->shaders.linear_gradient_stroke.program) {
        goto error;
    }

    renderer->shaders.linear_gradient_stroke.in_uv = glGetAttribLocation(prog, "inUV");
    renderer->shaders.linear_gradient_stroke.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
    renderer->shaders.linear_gradient_stroke.uv_rotation = glGetUniformLocation(prog, "uvRotation");
    renderer->shaders.linear_gradient_stroke.start = glGetUniformLocation(prog, "start");
    renderer->shaders.linear_gradient_stroke.end = glGetUniformLocation(prog, "end");
    renderer->shaders.linear_gradient_stroke.color_stop0_offset =
        glGetUniformLocation(prog, "colorStops[0].offset");
    renderer->shaders.linear_gradient_stroke.color_stop0_color =
        glGetUniformLocation(prog, "colorStops[0].color");
    renderer->shaders.linear_gradient_stroke.color_stop1_offset =
        glGetUniformLocation(prog, "colorStops[1].offset");
    renderer->shaders.linear_gradient_stroke.color_stop1_color =
        glGetUniformLocation(prog, "colorStops[1].color");
    renderer->shaders.linear_gradient_stroke.stroke_width =
        glGetUniformLocation(prog, "strokeWidth");
    renderer->shaders.linear_gradient_stroke.anti_aliasing =
        glGetUniformLocation(prog, "antiAliasing");
    renderer->shaders.linear_gradient_stroke.aspect = glGetUniformLocation(prog, "aspect");
    renderer->shaders.linear_gradient_stroke.round_corner_radius =
        glGetUniformLocation(prog, "roundCornerRadius");

    opengl_pop_debug(renderer);

    egl_unset_current(renderer->egl);

    return &renderer->wlr_renderer;

error:
    glDeleteProgram(renderer->shaders.quad.program);
    glDeleteProgram(renderer->shaders.tex_rgba.program);
    glDeleteProgram(renderer->shaders.tex_rgbx.program);
    glDeleteProgram(renderer->shaders.tex_ext.program);
    glDeleteProgram(renderer->shaders.quad_ex.program);
    glDeleteProgram(renderer->shaders.tex_rgba_ex.program);
    glDeleteProgram(renderer->shaders.tex_rgbx_ex.program);
    glDeleteProgram(renderer->shaders.tex_ext_ex.program);
    glDeleteProgram(renderer->shaders.quad_stroke.program);
    glDeleteProgram(renderer->shaders.linear_gradient.program);
    glDeleteProgram(renderer->shaders.linear_gradient_stroke.program);

    opengl_pop_debug(renderer);

    if (renderer->exts.KHR_debug) {
        glDisable(GL_DEBUG_OUTPUT_KHR);
        glDebugMessageCallbackKHR(NULL, NULL);
    }

    egl_unset_current(renderer->egl);

    free(renderer);
    return NULL;
}

struct wlr_renderer *opengl_renderer_create_with_drm_fd(int drm_fd)
{
    struct egl *egl = egl_create_with_drm_fd(drm_fd);
    if (!egl) {
        kywc_log(KYWC_ERROR, "Could not initialize EGL");
        return NULL;
    }

    struct wlr_renderer *renderer = opengl_renderer_create(egl);
    if (!renderer) {
        kywc_log(KYWC_ERROR, "Failed to create OpenGL renderer");
        egl_destroy(egl);
        return NULL;
    }

    return renderer;
}
