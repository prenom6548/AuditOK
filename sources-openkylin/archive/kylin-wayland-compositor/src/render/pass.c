// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "render/pass.h"
#include "render/opengl.h"
#include "render/pixman.h"

bool render_pass_options_has_radius(const struct render_round_corner *radius)
{
    return radius->lb > 0 || radius->lt > 0 || radius->rb > 0 || radius->rt > 0;
}

bool render_pass_submit(struct wlr_render_pass *render_pass, uint32_t quirks)
{
    if (wlr_render_pass_is_opengl(render_pass)) {
        struct opengl_render_pass *opengl_pass =
            opengl_render_pass_from_wlr_render_pass(render_pass);
        if (opengl_pass->impl && opengl_pass->impl->submit) {
            return opengl_pass->impl->submit(render_pass, quirks);
        }
    }

    return wlr_render_pass_submit(render_pass);
}

void render_pass_add_texture(struct wlr_render_pass *render_pass,
                             const struct render_texture_options *options)
{
    if (wlr_render_pass_is_opengl(render_pass)) {
        struct opengl_render_pass *opengl_pass =
            opengl_render_pass_from_wlr_render_pass(render_pass);
        if (opengl_pass->impl && opengl_pass->impl->add_texture) {
            opengl_pass->impl->add_texture(render_pass, options);
            return;
        }
    } else if (wlr_render_pass_is_pixman(render_pass)) {
        struct pixman_render_pass *pixman_pass =
            pixman_renderer_pass_from_wlr_render_pass(render_pass);
        if (pixman_pass->impl && pixman_pass->impl->add_texture) {
            pixman_pass->impl->add_texture(render_pass, options);
            return;
        }
    }

    wlr_render_pass_add_texture(render_pass, &options->base);
}

void render_pass_add_rect(struct wlr_render_pass *render_pass,
                          const struct render_rect_options *options)
{
    if (wlr_render_pass_is_opengl(render_pass)) {
        struct opengl_render_pass *opengl_pass =
            opengl_render_pass_from_wlr_render_pass(render_pass);
        if (opengl_pass->impl && opengl_pass->impl->add_rect) {
            opengl_pass->impl->add_rect(render_pass, options);
            return;
        }
    } else if (wlr_render_pass_is_pixman(render_pass)) {
        struct pixman_render_pass *pixman_pass =
            pixman_renderer_pass_from_wlr_render_pass(render_pass);
        if (pixman_pass->impl && pixman_pass->impl->add_rect) {
            pixman_pass->impl->add_rect(render_pass, options);
            return;
        }
    }

    wlr_render_pass_add_rect(render_pass, &options->base);
}

void render_pass_save(struct wlr_render_pass *render_pass)
{
    if (wlr_render_pass_is_opengl(render_pass)) {
        struct opengl_render_pass *opengl_pass =
            opengl_render_pass_from_wlr_render_pass(render_pass);
        opengl_render_pass_save(opengl_pass);
    } else if (wlr_render_pass_is_pixman(render_pass)) {
        struct pixman_render_pass *pixman_pass =
            pixman_renderer_pass_from_wlr_render_pass(render_pass);
        pixman_render_pass_save(pixman_pass);
    }
}

void render_pass_restore(struct wlr_render_pass *render_pass)
{
    if (wlr_render_pass_is_opengl(render_pass)) {
        struct opengl_render_pass *opengl_pass =
            opengl_render_pass_from_wlr_render_pass(render_pass);
        opengl_render_pass_restore(opengl_pass);
    } else if (wlr_render_pass_is_pixman(render_pass)) {
        struct pixman_render_pass *pixman_pass =
            pixman_renderer_pass_from_wlr_render_pass(render_pass);
        pixman_render_pass_restore(pixman_pass);
    }
}

struct wlr_renderer *render_pass_get_renderer(struct wlr_render_pass *render_pass)
{
    if (wlr_render_pass_is_opengl(render_pass)) {
        struct opengl_render_pass *opengl_pass =
            opengl_render_pass_from_wlr_render_pass(render_pass);
        return &opengl_pass->renderer->wlr_renderer;
    }

    return NULL;
}
