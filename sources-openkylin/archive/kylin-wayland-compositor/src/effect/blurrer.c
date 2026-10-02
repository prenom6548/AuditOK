// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <kywc/log.h>

#include "effect/blur.h"
#include "effect_p.h"
#include "pixman.h"
#include "render/opengl.h"
#include "render/profile.h"
#include "util/macros.h"
#include "util/matrix.h"

#include "blur_down_frag.h"
#include "blur_down_vert.h"
#include "blur_up_frag.h"
#include "blur_up_last_frag.h"
#include "blur_up_last_vert.h"
#include "blur_up_vert.h"

#define MAX_QUADS 86 // 4kb

struct blur_shader {
    GLuint prog;
    GLint pos;
    GLint frambuffer_size;
    GLint tex_clamp;
    GLint halfpixel;
    GLint offset;
};

// support alpha blend + round corner
struct blur_up_last_shader {
    struct blur_shader blur;
    GLint dst_box;
    GLint sdf_uv_rotation;
    GLint alpha;
    GLint anti_aliasing;
    GLint aspect;
    GLint round_corner_radius;
};

struct blur_task {
    struct wl_list link;
    struct blur_task_buffer *buffer;
};

struct blur_task_buffer {
    struct wl_list link;
    struct wl_list tasks; // blur_task->link

    uint32_t width, height, iterations;
    struct opengl_texture **textures;
    uint32_t textures_size;
};

struct blurrer {
    struct wl_list buffers; // blur_task_buffer->link

    struct wl_listener server_destroy;

    struct opengl_renderer *renderer;
    struct blur_shader down_shader;
    struct blur_shader up_shader;
    struct blur_up_last_shader up_last_shader;
};

static struct blurrer *manager = NULL;

static struct blur_task_buffer *get_or_create_blur_task_buffer(uint32_t width, uint32_t height,
                                                               uint32_t iterations)
{
    bool find = false;
    struct blur_task_buffer *buffer;
    wl_list_for_each(buffer, &manager->buffers, link) {
        if (buffer->width == width && buffer->height == height) {
            if (buffer->iterations >= iterations) {
                return buffer;
            }

            // match width+height but less iterations just realloc
            find = true;
            break;
        }
    }

    if (!find) {
        buffer = calloc(1, sizeof(*buffer));
        if (buffer == NULL) {
            return NULL;
        }
        wl_list_init(&buffer->tasks);
        wl_list_insert(&manager->buffers, &buffer->link);
        buffer->width = width;
        buffer->height = height;
    }

    uint32_t required_size = iterations;
    uint32_t size = buffer->textures_size;

    void *intermediate_textures =
        realloc(buffer->textures, required_size * sizeof(struct opengl_texture *));
    if (intermediate_textures == NULL) {
        return NULL;
    }
    buffer->textures = intermediate_textures;

    for (uint32_t i = size; i < required_size; i++) {
        uint32_t tex_width = MAX(width / (1 << (i + 1)), 1);
        uint32_t tex_height = MAX(height / (1 << (i + 1)), 1);
        struct opengl_texture *texture = opengl_texture_create_with_fbo(
            manager->renderer, tex_width, tex_height, GL_BGRA_EXT, GL_LINEAR);
        if (texture == NULL) {
            return NULL;
        }
        buffer->textures[i] = texture;
        buffer->textures_size++;
    }
    buffer->iterations = required_size;

    return buffer;
}

static void blur_task_buffer_destroy(struct blur_task_buffer *buffer)
{
    wl_list_remove(&buffer->link);

    struct blur_task *task, *tmp;
    wl_list_for_each_safe(task, tmp, &buffer->tasks, link) {
        task->buffer = NULL;
        wl_list_remove(&task->link);
        wl_list_init(&task->link);
    }

    for (uint32_t i = 0; i < buffer->textures_size; i++) {
        struct opengl_texture *texture = buffer->textures[i];
        opengl_texture_destroy(texture);
    }

    free(buffer->textures);
    free(buffer);
}

