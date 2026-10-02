// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdio.h>

#include "wlu_private.h"

uint32_t wlu_format_to_shm(enum wlu_format format)
{
    switch (format) {
    case WLU_FORMAT_XRGB8888:
        return WL_SHM_FORMAT_XRGB8888;
    case WLU_FORMAT_ARGB8888:
        return WL_SHM_FORMAT_ARGB8888;
    case WLU_FORMAT_BGRX8888:
        return WL_SHM_FORMAT_BGRX8888;
    case WLU_FORMAT_BGRA8888:
        return WL_SHM_FORMAT_BGRA8888;
    default:
        assert(0);
        return 0;
    }
}

enum wlu_format wlu_format_from_shm(uint32_t format)
{
    switch (format) {
    case WL_SHM_FORMAT_ARGB8888:
        return WLU_FORMAT_ARGB8888;
    case WL_SHM_FORMAT_XRGB8888:
        return WLU_FORMAT_XRGB8888;
    case WL_SHM_FORMAT_BGRX8888:
        return WLU_FORMAT_BGRX8888;
    case WL_SHM_FORMAT_BGRA8888:
        return WLU_FORMAT_BGRA8888;
    // 合成器发过来shm格式用了DRM fourcc，需要兼容
    case DRM_FORMAT_ARGB8888:
        return WLU_FORMAT_ARGB8888;
    case DRM_FORMAT_XRGB8888:
        return WLU_FORMAT_XRGB8888;
    default:
        assert(0);
        return WLU_FORMAT_XRGB8888;
    }
}

uint32_t wlu_format_to_drm(enum wlu_format format)
{
    switch (format) {
    case WLU_FORMAT_XRGB8888:
        return DRM_FORMAT_XRGB8888;
    case WLU_FORMAT_ARGB8888:
        return DRM_FORMAT_ARGB8888;
    case WLU_FORMAT_BGRX8888:
        return DRM_FORMAT_BGRX8888;
    case WLU_FORMAT_BGRA8888:
        return DRM_FORMAT_BGRA8888;
    default:
        assert(0);
        return 0;
    }
}

enum wlu_format wlu_format_from_drm(uint32_t format)
{
    switch (format) {
    case DRM_FORMAT_XRGB8888:
        return WLU_FORMAT_XRGB8888;
    case DRM_FORMAT_ARGB8888:
        return WLU_FORMAT_ARGB8888;
    case DRM_FORMAT_BGRX8888:
        return WLU_FORMAT_BGRX8888;
    case DRM_FORMAT_BGRA8888:
        return WLU_FORMAT_BGRA8888;
    default:
        assert(0);
        return WLU_FORMAT_XRGB8888;
    }
}

pixman_format_code_t wlu_format_to_pixman(enum wlu_format format)
{
    switch (format) {
    case WLU_FORMAT_XRGB8888:
        return PIXMAN_x8r8g8b8;
    case WLU_FORMAT_ARGB8888:
        return PIXMAN_a8r8g8b8;
    case WLU_FORMAT_BGRX8888:
        return PIXMAN_b8g8r8x8;
    case WLU_FORMAT_BGRA8888:
        return PIXMAN_b8g8r8a8;
    default:
        assert(0);
        return PIXMAN_x8r8g8b8;
    }
}

static GLuint create_program(const char *vert_src, const char *frag_src)
{
    int success = 0;

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vert_src, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        int log_length = 0;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
        char *info_log = malloc(log_length);
        glGetShaderInfoLog(vertex_shader, log_length, NULL, info_log);
        fprintf(stderr, "vert glsl compile error:\n%s", info_log);
        free(info_log);
        return 0;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &frag_src, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        int log_length = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
        char *info_log = malloc(log_length);
        glGetShaderInfoLog(fragment_shader, log_length, NULL, info_log);
        fprintf(stderr, "frag glsl compile error:\n%s", info_log);
        free(info_log);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        int log_length = 0;
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &log_length);
        char *info_log = malloc(log_length);
        glGetShaderInfoLog(program, log_length, NULL, info_log);
        fprintf(stderr, "shader link error:\n%s", info_log);
        free(info_log);
        return 0;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

