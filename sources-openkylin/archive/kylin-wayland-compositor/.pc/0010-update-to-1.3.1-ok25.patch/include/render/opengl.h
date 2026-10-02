// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _RENDER_OPENGL_H_
#define _RENDER_OPENGL_H_

#include <time.h>

#include <epoxy/gl.h>

#include <wlr/render/interface.h>
#include <wlr/render/wlr_renderer.h>

#include "egl.h"
#include "pass.h"

struct kywc_fbox;

struct opengl_tex_mix_shader {
    GLuint program;

    GLint uv2ndc;
    GLint uv2texcoord;
    GLint uv2texcoord2;
    GLint uvpos_attrib;

    GLint tex_a;
    GLint tex_b;
    GLint alpha;
    GLint blend_factor;
};

struct opengl_surface_shader {
    GLuint program;
    GLint in_position;
    GLint size;

    GLint texture;
    GLint contrast;
    GLint whitepoint;
    GLint color_matrix;
    GLint brightness;
    GLint pixel_offset_x;
};

struct opengl_linear_gradient_shader {
    GLuint program;
    GLint in_uv;
    GLint uv2ndc;
    GLint uv_rotation;

    GLint start;
    GLint end;
    GLint color_stop0_offset;
    GLint color_stop0_color;
    GLint color_stop1_offset;
    GLint color_stop1_color;
    GLint stroke_width;
    GLint anti_aliasing;
    GLint aspect;
    GLint round_corner_radius;
};

struct opengl_rect_ex_shader {
    GLuint program;

    GLint uv_rotation;
    GLint uv2ndc;
    GLint uv_attrib;

    GLint color;
    GLint stroke_width;
    GLint aspect;
    GLint anti_aliasing;
    GLint round_corner_radius;
};

struct opengl_tex_ex_shader {
    GLuint program;

    GLint uv_rotation;
    GLint uv2texcoord;
    GLint uv2ndc;
    GLint uv_attrib;

    GLint tex;
    GLint alpha;
    GLint force_opaque;
    GLint aspect;
    GLint anti_aliasing;
    GLint round_corner_radius;
};

struct opengl_renderer {
    struct wlr_renderer wlr_renderer;

    struct egl *egl;
    int drm_fd;

    struct {
        bool EXT_read_format_bgra;
        bool KHR_debug;
        bool OES_egl_image_external;
        bool OES_egl_image;
        bool EXT_texture_type_2_10_10_10_REV;
        bool OES_texture_half_float_linear;
        bool EXT_texture_norm16;
        bool EXT_disjoint_timer_query;
        bool KHR_robustness;
    } exts;

    struct {
        struct opengl_rect_ex_shader quad;
        struct opengl_tex_ex_shader tex_rgba;
        struct opengl_tex_ex_shader tex_rgbx;
        struct opengl_tex_ex_shader tex_ext;
        // round corner clip shader
        struct opengl_rect_ex_shader quad_ex;
        struct opengl_tex_ex_shader tex_rgba_ex;
        struct opengl_tex_ex_shader tex_rgbx_ex;
        struct opengl_tex_ex_shader tex_ext_ex;
        // blit surface
        struct opengl_surface_shader surface_tex;
        struct opengl_surface_shader surface_yuv_tex;
        // mix two textures
        struct opengl_tex_mix_shader tex_mix;
        struct opengl_rect_ex_shader quad_stroke;
        struct opengl_linear_gradient_shader linear_gradient;
        struct opengl_linear_gradient_shader linear_gradient_stroke;
    } shaders;

    struct wl_list buffers;  // opengl_buffer.link
    struct wl_list textures; // opengl_texture.link

    struct opengl_buffer *current_buffer;
};

struct opengl_render_timer {
    struct wlr_render_timer base;
    struct opengl_renderer *renderer;
    struct timespec cpu_start;
    struct timespec cpu_end;
    GLuint id;
    GLint64 gl_cpu_end;
};

struct opengl_buffer {
    struct wlr_buffer *buffer;
    struct opengl_renderer *renderer;
    struct wl_list link; // wlr_gles2_renderer.buffers

    EGLImageKHR image;
    GLuint rbo;
    GLuint fbo;

    bool has_alpha;

    struct wlr_addon addon;
};

struct opengl_texture {
    struct wlr_texture wlr_texture;
    struct opengl_renderer *renderer;
    struct wl_list link; // opengl_renderer.textures

    GLenum target;
    GLuint fbo; // maybe zero
    GLuint tex;

    EGLImageKHR image;

    bool has_alpha;

    uint32_t drm_format;       // for mutable textures only, used to interpret upload data
    struct wlr_buffer *buffer; // for DMA-BUF and WL-BUF imports only
    struct wlr_addon buffer_addon;
};

