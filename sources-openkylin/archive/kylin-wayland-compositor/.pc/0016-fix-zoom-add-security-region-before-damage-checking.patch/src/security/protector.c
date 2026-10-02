// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "security/render.h"
#include "security_p.h"
#include "server.h"
#include "view/view.h"

#include "painter.h"

static struct security_protector {
    struct server *server;
    struct wl_listener server_destroy;

    enum security_protector_mode mode;
    // fill color
    struct wlr_render_color color;
    // fill image
    struct {
        struct wlr_buffer *buffer;
        struct wlr_texture *texture;
    } image;
} *protector = NULL;

static struct wlr_texture *protector_get_image_texture(void)
{
    if (protector->image.texture) {
        return protector->image.texture;
    }

    if (!protector->image.buffer) {
        // draw a image for FILL_IMAGE
        struct draw_info info = {
            .width = 200,
            .height = 100,
            .scale = 1,
            .solid_rgba = (float[4]){ 1, 1, 1, 1 },
            .font_name = "monospace",
            .font_rgba = (float[4]){ 1, 0, 0, 1 },
            .font_size = 30,
            .text = "SECURITY",
            .align = TEXT_ALIGN_CENTER,
            .auto_resize = AUTO_RESIZE_EXTEND,
        };
        protector->image.buffer = painter_draw_buffer(&info);
    }

    if (protector->image.buffer) {
        protector->image.texture =
            wlr_texture_from_buffer(protector->server->renderer, protector->image.buffer);
    }

    return protector->image.texture;
}

void security_render(struct scene_render_target *target, const struct security_render_options *opts)
{
    if (!protector) {
        return;
    }

    assert(target->options & SCENE_RENDER_ENABLE_SECURITY);

    pixman_region32_t render_region;
    pixman_region32_init_rect(&render_region, 0, 0, opts->dst_box.width, opts->dst_box.height);
    if (opts->clip && pixman_region32_not_empty(opts->clip)) {
        pixman_region32_intersect(&render_region, &render_region, opts->clip);
    }
    pixman_region32_translate(&render_region, opts->dst_box.x, opts->dst_box.y);
    pixman_region32_intersect(&render_region, &render_region, &target->damage);

    /* skip render if dst box in not in target->damage */
    if (!pixman_region32_not_empty(&render_region)) {
        pixman_region32_fini(&render_region);
        return;
    }

    struct kywc_box dst_box = opts->dst_box;
    scene_render_target_box(target, &dst_box);
    scene_render_target_region(target, &render_region);

    struct render_round_corner radius = { 0 };
    if (!(target->options & SCENE_RENDER_DISABLE_ROUND_CORNER)) {
        radius = (struct render_round_corner){
            .rb = opts->radius.rb * target->scale,
            .rt = opts->radius.rt * target->scale,
            .lb = opts->radius.lb * target->scale,
            .lt = opts->radius.lt * target->scale,
        };
    }

    enum wl_output_transform transform = wlr_output_transform_invert(opts->src_transform);
    transform = wlr_output_transform_compose(transform, target->transform);

    if (opts->texture) {
        struct kywc_fbox src_box =
            kywc_fbox_empty(opts->src_box)
                ? (struct kywc_fbox){ 0, 0, opts->texture->width, opts->texture->height }
                : *opts->src_box;
        pixman_region32_t secure_region;
        pixman_region32_init(&secure_region);
        if (opts->secure_region) {
            scene_render_buffer_region(&src_box, opts->secure_region, opts->src_transform,
                                       opts->texture->width, opts->texture->height,
                                       target->transform, &dst_box, &secure_region);
        }

        pixman_region32_t clip_region;
        pixman_region32_init(&clip_region);
        pixman_region32_subtract(&clip_region, &render_region, &secure_region);

        // add texture when render region is not empty
        if (pixman_region32_not_empty(&clip_region)) {
            struct render_texture_options options = {
                .base = {
                    .texture = opts->texture,
                    .src_box = { src_box.x, src_box.y, src_box.width, src_box.height },
                    .dst_box = { dst_box.x, dst_box.y, dst_box.width, dst_box.height },
                    .transform = transform,
                    .clip = &clip_region,
                },
                .radius = radius,
            };
            render_pass_add_texture(target->render_pass, &options);
            pixman_region32_intersect(&render_region, &render_region, &secure_region);
        }

        pixman_region32_fini(&clip_region);
        pixman_region32_fini(&secure_region);
    }

    if (pixman_region32_not_empty(&render_region)) {
        if (protector->mode == PROTECTOR_FILL_COLOR) {
            struct render_rect_options options = {
                .base = {
                    .box = { dst_box.x, dst_box.y, dst_box.width, dst_box.height },
                    .color = protector->color,
                    .clip = &render_region,
                    .blend_mode = protector->color.a != 1.0 ?
                                  WLR_RENDER_BLEND_MODE_PREMULTIPLIED : WLR_RENDER_BLEND_MODE_NONE,
                },
                .transform = transform,
                .radius = radius,
            };
            render_pass_add_rect(target->render_pass, &options);
        } else if (protector->mode == PROTECTOR_FILL_IMAGE) {
            struct wlr_texture *tex = protector_get_image_texture();
            if (tex) {
                struct render_texture_options options = {
                    .base.texture = tex,
                    .base.src_box = { (dst_box.width - (int)tex->width) * 0.5,
                                      (dst_box.height - (int)tex->height) * 0.5, tex->width,
                                      tex->height },
                    .base.dst_box = { dst_box.x, dst_box.y, dst_box.width, dst_box.height },
                    .base.clip = &render_region,
                    .base.transform = target->transform,
                    .base.blend_mode = WLR_RENDER_BLEND_MODE_PREMULTIPLIED,
                    .radius = radius,
                    .repeat_mode = RENDER_TEXTURE_REPEAT_CLAMP,
                };
                render_pass_add_texture(target->render_pass, &options);
            }
        } else {
            // do nothing when fill_none
        }
    }

    pixman_region32_fini(&render_region);
}

