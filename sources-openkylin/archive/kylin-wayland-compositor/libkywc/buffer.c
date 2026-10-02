// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>
#include <string.h>

#include <epoxy/egl.h>
#include <epoxy/gl.h>
#include <libdrm/drm_fourcc.h>
#include <png.h>
#include <sys/mman.h>

#include "buffer.h"

#define DEBUG 0
#if DEBUG
#define debug(fmt, ...) fprintf(stdout, fmt "\n", ##__VA_ARGS__)
#else
#define debug(fmt, ...) ((void)0)
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

struct kywc_buffer_helper {
    kywc_context *ctx;
    struct wl_display *wl_display;
    struct wl_list buffers;

    EGLDisplay display;
    EGLContext context;

    struct {
        bool EXT_platform_wayland;
        bool EXT_image_dma_buf_import;
        bool EXT_image_dma_buf_import_modifiers;
    } exts;

    struct {
        size_t len;
        size_t capacity;
        struct buffer_format *formats;
    } format_set;
};

struct kywc_buffer {
    struct kywc_buffer_helper *helper;
    struct wl_list link;

    void *ptr;
    size_t size;

    EGLImageKHR image;
    GLuint tex, fbo;

    void *user_data;
    struct kywc_thumbnail_buffer buffer;
};

static bool egl_check_dmabuf_import(struct kywc_buffer_helper *helper, EGLDisplay display)
{
    EGLint major, minor;
    if (eglInitialize(display, &major, &minor) == EGL_FALSE) {
        fprintf(stderr, "Failed to initialize EGL\n");
        return false;
    }

    const char *display_exts_str = eglQueryString(display, EGL_EXTENSIONS);
    if (display_exts_str == NULL) {
        fprintf(stderr, "Failed to query EGL display extensions\n");
        return false;
    }

    debug("Using EGL %d.%d", (int)major, (int)minor);
    debug("Supported EGL display extensions: %s", display_exts_str);
    debug("EGL vendor: %s", eglQueryString(display, EGL_VENDOR));

    if (!epoxy_extension_in_string(display_exts_str, "EGL_KHR_surfaceless_context") ||
        !epoxy_extension_in_string(display_exts_str, "EGL_KHR_no_config_context")) {
        fprintf(stderr, "Failed to query EGL surfaceless and no_config context\n");
        return false;
    }

    helper->exts.EXT_image_dma_buf_import =
        epoxy_extension_in_string(display_exts_str, "EGL_EXT_image_dma_buf_import");
    helper->exts.EXT_image_dma_buf_import_modifiers =
        epoxy_extension_in_string(display_exts_str, "EGL_EXT_image_dma_buf_import_modifiers");

    return helper->exts.EXT_image_dma_buf_import;
}

static EGLDisplay egl_init_with_wayland(struct kywc_buffer_helper *helper)
{
    if (!helper->exts.EXT_platform_wayland) {
        return EGL_NO_DISPLAY;
    }

    EGLDisplay egl_display =
        eglGetPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_EXT, helper->wl_display, NULL);
    if (egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "egl wayland get display failed\n");
        return EGL_NO_DISPLAY;
    }

    if (!egl_check_dmabuf_import(helper, egl_display)) {
        fprintf(stderr, "egl wayland not support dmabuf import\n");
        eglTerminate(egl_display);
        return EGL_NO_DISPLAY;
    }

    return egl_display;
}