/* saved opengl render pass state for store/restore operations */
struct opengl_render_pass_state {
    GLint fbo;
    GLint viewport[4]; // x, y, width, height
    /* blend func */
    GLint blend_src_rgb;
    GLint blend_dst_rgb;
    /* scissor test */
    bool scissor_test_enabled;
};

struct opengl_render_pass {
    struct wlr_render_pass base;
    const struct render_pass_impl *impl;
    struct opengl_renderer *renderer;
    struct opengl_buffer *buffer;   // or texture, mutually exclusive
    struct opengl_texture *texture; // or buffer, mutually exclusive
    struct egl_context prev_ctx;
    struct opengl_render_timer *timer;
    int target_width, target_height;

    /* for opengl_render_pass_store/restore */
    struct opengl_render_pass_state saved_state;
};

struct opengl_texture_attribs {
    GLenum target; /* either GL_TEXTURE_2D or GL_TEXTURE_EXTERNAL_OES */
    GLuint tex;

    bool has_alpha;
};

struct wlr_renderer *opengl_renderer_create_with_drm_fd(int drm_fd);

bool wlr_renderer_is_opengl(struct wlr_renderer *wlr_renderer);

struct opengl_renderer *opengl_renderer_from_wlr_renderer(struct wlr_renderer *wlr_renderer);

struct egl *opengl_renderer_get_egl(struct wlr_renderer *wlr_renderer);

void opengl_pop_debug(struct opengl_renderer *renderer);

void opengl_push_debug_(struct opengl_renderer *renderer, const char *file, const char *func);

#define opengl_push_debug(renderer) opengl_push_debug_(renderer, __FILE__, __func__)

struct opengl_render_pass *opengl_begin_buffer_pass(struct opengl_buffer *buffer,
                                                    struct egl_context *prev_ctx,
                                                    struct opengl_render_timer *timer);

bool wlr_render_pass_is_opengl(struct wlr_render_pass *render_pass);

struct opengl_render_pass *
opengl_render_pass_from_wlr_render_pass(struct wlr_render_pass *wlr_pass);

struct wlr_texture *opengl_texture_from_buffer(struct wlr_renderer *wlr_renderer,
                                               struct wlr_buffer *buffer);

bool wayland_buffer_create(struct wl_display *wl_display, struct wlr_renderer *wlr_renderer);

bool wayland_buffer_is_opaque(struct wlr_buffer *buffer);

bool wlr_buffer_is_wayland_buffer(struct wlr_buffer *buffer);

struct wlr_texture *wlr_texture_from_wayland_buffer(struct opengl_renderer *renderer,
                                                    struct wlr_buffer *buffer);

struct opengl_texture *opengl_texture_create(struct opengl_renderer *renderer, uint32_t width,
                                             uint32_t height);

bool opengl_texture_invalidate(struct opengl_texture *texture);

void opengl_texture_destroy(struct opengl_texture *texture);

struct opengl_texture *opengl_texture_from_wlr_texture(struct wlr_texture *wlr_texture);

bool wlr_texture_is_opengl(struct wlr_texture *wlr_texture);

void opengl_texture_get_attribs(struct wlr_texture *texture,
                                struct opengl_texture_attribs *attribs);

void opengl_render_pass_add_texture(struct wlr_render_pass *wlr_pass,
                                    const struct render_texture_options *options);

void opengl_render_pass_add_rect(struct wlr_render_pass *wlr_pass,
                                 const struct render_rect_options *options);

GLuint opengl_create_program(struct opengl_renderer *renderer, const GLchar *vert_src,
                             const GLchar *frag_src);

struct opengl_texture *opengl_texture_create_with_fbo(struct opengl_renderer *renderer,
                                                      uint32_t width, uint32_t height,
                                                      GLenum format, GLint filter);

/* the opengl texture must create with fbo */
struct opengl_render_pass *opengl_begin_texture_pass(struct opengl_texture *texture,
                                                     struct opengl_render_timer *timer);

GLuint opengl_render_pass_get_fbo(struct opengl_render_pass *pass);

void opengl_render_pass_save(struct opengl_render_pass *pass);

void opengl_render_pass_restore(struct opengl_render_pass *pass);

#ifdef TRACY_ENABLE
#include <tracy/TracyC.h>

void profile_gl_create(struct wlr_renderer *renderer);
void profile_gl_destroy(void);
void profile_gl_begin(const struct ___tracy_source_location_data *data);
void profile_gl_end(void);
void profile_gl_collect(void);

#endif /* TRACY_ENABLE */

#endif /* _RENDER_OPENGL_H_ */
