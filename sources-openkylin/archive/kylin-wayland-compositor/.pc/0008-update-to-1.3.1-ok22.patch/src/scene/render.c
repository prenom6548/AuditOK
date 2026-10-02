// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <math.h>
#include <stdlib.h>

#include <wlr/types/wlr_output.h>
#include <wlr/util/region.h>

#include "effect/blur.h"
#include "effect/effect.h"
#include "render/pass.h"
#include "render/profile.h"
#include "scene/render.h"

static int scale_length(int length, int offset, float scale)
{
    return round((offset + length) * scale) - round(offset * scale);
}

void ky_scene_render_box(struct wlr_box *box, struct ky_scene_render_target *target)
{
    box->width = scale_length(box->width, box->x, target->scale);
    box->height = scale_length(box->height, box->y, target->scale);
    box->x = round(box->x * target->scale);
    box->y = round(box->y * target->scale);

    enum wl_output_transform transform = wlr_output_transform_invert(target->transform);
    wlr_box_transform(box, box, transform, target->trans_width, target->trans_height);
}

void ky_scene_render_target_box(const struct ky_scene_render_target *target, struct kywc_box *box)
{
    box->x = box->x - target->logical.x;
    box->y = box->y - target->logical.y;

    box->width = scale_length(box->width, box->x, target->scale);
    box->height = scale_length(box->height, box->y, target->scale);
    box->x = round(box->x * target->scale);
    box->y = round(box->y * target->scale);

    enum wl_output_transform transform = wlr_output_transform_invert(target->transform);
    kywc_box_transform(box, box, transform, target->trans_width, target->trans_height);
}

void ky_scene_render_region(pixman_region32_t *region, struct ky_scene_render_target *target)
{
    wlr_region_scale(region, region, target->scale);

    enum wl_output_transform transform = wlr_output_transform_invert(target->transform);
    wlr_region_transform(region, region, transform, target->trans_width, target->trans_height);
}

void ky_scene_render_point(float *x, float *y, struct ky_scene_render_target *target)
{
    *x = roundf(*x * target->scale);
    *y = roundf(*y * target->scale);

    enum wl_output_transform transform = wlr_output_transform_invert(target->transform);
    struct wlr_box temp_box = { *x, *y, 1, 1 };
    wlr_box_transform(&temp_box, &temp_box, transform, target->trans_width, target->trans_height);
    *x = temp_box.x;
    *y = temp_box.y;
}

void ky_scene_render_damage_in_target(struct ky_scene *scene, struct ky_scene_render_target *target)
{
    ky_scene_output_render_begin(target);

    // clear current output buffer damage region
    pixman_region32_t background;
    pixman_region32_init(&background);
    pixman_region32_subtract(&background, &target->damage, &scene->collected_invisible);
    pixman_region32_translate(&background, -target->logical.x, -target->logical.y);
    ky_scene_render_region(&background, target);
    wlr_render_pass_add_rect(target->render_pass, &(struct wlr_render_rect_options){
                                                      .box = { .width = target->buffer->width,
                                                               .height = target->buffer->height },
                                                      .color = { .r = 0, .g = 0, .b = 0, .a = 1 },
                                                      .clip = &background,
                                                  });
    pixman_region32_fini(&background);

    if (!ky_scene_output_render(target)) {
        struct ky_scene_node *root = &scene->tree.node;
        // render each node with damage region and visible region
        root->impl.render(root, root->x, root->y, target);
    }

    ky_scene_output_render_end(target);
}

void ky_scene_render_target_add_software_cursors(struct ky_scene_render_target *target)
{
    struct wlr_output *output = target->output->output;
    bool need_render = false;

    pixman_region32_t damage;
    pixman_region32_init(&damage);

    struct wlr_output_cursor *cursor;
    wl_list_for_each(cursor, &output->cursors, link) {
        if ((!cursor->enabled || !cursor->visible || output->hardware_cursor == cursor) &&
            !(target->options & KY_SCENE_RENDER_ENABLE_CURSORS)) {
            continue;
        }

        struct wlr_texture *texture = cursor->texture;
        if (texture == NULL) {
            continue;
        }

        if (!need_render) {
            pixman_region32_copy(&damage, &target->damage);
            pixman_region32_translate(&damage, -target->logical.x, -target->logical.y);
            ky_scene_render_region(&damage, target);
            pixman_region32_subtract(&damage, &damage, &target->excluded_buffer_damage);
            need_render = true;
        }

        /* cursor is the buffer coordinate of the output, but does not include rotation */
        /* lx ly is the logical coordinate in the scene */
        float scale = target->output->output->scale;
        int lx = (cursor->x - cursor->hotspot_x) / scale + target->output->geometry.x;
        int ly = (cursor->y - cursor->hotspot_y) / scale + target->output->geometry.y;

        struct wlr_box box = {
            .x = lx - target->logical.x,
            .y = ly - target->logical.y,

            .width = cursor->width / scale,
            .height = cursor->height / scale,
        };

        ky_scene_render_box(&box, target);

        pixman_region32_t cursor_damage;
        pixman_region32_init_rect(&cursor_damage, box.x, box.y, box.width, box.height);
        pixman_region32_intersect(&cursor_damage, &cursor_damage, &damage);
        if (!pixman_region32_not_empty(&cursor_damage)) {
            pixman_region32_fini(&cursor_damage);
            continue;
        }

        struct wlr_render_texture_options options = {
            .texture = texture,
            .src_box = cursor->src_box,
            .dst_box = box,
            .clip = &cursor_damage,
            .transform = output->transform,
        };
        wlr_render_pass_add_texture(target->render_pass, &options);
        pixman_region32_fini(&cursor_damage);
    }

    pixman_region32_fini(&damage);
}