static int egl_get_dmabuf_formats(struct kywc_buffer_helper *helper, EGLint **formats)
{
    if (!helper->exts.EXT_image_dma_buf_import_modifiers) {
        static const EGLint fallback_formats[] = {
            DRM_FORMAT_ARGB8888,
            DRM_FORMAT_XRGB8888,
        };
        int num = ARRAY_SIZE(fallback_formats);

        *formats = calloc(num, sizeof(**formats));
        if (!*formats) {
            return -1;
        }

        memcpy(*formats, fallback_formats, num * sizeof(**formats));
        return num;
    }

    EGLint num;
    if (!eglQueryDmaBufFormatsEXT(helper->display, 0, NULL, &num)) {
        fprintf(stderr, "Failed to query number of dmabuf formats: %#x\n", eglGetError());
        return -1;
    }

    *formats = calloc(num, sizeof(**formats));
    if (*formats == NULL) {
        return -1;
    }

    if (!eglQueryDmaBufFormatsEXT(helper->display, num, *formats, &num)) {
        fprintf(stderr, "Failed to query dmabuf format: %#x\n", eglGetError());
        free(*formats);
        return -1;
    }

    return num;
}

static int egl_get_dmabuf_modifiers(struct kywc_buffer_helper *helper, EGLint format,
                                    uint64_t **modifiers, EGLBoolean **external_only)
{
    *modifiers = NULL;
    *external_only = NULL;

    if (!helper->exts.EXT_image_dma_buf_import_modifiers) {
        return 0;
    }

    EGLint num;
    if (!eglQueryDmaBufModifiersEXT(helper->display, format, 0, NULL, NULL, &num)) {
        fprintf(stderr, "Failed to query dmabuf number of modifiers: %#x\n", eglGetError());
        return -1;
    }
    if (num == 0) {
        return 0;
    }

    *modifiers = calloc(num, sizeof(**modifiers));
    if (*modifiers == NULL) {
        return -1;
    }
    *external_only = calloc(num, sizeof(**external_only));
    if (*external_only == NULL) {
        free(*modifiers);
        *modifiers = NULL;
        return -1;
    }

    /* GL_TEXTURE_EXTERNAL_OES texture target when GL_OES_EGL_image_external */
    if (!eglQueryDmaBufModifiersEXT(helper->display, format, num, *modifiers, *external_only,
                                    &num)) {
        fprintf(stderr, "Failed to query dmabuf modifiers: %#x", eglGetError());
        free(*modifiers);
        free(*external_only);
        return -1;
    }

    return num;
}

struct buffer_format *kywc_buffer_helper_get_format(struct kywc_buffer_helper *helper,
                                                    uint32_t format)
{
    for (size_t i = 0; i < helper->format_set.len; ++i) {
        if (helper->format_set.formats[i].format == format) {
            return &helper->format_set.formats[i];
        }
    }
    return NULL;
}

static bool buffer_format_add_modifier(struct buffer_format *format, uint64_t modifier)
{
    for (size_t i = 0; i < format->len; ++i) {
        if (format->modifiers[i] == modifier) {
            return true;
        }
    }

    if (format->len == format->capacity) {
        size_t capacity = format->capacity ? format->capacity * 2 : 4;
        uint64_t *new_modifiers = realloc(format->modifiers, sizeof(*format->modifiers) * capacity);
        if (!new_modifiers) {
            return false;
        }
        format->capacity = capacity;
        format->modifiers = new_modifiers;
    }

    format->modifiers[format->len++] = modifier;
    return true;
}

static bool helper_add_buffer_format(struct kywc_buffer_helper *helper, uint32_t format,
                                     uint64_t modifier)
{
    struct buffer_format *existing = kywc_buffer_helper_get_format(helper, format);
    if (existing) {
        return buffer_format_add_modifier(existing, modifier);
    }

    struct buffer_format fmt = { .format = format };
    if (!buffer_format_add_modifier(&fmt, modifier)) {
        free(fmt.modifiers);
        return false;
    }

    if (helper->format_set.len == helper->format_set.capacity) {
        size_t capacity = helper->format_set.capacity ? helper->format_set.capacity * 2 : 4;
        struct buffer_format *fmts = realloc(helper->format_set.formats, sizeof(*fmts) * capacity);
        if (!fmts) {
            return false;
        }
        helper->format_set.capacity = capacity;
        helper->format_set.formats = fmts;
    }

    helper->format_set.formats[helper->format_set.len++] = fmt;
    return true;
}