static void render_view(struct scene_node *node, int lx, int ly, struct scene_render_target *target)
{
    if (security_access(target->client, SECURITY_ACCESS_NODE, node)) {
        node->impl.render(node, lx, ly, target);
        return;
    }

    struct view *view = node->role.data;
    struct kywc_view *kywc_view = &view->base;

    struct render_round_corner radius = { 0 };
    if (kywc_view->has_round_corner && !kywc_view->maximized && !kywc_view->fullscreen &&
        !kywc_view->tiled) {
        radius = (struct render_round_corner){ 12, 12, 12, 12 };
    }
    pixman_region32_t clip;
    pixman_region32_init_rect(&clip, kywc_view->padding.left, kywc_view->padding.top,
                              kywc_view->geometry.width + kywc_view->margin.off_width,
                              kywc_view->geometry.height + kywc_view->margin.off_height);
    struct security_render_options opts = {
        .dst_box.x = lx - kywc_view->margin.off_x - kywc_view->padding.left,
        .dst_box.y = ly - kywc_view->margin.off_y - kywc_view->padding.top,
        .dst_box.width = kywc_view->geometry.width + kywc_view->margin.off_width +
                         kywc_view->padding.left + kywc_view->padding.right,
        .dst_box.height = kywc_view->geometry.height + kywc_view->margin.off_height +
                          kywc_view->padding.top + kywc_view->padding.bottom,
        .radius = radius,
        .clip = &clip,
    };
    security_render(target, &opts);
    pixman_region32_fini(&clip);
}

void security_render_node(struct scene_node *node, int lx, int ly,
                          struct scene_render_target *target)
{
    assert(target->options & SCENE_RENDER_ENABLE_SECURITY && target->client);

    if (node->has_effect) {
        node->impl.render(node, lx, ly, target);
        return;
    }

    if (!node->enabled) {
        return;
    }

    if (node->type != SCENE_NODE_TREE) {
        node->impl.render(node, lx, ly, target);
        return;
    }

    // tree node left
    if (node->role.type == SCENE_ROLE_VIEW) {
        render_view(node, lx, ly, target);
    } else {
        struct scene_tree *tree = scene_tree_from_node(node);
        struct scene_node *child;
        wl_list_for_each(child, &tree->children, link) {
            security_render_node(child, lx + child->x, ly + child->y, target);
        }
    }
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&protector->server_destroy.link);
    wlr_texture_destroy(protector->image.texture);
    wlr_buffer_drop(protector->image.buffer);
    free(protector);
    protector = NULL;
}

bool security_protector_create(struct server *server)
{
    protector = calloc(1, sizeof(*protector));
    if (!protector) {
        return false;
    }

    protector->server = server;
    protector->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &protector->server_destroy);

    protector->mode = PROTECTOR_FILL_IMAGE;
    protector->color = (struct wlr_render_color){ 1, 1, 1, 1 };

    return true;
}

void security_set_protector_mode(enum security_protector_mode mode)
{
    if (!protector || protector->mode == mode) {
        return;
    }

    // destroy the texture if mode changed
    if (protector->mode == PROTECTOR_FILL_IMAGE && protector->image.texture) {
        wlr_texture_destroy(protector->image.texture);
        protector->image.texture = NULL;
    }

    protector->mode = mode;
}

void security_set_protector_color(float r, float g, float b, float a)
{
    if (!protector) {
        return;
    }

    protector->color = (struct wlr_render_color){ r, b, g, a };
}

bool security_set_protector_image(uint32_t width, uint32_t height, void *data)
{
    if (!protector) {
        return false;
    }

    struct draw_info info = {
        .width = width,
        .height = height,
        .scale = 1,
        .pixel.width = width,
        .pixel.height = height,
        .pixel.data = data,
    };
    struct wlr_buffer *buffer = painter_draw_buffer(&info);
    if (!buffer) {
        return false;
    }

    if (protector->image.texture) {
        wlr_texture_destroy(protector->image.texture);
        protector->image.texture = NULL;
    }

    wlr_buffer_drop(protector->image.buffer);
    protector->image.buffer = buffer;

    return true;
}