static inline void blur_box_draw(const pixman_box32_t *box, GLint pos_location)
{
    glEnableVertexAttribArray(pos_location);
    GLfloat verts[8] = {
        (GLfloat)(box->x1), (GLfloat)(box->y1), (GLfloat)(box->x2), (GLfloat)(box->y1),
        (GLfloat)(box->x2), (GLfloat)(box->y2), (GLfloat)(box->x1), (GLfloat)(box->y2),
    };
    glVertexAttribPointer(pos_location, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(pos_location);
}

static void blur_region_batch_draw(const pixman_region32_t *region, GLint pos_location)
{
    int rects_len;
    const pixman_box32_t *rects = pixman_region32_rectangles(region, &rects_len);

    glEnableVertexAttribArray(pos_location);
    for (int i = 0; i < rects_len;) {
        int batch = rects_len - i < MAX_QUADS ? rects_len - i : MAX_QUADS;
        int batch_end = batch + i;

        size_t vert_index = 0;
        GLfloat verts[MAX_QUADS * 6 * 4];
        for (; i < batch_end; i++) {
            const pixman_box32_t *rect = &rects[i];

            verts[vert_index++] = (GLfloat)rect->x1;
            verts[vert_index++] = (GLfloat)rect->y1;

            verts[vert_index++] = (GLfloat)rect->x2;
            verts[vert_index++] = (GLfloat)rect->y1;

            verts[vert_index++] = (GLfloat)rect->x1;
            verts[vert_index++] = (GLfloat)rect->y2;

            verts[vert_index++] = (GLfloat)rect->x2;
            verts[vert_index++] = (GLfloat)rect->y1;

            verts[vert_index++] = (GLfloat)rect->x2;
            verts[vert_index++] = (GLfloat)rect->y2;

            verts[vert_index++] = (GLfloat)rect->x1;
            verts[vert_index++] = (GLfloat)rect->y2;
        }

        glVertexAttribPointer(pos_location, 2, GL_FLOAT, GL_FALSE, 0, verts);
        glDrawArrays(GL_TRIANGLES, 0, batch * 6);
    }
    glDisableVertexAttribArray(pos_location);
}

static void blur_first_down(struct blur_task_buffer *buffer,
                            const struct blur_task_render_options *options,
                            const pixman_box32_t *blur_bbox)
{
    PROFILE_RENDER_ZONE(&manager->renderer->wlr_renderer, gzone, __func__);
    struct opengl_texture *in_tex = options->in_texture;
    int tex_width = in_tex->wlr_texture.width;
    int tex_height = in_tex->wlr_texture.height;

    pixman_box32_t dst_box = {
        .x1 = blur_bbox->x1 / 2,
        .y1 = blur_bbox->y1 / 2,
        .x2 = blur_bbox->x2 / 2,
        .y2 = blur_bbox->y2 / 2,
    };
    dst_box.x2 = MAX(dst_box.x2, dst_box.x1 + 1);
    dst_box.y2 = MAX(dst_box.y2, dst_box.y1 + 1);

    pixman_box32_t src_box = {
        .x1 = blur_bbox->x1 + 1,
        .y1 = blur_bbox->y1 + 1,
        .x2 = blur_bbox->x2 - 1,
        .y2 = blur_bbox->y2 - 1,
    };
    src_box.x2 = MAX(src_box.x2, src_box.x1 + 1);
    src_box.y2 = MAX(src_box.y2, src_box.y1 + 1);
    float uv_min_x = (float)(src_box.x1) / tex_width;
    float uv_min_y = (float)(src_box.y1) / tex_height;
    float uv_max_x = (float)(src_box.x2) / tex_width;
    float uv_max_y = (float)(src_box.y2) / tex_height;

    struct opengl_texture *out_tex = buffer->textures[0];
    glBindFramebuffer(GL_FRAMEBUFFER, out_tex->fbo);
    glViewport(0, 0, out_tex->wlr_texture.width, out_tex->wlr_texture.height);

    glDisable(GL_BLEND);
    struct blur_shader *shader = &manager->down_shader;
    glUseProgram(shader->prog);

    glUniform2f(shader->frambuffer_size, out_tex->wlr_texture.width, out_tex->wlr_texture.height);
    glUniform4f(shader->tex_clamp, uv_min_x, uv_min_y, uv_max_x, uv_max_y);
    glUniform2f(shader->halfpixel, 0.5f / tex_width, 0.5f / tex_height);
    glUniform1f(shader->offset, options->offset);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(in_tex->target, in_tex->tex);
    glTexParameteri(in_tex->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(in_tex->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(in_tex->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(in_tex->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    blur_box_draw(&dst_box, shader->pos);

    PROFILE_RENDER_ZONE_END(&manager->renderer->wlr_renderer);
}

static void blur_iteration_draw(struct blur_task_buffer *buffer, struct blur_shader *shader,
                                uint32_t in, uint32_t out, const pixman_box32_t *blur_bbox)
{
    struct opengl_texture *in_tex = buffer->textures[in];
    struct opengl_texture *out_tex = buffer->textures[out];

    int scale = 1 << (out + 1);
    pixman_box32_t dst_box = {
        .x1 = blur_bbox->x1 / scale,
        .y1 = blur_bbox->y1 / scale,
        .x2 = blur_bbox->x2 / scale,
        .y2 = blur_bbox->y2 / scale,
    };
    dst_box.x2 = MAX(dst_box.x2, dst_box.x1 + 1);
    dst_box.y2 = MAX(dst_box.y2, dst_box.y1 + 1);

    scale = 1 << (in + 1);
    pixman_box32_t src_box = {
        .x1 = blur_bbox->x1 / scale + 1,
        .y1 = blur_bbox->y1 / scale + 1,
        .x2 = blur_bbox->x2 / scale - 1,
        .y2 = blur_bbox->y2 / scale - 1,
    };
    src_box.x2 = MAX(src_box.x2, src_box.x1 + 1);
    src_box.y2 = MAX(src_box.y2, src_box.y1 + 1);
    float uv_min_x = (float)(src_box.x1) / in_tex->wlr_texture.width;
    float uv_min_y = (float)(src_box.y1) / in_tex->wlr_texture.height;
    float uv_max_x = (float)(src_box.x2) / in_tex->wlr_texture.width;
    float uv_max_y = (float)(src_box.y2) / in_tex->wlr_texture.height;

    glBindFramebuffer(GL_FRAMEBUFFER, out_tex->fbo);
    glViewport(0, 0, out_tex->wlr_texture.width, out_tex->wlr_texture.height);

    glUniform2f(shader->frambuffer_size, out_tex->wlr_texture.width, out_tex->wlr_texture.height);
    glUniform4f(shader->tex_clamp, uv_min_x, uv_min_y, uv_max_x, uv_max_y);
    glUniform2f(shader->halfpixel, 0.5f / in_tex->wlr_texture.width,
                0.5f / in_tex->wlr_texture.height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, in_tex->tex);

    blur_box_draw(&dst_box, shader->pos);
}

static void blur_iteration(struct blur_task_buffer *buffer,
                           const struct blur_task_render_options *options,
                           const pixman_box32_t *blur_bbox)
{
    if (options->iterations < 2) {
        return;
    }

    PROFILE_RENDER_ZONE(&manager->renderer->wlr_renderer, gzone, __func__);

    // down scale
    glDisable(GL_BLEND);
    struct blur_shader *shader = &manager->down_shader;
    glUseProgram(shader->prog);
    glUniform1f(shader->offset, options->offset);
    for (uint32_t i = 0; i < options->iterations - 1; i++) {
        blur_iteration_draw(buffer, shader, i, i + 1, blur_bbox);
    }

    // up scale
    shader = &manager->up_shader;
    glUseProgram(shader->prog);
    glUniform1f(shader->offset, options->offset);
    for (uint32_t i = options->iterations - 1; i > 0; i--) {
        blur_iteration_draw(buffer, shader, i, i - 1, blur_bbox);
    }

    PROFILE_RENDER_ZONE_END(&manager->renderer->wlr_renderer);
}

static void blur_last_up_set_effect_uniform(const struct blur_render_options *options)
{
    struct blur_up_last_shader *shader = &manager->up_last_shader;
    float alpha = options->alpha ? *options->alpha : 1.0f;
    glUniform1f(shader->alpha, alpha * options->blur->alpha);

    glUniform4f(shader->dst_box, options->dst_box->x, options->dst_box->y, options->dst_box->width,
                options->dst_box->height);
    struct mat3 uv_rotation;
    mat3_invert_output_transform(&uv_rotation, options->transform);
    glUniformMatrix3fv(shader->sdf_uv_rotation, 1, GL_FALSE, uv_rotation.matrix);
    int dst_width = options->dst_box->width;
    int dst_height = options->dst_box->height;
    if (options->transform & WL_OUTPUT_TRANSFORM_90) {
        dst_width = options->dst_box->height;
        dst_height = options->dst_box->width;
    }
    glUniform1f(shader->aspect, dst_width / (float)dst_height);
    float one_pixel_distance = 1.0f / dst_height;
    glUniform1f(shader->anti_aliasing, one_pixel_distance);
    if (options->radius) {
        glUniform4f(shader->round_corner_radius, options->radius->rb * one_pixel_distance,
                    options->radius->rt * one_pixel_distance,
                    options->radius->lb * one_pixel_distance,
                    options->radius->lt * one_pixel_distance);
    } else {
        glUniform4f(shader->round_corner_radius, 0.f, 0.f, 0.f, 0.f);
    }
}

static void blur_last_up(struct blur_task_buffer *buffer,
                         const struct blur_task_render_options *options,
                         const pixman_box32_t *blur_bbox)
{
    PROFILE_RENDER_ZONE(&manager->renderer->wlr_renderer, gzone, __func__);

    int out_width = options->in_texture->wlr_texture.width;
    int out_height = options->in_texture->wlr_texture.height;
    struct opengl_texture *in_tex = buffer->textures[0];
    int width = in_tex->wlr_texture.width;
    int height = in_tex->wlr_texture.height;

    pixman_box32_t src_box = {
        .x1 = blur_bbox->x1 / 2 + 1,
        .y1 = blur_bbox->y1 / 2 + 1,
        .x2 = blur_bbox->x2 / 2 - 1,
        .y2 = blur_bbox->y2 / 2 - 1,
    };
    src_box.x2 = MAX(src_box.x2, src_box.x1 + 1);
    src_box.y2 = MAX(src_box.y2, src_box.y1 + 1);
    float uv_min_x = (float)(src_box.x1) / width;
    float uv_min_y = (float)(src_box.y1) / height;
    float uv_max_x = (float)(src_box.x2) / width;
    float uv_max_y = (float)(src_box.y2) / height;

    glBindFramebuffer(GL_FRAMEBUFFER, options->out_fbo);
    glViewport(0, 0, out_width, out_height);

    bool apply_effect = options->apply_blur;
    struct blur_shader *shader;
    if (apply_effect) {
        shader = &manager->up_last_shader.blur;
        glEnable(GL_BLEND);
    } else {
        shader = &manager->up_shader;
        glDisable(GL_BLEND);
    }
    glUseProgram(shader->prog);

    glUniform2f(shader->frambuffer_size, out_width, out_height);
    glUniform4f(shader->tex_clamp, uv_min_x, uv_min_y, uv_max_x, uv_max_y);
    glUniform2f(shader->halfpixel, 0.5f / width, 0.5f / height);
    glUniform1f(shader->offset, options->offset);

    if (apply_effect) {
        blur_last_up_set_effect_uniform(options->apply_blur);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, in_tex->tex);

    if (options->clip && pixman_region32_not_empty(options->clip)) {
        blur_region_batch_draw(options->clip, shader->pos);
    } else {
        blur_box_draw(blur_bbox, shader->pos);
    }

    PROFILE_RENDER_ZONE_END(&manager->renderer->wlr_renderer);
}

struct blur_task *blur_task_create(void)
{
    struct blur_task *task = calloc(1, sizeof(*task));
    if (!task) {
        return NULL;
    }

    wl_list_init(&task->link);
    return task;
}

static inline void check_buffer_need_destroy(struct blur_task *task)
{
    wl_list_remove(&task->link);
    if (wl_list_empty(&task->buffer->tasks)) {
        struct egl_context prev_ctx;
        egl_make_current(manager->renderer->egl, &prev_ctx);
        blur_task_buffer_destroy(task->buffer);
        egl_restore_context(&prev_ctx);
    }
}

void blur_task_destroy(struct blur_task *task)
{
    if (!task) {
        return;
    }

    if (manager && task->buffer) {
        check_buffer_need_destroy(task);
    }

    free(task);
}

void blur_task_render(struct blur_task *task, struct blur_task_render_options *options)
{
    if (!manager || !task) {
        return;
    }

    // Check if we need to allocate/replace the blur buffer for this task
    // 1. No buffer exists yet
    // 2. Existing buffer has mismatched width/height (needs re-association)
    uint32_t in_width = options->in_texture->wlr_texture.width;
    uint32_t in_height = options->in_texture->wlr_texture.height;
    if (!task->buffer || task->buffer->width != in_width || task->buffer->height != in_height) {
        // Clean up and unlink the old buffer if it exists
        if (task->buffer) {
            check_buffer_need_destroy(task);
        }

        task->buffer = get_or_create_blur_task_buffer(in_width, in_height, options->iterations);
        if (!task->buffer) {
            return;
        }
        wl_list_insert(&task->buffer->tasks, &task->link);
    }

    pixman_box32_t blur_bbox = { 0, 0, in_width, in_height };
    if (options->clip && pixman_region32_not_empty(options->clip)) {
        blur_bbox = *pixman_region32_extents(options->clip);
    }

    PROFILE_RENDER_ZONE(&manager->renderer->wlr_renderer, gzone, __func__);

    blur_first_down(task->buffer, options, &blur_bbox);
    blur_iteration(task->buffer, options, &blur_bbox);
    blur_last_up(task->buffer, options, &blur_bbox);

    PROFILE_RENDER_ZONE_END(&manager->renderer->wlr_renderer);
}

static inline void get_blur_shader_location(struct blur_shader *shader)
{
    shader->pos = glGetAttribLocation(shader->prog, "pos");
    shader->frambuffer_size = glGetUniformLocation(shader->prog, "frambufferSize");
    shader->tex_clamp = glGetUniformLocation(shader->prog, "texClamp");
    shader->halfpixel = glGetUniformLocation(shader->prog, "halfpixel");
    shader->offset = glGetUniformLocation(shader->prog, "offset");

    glUseProgram(shader->prog);
    glUniform1i(glGetUniformLocation(shader->prog, "texture"), 0);
    glUseProgram(0);
}

static bool create_blur_shaders(struct opengl_renderer *renderer)
{
    if (manager->down_shader.prog > 0 && manager->up_shader.prog > 0 &&
        manager->up_last_shader.blur.prog > 0) {
        return true;
    }

    struct blur_shader *shader = &manager->down_shader;
    if (shader->prog <= 0) {
        shader->prog = opengl_create_program(renderer, blur_down_vert, blur_down_frag);
        if (shader->prog > 0) {
            get_blur_shader_location(shader);
        } else {
            return false;
        }
    }

    shader = &manager->up_shader;
    if (shader->prog <= 0) {
        shader->prog = opengl_create_program(renderer, blur_up_vert, blur_up_frag);
        if (shader->prog <= 0) {
            return false;
        }
        get_blur_shader_location(shader);
    }

    shader = &manager->up_last_shader.blur;
    if (shader->prog <= 0) {
        shader->prog = opengl_create_program(renderer, blur_up_last_vert, blur_up_last_frag);
        if (shader->prog <= 0) {
            return false;
        }

        get_blur_shader_location(shader);
        manager->up_last_shader.dst_box = glGetUniformLocation(shader->prog, "dstBox");
        manager->up_last_shader.sdf_uv_rotation =
            glGetUniformLocation(shader->prog, "sdfUvRotation");
        manager->up_last_shader.alpha = glGetUniformLocation(shader->prog, "alpha");
        manager->up_last_shader.anti_aliasing = glGetUniformLocation(shader->prog, "antiAliasing");
        manager->up_last_shader.aspect = glGetUniformLocation(shader->prog, "aspect");
        manager->up_last_shader.round_corner_radius =
            glGetUniformLocation(shader->prog, "roundCornerRadius");
    }

    return true;
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct egl_context prev_ctx;
    egl_make_current(manager->renderer->egl, &prev_ctx);

    struct blur_task_buffer *buffer, *tmp;
    wl_list_for_each_safe(buffer, tmp, &manager->buffers, link) {
        // list remove in buffer_destroy need for_each_safe
        blur_task_buffer_destroy(buffer);
    }

    if (manager->down_shader.prog > 0) {
        glDeleteProgram(manager->down_shader.prog);
    }
    if (manager->up_shader.prog > 0) {
        glDeleteProgram(manager->up_shader.prog);
    }
    if (manager->up_last_shader.blur.prog > 0) {
        glDeleteProgram(manager->up_last_shader.blur.prog);
    }

    egl_restore_context(&prev_ctx);

    wl_list_remove(&manager->server_destroy.link);
    free(manager);
    manager = NULL;
}

bool blurrer_create(struct server *server)
{
    if (!wlr_renderer_is_opengl(server->renderer)) {
        return false;
    }

    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    wl_list_init(&manager->buffers);

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    manager->renderer = opengl_renderer_from_wlr_renderer(server->renderer);

    struct egl_context prev_ctx;
    egl_make_current(manager->renderer->egl, &prev_ctx);
    create_blur_shaders(manager->renderer);
    egl_restore_context(&prev_ctx);

    return true;
}