static void dmabuf_formats_init(struct kywc_buffer_helper *helper)
{
    EGLint *formats;
    int formats_len = egl_get_dmabuf_formats(helper, &formats);
    if (formats_len < 0) {
        return;
    }

    bool has_modifiers = false;
    for (int i = 0; i < formats_len; i++) {
        EGLint fmt = formats[i];

        uint64_t *modifiers = NULL;
        EGLBoolean *external_only = NULL;
        int modifiers_len = 0;
        modifiers_len = egl_get_dmabuf_modifiers(helper, fmt, &modifiers, &external_only);
        if (modifiers_len < 0) {
            continue;
        }

        has_modifiers = has_modifiers || modifiers_len > 0;

        bool all_external_only = true;
        for (int j = 0; j < modifiers_len; j++) {
            if (!external_only[j]) {
                helper_add_buffer_format(helper, fmt, modifiers[j]);
                all_external_only = false;
            }
        }

        if (modifiers_len == 0 || !all_external_only) {
            helper_add_buffer_format(helper, fmt, DRM_FORMAT_MOD_INVALID);
        }
        if (modifiers_len == 0) {
            helper_add_buffer_format(helper, fmt, DRM_FORMAT_MOD_LINEAR);
        }

        free(modifiers);
        free(external_only);
    }
    free(formats);
}