static const char *SHADER_TEX_VERT = "attribute vec2 a_pos;\n"
                                     "attribute vec2 a_uv;\n"
                                     "varying   vec2 v_uv;\n"
                                     "uniform   vec2 u_scale;\n"
                                     "void main() {\n"
                                     "    v_uv        = a_uv;\n"
                                     "    vec2 ndc    = a_pos * u_scale - 1.0;\n"
                                     "    gl_Position = vec4(ndc.x, ndc.y, 0.0, 1.0);\n"
                                     "}\n";

static const char *SHADER_TEX_FRAG = "#ifdef GL_ES\n"
                                     "precision highp float;\n"
                                     "#endif\n"
                                     "varying vec2      v_uv;\n"
                                     "uniform sampler2D u_tex;\n"
                                     "void main() {\n"
                                     "    gl_FragColor = texture2D(u_tex, v_uv);\n"
                                     "}\n";

static struct {
    GLuint program;
    GLint loc_pos;
    GLint loc_uv;
    GLint loc_scale;
    GLint loc_tex;
} g_texture_shader = { 0 };

bool wlu_draw_init(struct wlu_window *window)
{
    if (!wlu_window_supports_gl(window)) {
        return true;
    }

    g_texture_shader.program = create_program(SHADER_TEX_VERT, SHADER_TEX_FRAG);
    if (g_texture_shader.program == 0) {
        return false;
    }

    glUseProgram(g_texture_shader.program);
    g_texture_shader.loc_pos = glGetAttribLocation(g_texture_shader.program, "a_pos");
    g_texture_shader.loc_uv = glGetAttribLocation(g_texture_shader.program, "a_uv");
    g_texture_shader.loc_scale = glGetUniformLocation(g_texture_shader.program, "u_scale");
    g_texture_shader.loc_tex = glGetUniformLocation(g_texture_shader.program, "u_tex");
    glUseProgram(0);

    return true;
}

static void draw_texture_gl(struct wlu_draw_texture_options *options)
{
    if (options->texture->window != options->target->window ||
        !wlu_window_supports_gl(options->target->window) || g_texture_shader.program == 0) {
        return;
    }

    float src_x = options->src_width == 0 ? 0.0f : options->src_x;
    float src_y = options->src_height == 0 ? 0.0f : options->src_y;
    float src_w = options->src_width == 0 ? options->texture->width : options->src_width;
    float src_h = options->src_height == 0 ? options->texture->height : options->src_height;

    float dst_x = options->dst_width == 0 ? 0.0f : options->dst_x;
    float dst_y = options->dst_height == 0 ? 0.0f : options->dst_y;
    float dst_w = options->dst_width == 0 ? options->target->width : options->dst_width;
    float dst_h = options->dst_height == 0 ? options->target->height : options->dst_height;

    float u0 = src_x / (float)options->texture->width;
    float u1 = (src_x + src_w) / (float)options->texture->width;
    float v0 = src_y / (float)options->texture->height;
    float v1 = (src_y + src_h) / (float)options->texture->height;

    float x0 = dst_x;
    float y0 = dst_y;
    float x1 = dst_x + dst_w;
    float y1 = dst_y + dst_h;

    float verts[6 * 4] = {
        x0, y0, u0, v0, x0, y1, u0, v1, x1, y0, u1, v0,
        x1, y0, u1, v0, x0, y1, u0, v1, x1, y1, u1, v1,
    };

    glBindFramebuffer(GL_FRAMEBUFFER, wlu_buffer_get_gl_fbo(options->target));
    glViewport(0, 0, options->target->width, options->target->height);
    glUseProgram(g_texture_shader.program);
    glUniform2f(g_texture_shader.loc_scale, 2.0f / (float)options->target->width,
                2.0f / (float)options->target->height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wlu_buffer_get_gl_texture(options->texture));
    glUniform1i(g_texture_shader.loc_tex, 0);

    GLsizei stride = 4 * sizeof(float);
    glEnableVertexAttribArray(g_texture_shader.loc_pos);
    glVertexAttribPointer(g_texture_shader.loc_pos, 2, GL_FLOAT, GL_FALSE, stride, verts);
    glEnableVertexAttribArray(g_texture_shader.loc_uv);
    glVertexAttribPointer(g_texture_shader.loc_uv, 2, GL_FLOAT, GL_FALSE, stride, verts + 2);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_texture_shader.loc_pos);
    glDisableVertexAttribArray(g_texture_shader.loc_uv);
    glUseProgram(0);
};