static void ky_region_scale(pixman_region32_t *dst, const pixman_region32_t *src, float scale_x,
                            float scale_y)
{
    if (scale_x == 1.0 && scale_y == 1.0) {
        pixman_region32_copy(dst, src);
        return;
    }

    int nrects;
    const pixman_box32_t *src_rects = pixman_region32_rectangles(src, &nrects);
    pixman_box32_t *dst_rects = malloc(nrects * sizeof(pixman_box32_t));
    if (dst_rects == NULL) {
        return;
    }

    for (int i = 0; i < nrects; ++i) {
        dst_rects[i].x1 = (int32_t)roundf(src_rects[i].x1 * scale_x);
        dst_rects[i].x2 = (int32_t)roundf(src_rects[i].x2 * scale_x);
        dst_rects[i].y1 = (int32_t)roundf(src_rects[i].y1 * scale_y);
        dst_rects[i].y2 = (int32_t)roundf(src_rects[i].y2 * scale_y);
    }

    pixman_region32_fini(dst);
    pixman_region32_init_rects(dst, dst_rects, nrects);
    free(dst_rects);
}

void ky_scene_render_target_add_texture(struct ky_scene_render_target *target,
                                        const struct ky_scene_render_texture_options *opts)
{
    struct wlr_box dst_box = {
        .x = opts->geometry_box->x - target->logical.x,
        .y = opts->geometry_box->y - target->logical.y,
        .width = opts->geometry_box->width,
        .height = opts->geometry_box->height,
    };
    ky_scene_render_box(&dst_box, target);

    pixman_region32_t render_region;
    pixman_region32_init(&render_region);
    pixman_region32_copy(&render_region, &target->damage);
    pixman_region32_translate(&render_region, -target->logical.x, -target->logical.y);
    ky_scene_render_region(&render_region, target);

    pixman_region32_intersect_rect(&render_region, &render_region, dst_box.x, dst_box.y,
                                   dst_box.width, dst_box.height);
    if (!pixman_region32_not_empty(&render_region)) {
        pixman_region32_fini(&render_region);
        return;
    }

    enum wl_output_transform transform = wlr_output_transform_invert(opts->transform);
    transform = wlr_output_transform_compose(transform, target->transform);

    struct ky_render_texture_options tex_opts = {
        .base = {
            .texture = opts->texture,
            .dst_box = dst_box,
            .transform = transform,
            .alpha = opts->alpha,
            .clip =  &render_region,
        },
        .radius = {
            .rb = opts->radius ? (*opts->radius)[0] * target->scale : 0,
            .rt = opts->radius ? (*opts->radius)[1] * target->scale : 0,
            .lb = opts->radius ? (*opts->radius)[2] * target->scale : 0,
            .lt = opts->radius ? (*opts->radius)[3] * target->scale : 0,
        },
        .rotation_angle = opts->angle,
    };

    if (opts->src) {
        tex_opts.base.src_box.x = opts->src->x;
        tex_opts.base.src_box.y = opts->src->y;
        tex_opts.base.src_box.width = opts->src->width;
        tex_opts.base.src_box.height = opts->src->height;
    }

    KY_PROFILE_RENDER_ZONE(ky_render_pass_get_renderer(target->render_pass), gzone, __func__);
    if (!(target->options & KY_SCENE_RENDER_DISABLE_BLUR) && opts->blur.info) {
        const struct blur_info *info = opts->blur.info;
        struct blur_info blur = {
            .iterations = info->iterations,
            .offset = info->offset,
            .alpha = info->alpha,
        };
        pixman_region32_init(&blur.region);

        /* region in texture */
        pixman_region32_copy(&blur.region, &info->region);
        pixman_region32_translate(&blur.region, opts->blur.offset_x, opts->blur.offset_y);

        struct wlr_fbox *src_fbox = &tex_opts.base.src_box;
        struct wlr_fbox effective_src = *src_fbox;
        if (*opts->blur.scale != 1.0f && !wlr_fbox_empty(src_fbox)) {
            float scale = 1.0f / *opts->blur.scale;
            effective_src.y = src_fbox->y * scale;
            effective_src.x = src_fbox->x * scale;
            effective_src.width = src_fbox->width * scale;
            effective_src.height = src_fbox->height * scale;
        }

        if (pixman_region32_not_empty(&blur.region) && !wlr_fbox_empty(&effective_src)) {
            int x = floorf(effective_src.x);
            int y = floorf(effective_src.y);
            int width = ceilf(effective_src.x + effective_src.width) - x;
            int height = ceilf(effective_src.y + effective_src.height) - y;
            pixman_region32_intersect_rect(&blur.region, &blur.region, x, y, width, height);
            pixman_region32_translate(&blur.region, -x, -y);
        }

        /* region in tex geometry box, logic coord */
        float src_width = wlr_fbox_empty(src_fbox) ? opts->texture->width : src_fbox->width;
        float src_height = wlr_fbox_empty(src_fbox) ? opts->texture->height : src_fbox->height;
        float scale_w = opts->geometry_box->width / src_width;
        float scale_h = opts->geometry_box->height / src_height;

        ky_region_scale(&blur.region, &blur.region, *opts->blur.scale * scale_w,
                        *opts->blur.scale * scale_h);

        struct wlr_box blur_dst = dst_box;
        if (pixman_region32_not_empty(&blur.region)) {
            pixman_region32_t blur_region;
            pixman_region32_init(&blur_region);
            pixman_region32_copy(&blur_region, &blur.region);
            pixman_region32_translate(&blur_region, opts->geometry_box->x - target->logical.x,
                                      opts->geometry_box->y - target->logical.y);
            ky_scene_render_region(&blur_region, target);

            /* blur beyond blur_dst (sdfbox) will be discarded by the fillet shader */
            pixman_box32_t *blur_bbox = pixman_region32_extents(&blur_region);
            blur_dst.x = blur_bbox->x1;
            blur_dst.y = blur_bbox->y1;
            blur_dst.width = blur_bbox->x2 - blur_bbox->x1;
            blur_dst.height = blur_bbox->y2 - blur_bbox->y1;
            pixman_region32_fini(&blur_region);

            bool fractional_scale_w = floor(scale_w) != scale_w;
            bool fractional_scale_h = floor(scale_h) != scale_h;
            bool fractional_scale_target = floor(target->scale) != target->scale;
            int x_expand = fractional_scale_w || fractional_scale_target ? 1 : 0;
            int y_expand = fractional_scale_h || fractional_scale_target ? 1 : 0;
            struct wlr_box bbox = {
                .x = blur_dst.x + x_expand,
                .y = blur_dst.y + y_expand,
                .width = blur_dst.width - 2 * x_expand,
                .height = blur_dst.height - 2 * y_expand,
            };
            if (bbox.width > 0 && bbox.height > 0) {
                blur_dst = bbox;
            }
        }

        struct ky_render_round_corner radius = {
            .rb = opts->blur.radius ? (*opts->blur.radius)[0] * target->scale : 0,
            .rt = opts->blur.radius ? (*opts->blur.radius)[1] * target->scale : 0,
            .lb = opts->blur.radius ? (*opts->blur.radius)[2] * target->scale : 0,
            .lt = opts->blur.radius ? (*opts->blur.radius)[3] * target->scale : 0,
        };

        /* blur region in tex geometry box, logic coord */
        struct blur_render_options blur_opts = {
            .lx = opts->geometry_box->x,
            .ly = opts->geometry_box->y,
            .dst_box = &blur_dst,
            .clip = &render_region,
            .radius = opts->blur.radius ? &radius : NULL,
            .blur = pixman_region32_not_empty(&blur.region) ? &blur : NULL,
            .alpha = opts->blur.alpha,
        };
        blur_render_with_target(target, &blur_opts);
        pixman_region32_fini(&blur.region);
    }

    ky_render_pass_add_texture(target->render_pass, &tex_opts);
    pixman_region32_fini(&render_region);
    KY_PROFILE_RENDER_ZONE_END(ky_render_pass_get_renderer(target->render_pass));
}