static void dmabuf_helper_init(struct kywc_buffer_helper *helper)
{
    const char *client_exts_str = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (client_exts_str == NULL) {
        if (eglGetError() == EGL_BAD_DISPLAY) {
            fprintf(stderr, "EGL_EXT_client_extensions not supported\n");
        } else {
            fprintf(stderr, "Failed to query EGL client extensions: %#x\n", eglGetError());
        }
        return;
    }

    debug("Supported EGL client extensions: %s", client_exts_str);

    if (!epoxy_extension_in_string(client_exts_str, "EGL_EXT_platform_base")) {
        fprintf(stderr, "EGL_EXT_platform_base not supported\n");
        return;
    }

    helper->exts.EXT_platform_wayland =
        epoxy_extension_in_string(client_exts_str, "EGL_EXT_platform_wayland") ||
        epoxy_extension_in_string(client_exts_str, "EGL_KHR_platform_wayland");

    if (!helper->exts.EXT_platform_wayland) {
        fprintf(stderr, "EXT_platform_wayland not supported\n");
        return;
    }

    EGLDisplay egl_display = EGL_NO_DISPLAY;
    egl_display = egl_init_with_wayland(helper);
    if (egl_display == EGL_NO_DISPLAY) {
        return;
    }

    if (eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE) {
        fprintf(stderr, "Failed to bind to the OpenGL API\n");
        return;
    }

    /* using opengles 2 */
    const EGLint attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    EGLContext egl_context =
        eglCreateContext(egl_display, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, attribs);
    if (egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "Failed to create EGL context\n");
        return;
    }

    if (!eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl_context)) {
        fprintf(stderr, "eglMakeCurrent failed\n");
        eglDestroyContext(egl_display, egl_context);
        return;
    }

    const char *exts_str = (const char *)glGetString(GL_EXTENSIONS);
    if (exts_str == NULL) {
        fprintf(stderr, "Failed to get GL_EXTENSIONS\n");
        eglMakeCurrent(helper->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(egl_display, egl_context);
        return;
    }

    debug("Using %s", glGetString(GL_VERSION));
    debug("GL vendor: %s", glGetString(GL_VENDOR));
    debug("GL renderer: %s", glGetString(GL_RENDERER));
    debug("Supported GL extensions: %s", exts_str);

    helper->display = egl_display;
    helper->context = egl_context;

    if (helper->exts.EXT_image_dma_buf_import) {
        dmabuf_formats_init(helper);
#if DEBUG
        debug("Supported DMA-BUF formats:");
        for (size_t i = 0; i < helper->format_set.len; ++i) {
            struct buffer_format *fmt = &helper->format_set.formats[i];
            debug("format (0x%08" PRIX32 ")", fmt->format);
            for (size_t j = 0; j < fmt->len; ++j) {
                debug("    (0x%016" PRIX64 ")", fmt->modifiers[j]);
            }
        }
#endif
    }

    eglMakeCurrent(helper->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

struct kywc_buffer_helper *kywc_buffer_helper_create(kywc_context *ctx)
{
    struct kywc_buffer_helper *helper = calloc(1, sizeof(*helper));
    if (helper == NULL) {
        fprintf(stderr, "Allocation failed\n");
        return NULL;
    }

    helper->ctx = ctx;
    helper->wl_display = kywc_context_get_display(ctx);
    wl_list_init(&helper->buffers);

    dmabuf_helper_init(helper);

    return helper;
}

void kywc_buffer_helper_destroy_buffers(struct kywc_buffer_helper *helper)
{
    if (helper == NULL) {
        return;
    }

    struct kywc_buffer *buffer, *tmp;
    wl_list_for_each_safe(buffer, tmp, &helper->buffers, link) {
        kywc_buffer_destroy(buffer);
    }
}

void kywc_buffer_helper_destroy(struct kywc_buffer_helper *helper)
{
    if (helper == NULL) {
        return;
    }

    kywc_buffer_helper_destroy_buffers(helper);

    for (size_t i = 0; i < helper->format_set.len; i++) {
        free(helper->format_set.formats[i].modifiers);
    }
    free(helper->format_set.formats);

    if (helper->display) {
        eglMakeCurrent(helper->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(helper->display, helper->context);
        eglTerminate(helper->display);
        eglReleaseThread();
    }

    free(helper);
}

#define ADD_ATTRIB(name, value)                                                                    \
    do {                                                                                           \
        attribs[num_attribs++] = (name);                                                           \
        attribs[num_attribs++] = (value);                                                          \
        attribs[num_attribs] = EGL_NONE;                                                           \
    } while (0)

static bool kywc_buffer_import_dmabuf(struct kywc_buffer *kywc_buffer,
                                      const struct kywc_thumbnail_buffer *buffer, bool can_reuse)
{
    struct kywc_buffer_helper *helper = kywc_buffer->helper;

    if (!helper->exts.EXT_image_dma_buf_import) {
        fprintf(stderr, "dma_buf_import is not support\n");
        return false;
    }

    if (buffer->modifier != DRM_FORMAT_MOD_INVALID && buffer->modifier != DRM_FORMAT_MOD_LINEAR &&
        !helper->exts.EXT_image_dma_buf_import_modifiers) {
        fprintf(stderr, "buffer has a modifier when dma_buf_import_modifiers is not supported\n");
        return false;
    }

    if (kywc_buffer->image) {
        if (can_reuse) {
            return true;
        }
        /* release prev buffer stuff */
        eglMakeCurrent(helper->display, EGL_NO_SURFACE, EGL_NO_SURFACE, helper->context);
        glDeleteFramebuffers(1, &kywc_buffer->fbo);
        glDeleteTextures(1, &kywc_buffer->tex);
        eglDestroyImageKHR(helper->display, kywc_buffer->image);
        kywc_buffer->image = EGL_NO_IMAGE_KHR;
        kywc_buffer->tex = 0;
        kywc_buffer->fbo = 0;
    }

    EGLint attribs[50] = { EGL_NONE };
    int num_attribs = 0;

    ADD_ATTRIB(EGL_WIDTH, buffer->width);
    ADD_ATTRIB(EGL_HEIGHT, buffer->height);
    ADD_ATTRIB(EGL_LINUX_DRM_FOURCC_EXT, buffer->format);

    struct {
        EGLint fd, offset, pitch, mod_lo, mod_hi;
    } attr_names[4] = {
        { EGL_DMA_BUF_PLANE0_FD_EXT, EGL_DMA_BUF_PLANE0_OFFSET_EXT, EGL_DMA_BUF_PLANE0_PITCH_EXT,
          EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT },
        { EGL_DMA_BUF_PLANE1_FD_EXT, EGL_DMA_BUF_PLANE1_OFFSET_EXT, EGL_DMA_BUF_PLANE1_PITCH_EXT,
          EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT },
        { EGL_DMA_BUF_PLANE2_FD_EXT, EGL_DMA_BUF_PLANE2_OFFSET_EXT, EGL_DMA_BUF_PLANE2_PITCH_EXT,
          EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT },
        { EGL_DMA_BUF_PLANE3_FD_EXT, EGL_DMA_BUF_PLANE3_OFFSET_EXT, EGL_DMA_BUF_PLANE3_PITCH_EXT,
          EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT, EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT }
    };

    for (uint32_t i = 0; i < buffer->n_planes; i++) {
        ADD_ATTRIB(attr_names[i].fd, buffer->planes[i].fd);
        ADD_ATTRIB(attr_names[i].offset, buffer->planes[i].offset);
        ADD_ATTRIB(attr_names[i].pitch, buffer->planes[i].stride);
        if (buffer->modifier != DRM_FORMAT_MOD_INVALID) {
            ADD_ATTRIB(attr_names[i].mod_hi, buffer->modifier >> 32);
            ADD_ATTRIB(attr_names[i].mod_lo, buffer->modifier & 0xFFFFFFFF);
        }
    }

    ADD_ATTRIB(EGL_IMAGE_PRESERVED_KHR, EGL_TRUE);

    if (kywc_buffer->image == EGL_NO_IMAGE_KHR) {
        eglMakeCurrent(helper->display, EGL_NO_SURFACE, EGL_NO_SURFACE, helper->context);
    }

    kywc_buffer->image =
        eglCreateImageKHR(helper->display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs);
    if (kywc_buffer->image == EGL_NO_IMAGE_KHR) {
        fprintf(stderr, "eglCreateImageKHR failed\n");
        eglMakeCurrent(helper->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        return false;
    }

    glGenTextures(1, &kywc_buffer->tex);
    glBindTexture(GL_TEXTURE_2D, kywc_buffer->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, kywc_buffer->image);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &kywc_buffer->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, kywc_buffer->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, kywc_buffer->tex,
                           0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    eglMakeCurrent(helper->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    return true;
}

static bool kywc_buffer_export_dmabuf(struct kywc_buffer *buffer, int32_t x, int32_t y,
                                      uint32_t width, uint32_t height, void *data)
{
    struct kywc_buffer_helper *helper = buffer->helper;

    if (!eglMakeCurrent(helper->display, EGL_NO_SURFACE, EGL_NO_SURFACE, helper->context)) {
        fprintf(stderr, "eglMakeCurrent failed\n");
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(x, y, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    eglMakeCurrent(helper->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    return true;
}

static bool kywc_buffer_import_memfd(struct kywc_buffer *kywc_buffer,
                                     const struct kywc_thumbnail_buffer *buffer, bool can_reuse)
{
    if (kywc_buffer->ptr) {
        if (can_reuse) {
            return true;
        }
        munmap(kywc_buffer->ptr, kywc_buffer->size);
        kywc_buffer->ptr = NULL;
    }

    kywc_buffer->size = buffer->height * buffer->stride + buffer->offset;
    kywc_buffer->ptr = mmap(0, kywc_buffer->size, PROT_READ, MAP_SHARED, buffer->fd, 0);
    if (kywc_buffer->ptr == MAP_FAILED) {
        kywc_buffer->ptr = NULL;
        return false;
    }

    return true;
}

static bool kywc_buffer_export_memfd(struct kywc_buffer *buffer, int32_t x, int32_t y,
                                     uint32_t width, uint32_t height, void *data)
{
    char *src = (char *)buffer->ptr + buffer->buffer.offset;
    size_t line = width * 4;

    /* check if can use one memcpy */
    if (buffer->buffer.stride == line) {
        memcpy(data, src, buffer->size - buffer->buffer.offset);
        return true;
    }

    char *dst = data;
    for (uint32_t i = 0; i < height; i++) {
        memcpy(dst, src + (i + y) * buffer->buffer.stride + x * 4, line);
        dst += line;
    }

    return true;
}

static struct kywc_buffer *helper_get_buffer(struct kywc_buffer_helper *helper, void *data)
{
    struct kywc_buffer *buffer;
    wl_list_for_each(buffer, &helper->buffers, link) {
        if (buffer->user_data == data) {
            return buffer;
        }
    }
    return NULL;
}

struct kywc_buffer *kywc_buffer_helper_import_thumbnail(struct kywc_buffer_helper *helper,
                                                        void *data,
                                                        const struct kywc_thumbnail_buffer *buffer)
{
    if (helper == NULL || buffer == NULL) {
        return NULL;
    }

    struct kywc_buffer *kywc_buffer = helper_get_buffer(helper, data);
    if (!kywc_buffer) {
        kywc_buffer = calloc(1, sizeof(*kywc_buffer));
        if (kywc_buffer == NULL) {
            fprintf(stderr, "Allocation failed\n");
            return NULL;
        }
        kywc_buffer->helper = helper;
        kywc_buffer->user_data = data;
        wl_list_insert(&helper->buffers, &kywc_buffer->link);
    }

    bool success = false;
    bool can_reuse = buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_REUSED;
    /* try egl dambuf import if support */
    if (helper->display && (buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_DMABUF)) {
        success = kywc_buffer_import_dmabuf(kywc_buffer, buffer, can_reuse);
    }
    /* fallback to mmap */
    if (!success) {
        success = kywc_buffer_import_memfd(kywc_buffer, buffer, can_reuse);
    }

    if (!success) {
        /* a newly created buffer is already linked into helper->buffers */
        kywc_buffer_destroy(kywc_buffer);
        return NULL;
    }

    kywc_buffer->buffer = *buffer;
    return kywc_buffer;
}

void kywc_buffer_destroy(struct kywc_buffer *buffer)
{
    if (buffer == NULL) {
        return;
    }

    wl_list_remove(&buffer->link);

    if (buffer->image) {
        struct kywc_buffer_helper *helper = buffer->helper;
        eglMakeCurrent(helper->display, EGL_NO_SURFACE, EGL_NO_SURFACE, helper->context);
        glDeleteFramebuffers(1, &buffer->fbo);
        glDeleteTextures(1, &buffer->tex);
        eglDestroyImageKHR(helper->display, buffer->image);
        eglMakeCurrent(helper->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    if (buffer->ptr) {
        munmap(buffer->ptr, buffer->size);
    }

    free(buffer);
}

typedef struct __attribute__((__packed__)) {
    uint16_t type;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} bmp_file_header;

typedef struct __attribute__((__packed__)) {
    uint32_t dib_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t img_size;
    uint32_t hres;
    uint32_t vres;
    uint32_t clr_palette;
    uint32_t clr_important;
} bmp_info_header;

static bool do_encode_bmp(FILE *file, int width, int height, size_t stride, uint8_t *data)
{
    bmp_info_header info_header = {
        .dib_size = sizeof(info_header),
        .width = width,
        .height = -height,
        .planes = 1,
        .bpp = 32,
        .img_size = stride * height,
    };

    bmp_file_header file_header = {
        .type = 'B' | ('M' << 8),
        .file_size = sizeof(file_header) + sizeof(info_header) + info_header.img_size,
        .offset = sizeof(file_header) + sizeof(info_header),
    };

    fwrite(&file_header, sizeof(file_header), 1, file);
    fwrite(&info_header, sizeof(info_header), 1, file);
    fwrite(data, stride, height, file);

    return true;
}

static bool do_encode_png(FILE *file, int width, int height, size_t stride, uint8_t *data)
{
    png_struct *png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        return false;
    }
    png_info *info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, NULL);
        return false;
    }

    png_init_io(png, file);
    // png_set_compression_level(png, 6);
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_alpha_mode(png, PNG_ALPHA_PREMULTIPLIED, PNG_GAMMA_LINEAR);
    png_set_bgr(png);
    png_write_info(png, info);

    png_bytep *row_ptrs = malloc(height * sizeof(*row_ptrs));
    if (!row_ptrs) {
        png_destroy_write_struct(&png, &info);
        return false;
    }

    for (int i = 0; i < height; ++i) {
        row_ptrs[i] = data + i * stride;
    }

    png_write_image(png, row_ptrs);
    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    free(row_ptrs);

    return true;
}

bool kywc_buffer_write_to_file(struct kywc_buffer *buffer, int32_t x, int32_t y, uint32_t width,
                               uint32_t height, const char *path)
{
    if (x < 0 || y < 0 || (uint32_t)x >= buffer->buffer.width ||
        (uint32_t)y >= buffer->buffer.height) {
        fprintf(stderr, "position or size mismatch\n");
        return false;
    }

    if (x + width > buffer->buffer.width) {
        width = buffer->buffer.width;
    }
    if (y + height > buffer->buffer.height) {
        height = buffer->buffer.height;
    }

    size_t size = width * height * 4;
    unsigned char *data = malloc(size);
    if (!data) {
        return false;
    }

    bool success = false;
    if (buffer->image) {
        success = kywc_buffer_export_dmabuf(buffer, x, y, width, height, data);
    } else if (buffer->ptr) {
        success = kywc_buffer_export_memfd(buffer, x, y, width, height, data);
    }
    if (!success) {
        free(data);
        return false;
    }

    FILE *fp = fopen(path, "w+");
    if (!fp) {
        free(data);
        fprintf(stderr, "failed to open cache file %s\n", path);
        return false;
    }

    size_t len = strlen(path);
    const char *suffix = path + len - 3;
    if (len > 3 && strncmp(suffix, "bmp", 3) == 0) {
        success = do_encode_bmp(fp, width, height, width * 4, data);
    } else if (len > 3 && strncmp(suffix, "png", 3) == 0) {
        success = do_encode_png(fp, width, height, width * 4, data);
    } else {
        fwrite(data, 1, size, fp);
    }

    fflush(fp);
    fclose(fp);
    free(data);

    return success;
}

uint32_t kywc_buffer_get_pixel(struct kywc_buffer *buffer, int32_t x, int32_t y)
{
    if (x < 0 || y < 0 || (uint32_t)x >= buffer->buffer.width ||
        (uint32_t)y >= buffer->buffer.height) {
        fprintf(stderr, "position or size mismatch\n");
        return 0;
    }

    uint32_t pixel = 0;
    if (buffer->image) {
        kywc_buffer_export_dmabuf(buffer, x, y, 1, 1, &pixel);
    } else if (buffer->ptr) {
        kywc_buffer_export_memfd(buffer, x, y, 1, 1, &pixel);
    }

    return pixel;
}

void kywc_buffer_read_pixels(struct kywc_buffer *buffer, int32_t x, int32_t y, uint32_t width,
                             uint32_t height, void *pixels)
{
    if (x < 0 || y < 0 || (uint32_t)x >= buffer->buffer.width ||
        (uint32_t)y >= buffer->buffer.height) {
        fprintf(stderr, "position or size mismatch\n");
        return;
    }

    if (buffer->image) {
        kywc_buffer_export_dmabuf(buffer, x, y, width, height, pixels);
    } else if (buffer->ptr) {
        kywc_buffer_export_memfd(buffer, x, y, width, height, pixels);
    }
}