static void draw_texture_pixman(struct wlu_draw_texture_options *options)
{
    uint32_t src_x = options->src_width == 0 ? 0 : options->src_x;
    uint32_t src_y = options->src_height == 0 ? 0 : options->src_y;
    uint32_t src_w = options->src_width == 0 ? options->texture->width : options->src_width;
    uint32_t src_h = options->src_height == 0 ? options->texture->height : options->src_height;

    uint32_t dst_x = options->dst_width == 0 ? 0 : options->dst_x;
    uint32_t dst_y = options->dst_height == 0 ? 0 : options->dst_y;
    uint32_t dst_w = options->dst_width == 0 ? options->target->width : options->dst_width;
    uint32_t dst_h = options->dst_height == 0 ? options->target->height : options->dst_height;

    pixman_format_code_t src_fmt = wlu_format_to_pixman(options->texture->format);
    pixman_format_code_t dst_fmt = wlu_format_to_pixman(options->target->format);

    int32_t src_stride =
        options->texture->width *
        options->texture->window->display->shm_formats[options->texture->format].bytes_per_pixel;
    int32_t dst_stride =
        options->target->width *
        options->target->window->display->shm_formats[options->target->format].bytes_per_pixel;

    pixman_image_t *src_image =
        pixman_image_create_bits(src_fmt, options->texture->width, options->texture->height,
                                 wlu_buffer_get_shm_data(options->texture), src_stride);
    if (!src_image) {
        fprintf(stderr, "pixman: failed to create src image\n");
        return;
    }

    pixman_image_t *dst_image =
        pixman_image_create_bits(dst_fmt, options->target->width, options->target->height,
                                 wlu_buffer_get_shm_data(options->target), dst_stride);
    if (!dst_image) {
        fprintf(stderr, "pixman: failed to create dst image\n");
        pixman_image_unref(src_image);
        return;
    }

    // src/dst 尺寸不一致时设置缩放变换
    bool needs_scale = (src_w != dst_w || src_h != dst_h);
    if (needs_scale) {
        pixman_transform_t transform;
        pixman_transform_init_scale(&transform, pixman_double_to_fixed((double)src_w / dst_w),
                                    pixman_double_to_fixed((double)src_h / dst_h));
        pixman_image_set_transform(src_image, &transform);
        pixman_image_set_filter(src_image, PIXMAN_FILTER_BILINEAR, NULL, 0);
    }

    pixman_image_composite32(PIXMAN_OP_SRC, src_image, NULL, dst_image, (int32_t)src_x,
                             (int32_t)src_y,                 // src origin
                             0, 0,                           // mask origin
                             (int32_t)dst_x, (int32_t)dst_y, // dst origin
                             (int32_t)dst_w, (int32_t)dst_h);

    pixman_image_unref(src_image);
    pixman_image_unref(dst_image);
}

void wlu_draw_texture(struct wlu_draw_texture_options *options)
{
    if (!options->texture || !options->target) {
        return;
    }

    if (options->texture->type == WLU_BUFFER_TYPE_DMABUF_GL &&
        options->target->type == WLU_BUFFER_TYPE_DMABUF_GL) {
        draw_texture_gl(options);
    } else if (options->texture->type == WLU_BUFFER_TYPE_SHM &&
               options->target->type == WLU_BUFFER_TYPE_SHM) {
        draw_texture_pixman(options);
    }
}

void wlu_draw_fini(struct wlu_window *window)
{
    if (!wlu_window_supports_gl(window)) {
        return;
    }

    if (g_texture_shader.program > 0) {
        glDeleteProgram(g_texture_shader.program);
    }
}
