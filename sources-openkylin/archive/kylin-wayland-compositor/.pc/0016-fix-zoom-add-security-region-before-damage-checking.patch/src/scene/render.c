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
#include "security/render.h"
#include "util/macros.h"

static int scale_length(int length, int offset, float scale)
{
    return round((offset + length) * scale) - round(offset * scale);
}

void scene_render_box(struct wlr_box *box, struct scene_render_target *target)
{
    box->width = scale_length(box->width, box->x, target->scale);
    box->height = scale_length(box->height, box->y, target->scale);
    box->x = round(box->x * target->scale) + target->offset_x;
    box->y = round(box->y * target->scale) + target->offset_y;

    enum wl_output_transform transform = wlr_output_transform_invert(target->transform);
    wlr_box_transform(box, box, transform, target->trans_width, target->trans_height);
}

void scene_render_target_box(const struct scene_render_target *target, struct kywc_box *box)
{
    box->x = box->x - target->logical.x;
    box->y = box->y - target->logical.y;

    box->width = scale_length(box->width, box->x, target->scale);
    box->height = scale_length(box->height, box->y, target->scale);
    box->x = round(box->x * target->scale) + target->offset_x;
    box->y = round(box->y * target->scale) + target->offset_y;

    enum wl_output_transform transform = wlr_output_transform_invert(target->transform);
    kywc_box_transform(box, box, transform, target->trans_width, target->trans_height);
}

void scene_render_region(pixman_region32_t *region, struct scene_render_target *target)
{
    wlr_region_scale(region, region, target->scale);
    pixman_region32_translate(region, target->offset_x, target->offset_y);

    enum wl_output_transform transform = wlr_output_transform_invert(target->transform);
    wlr_region_transform(region, region, transform, target->trans_width, target->trans_height);
}

void scene_render_target_region(struct scene_render_target *target, pixman_region32_t *region)
{
    pixman_region32_translate(region, -target->logical.x, -target->logical.y);
    scene_render_region(region, target);
}

void scene_render_point(float *x, float *y, struct scene_render_target *target)
{
    *x = roundf(*x * target->scale) + target->offset_x;
    *y = roundf(*y * target->scale) + target->offset_y;

    enum wl_output_transform transform = wlr_output_transform_invert(target->transform);
    struct wlr_box temp_box = { *x, *y, 1, 1 };
    wlr_box_transform(&temp_box, &temp_box, transform, target->trans_width, target->trans_height);
    *x = temp_box.x;
    *y = temp_box.y;
}

/**
 * calc the dst region corresponding to the src region
 * when render src_box of src buffer (src_width * src_height with src_tranform)
 * to dst_box in dst buffer with dst_transform.
 * boxes are all in their buffer local coord
 */
bool scene_render_buffer_region(const struct kywc_fbox *src_box,
                                const pixman_region32_t *src_region,
                                enum wl_output_transform src_transform, int src_width,
                                int src_height, enum wl_output_transform dst_transform,
                                const struct kywc_box *dst_box, pixman_region32_t *dst_region)
{
    pixman_region32_init_rects(dst_region,
                               &(pixman_box32_t){
                                   .x1 = floor(src_box->x),
                                   .x2 = ceil(src_box->x + src_box->width),
                                   .y1 = floor(src_box->y),
                                   .y2 = ceil(src_box->y + src_box->height),
                               },
                               1);
    pixman_region32_intersect(dst_region, dst_region, src_region);
    if (!pixman_region32_not_empty(dst_region)) {
        return false;
    }

    struct kywc_fbox box = *src_box;
    kywc_fbox_transform(&box, &box, src_transform, src_width, src_height);

    wlr_region_transform(dst_region, src_region, src_transform, src_width, src_height);
    pixman_region32_intersect_rect(dst_region, dst_region, box.x, box.y, box.width, box.height);
    pixman_region32_translate(dst_region, -box.x, -box.y);

    int trans_width = dst_box->width;
    int trans_height = dst_box->height;
    if (dst_transform & WL_OUTPUT_TRANSFORM_90) {
        trans_width = dst_box->height;
        trans_height = dst_box->width;
    }
    wlr_region_scale_xy(dst_region, dst_region, trans_width / box.width, trans_height / box.height);

    enum wl_output_transform transform = wlr_output_transform_invert(dst_transform);
    wlr_region_transform(dst_region, dst_region, transform, trans_width, trans_height);
    pixman_region32_translate(dst_region, dst_box->x, dst_box->y);

    return true;
}

// clear current target buffer damage region
static void scene_render_target_clear_buffer(struct scene_render_target *target,
                                             const pixman_region32_t *invisible,
                                             const pixman_region32_t *damage)
{
    pixman_region32_t background;
    pixman_region32_init(&background);
    if (invisible) {
        pixman_region32_subtract(&background, damage, invisible);
    } else {
        pixman_region32_copy(&background, damage);
    }
    pixman_region32_translate(&background, -target->logical.x, -target->logical.y);
    scene_render_region(&background, target);
    wlr_render_pass_add_rect(target->render_pass, &(struct wlr_render_rect_options){
                                                      .box = { .width = target->buffer->width,
                                                               .height = target->buffer->height },
                                                      .color = { .r = 0, .g = 0, .b = 0, .a = 1 },
                                                      .clip = &background,
                                                  });
    pixman_region32_fini(&background);
}

void scene_render_damage_in_target(struct scene *scene, const pixman_region32_t *invisible,
                                   struct scene_render_target *target)
{
    // check whether the output is secure
    bool secure = security_check_output(target->output);
    bool no_access = secure && target->options & SCENE_RENDER_ENABLE_SECURITY &&
                     !security_access(target->client, SECURITY_ACCESS_OUTPUT, target->output);
    struct kywc_box *logical = kywc_box_not_empty(&target->output->viewport.src)
                                   ? &target->output->viewport.src
                                   : &target->output->geometry;

    pixman_region32_t damage;
    pixman_region32_init(&damage);
    // subtract the region of the secure output
    if (no_access) {
        pixman_region32_copy(&damage, &target->damage);
        pixman_region32_t region;
        pixman_region32_init_rect(&region, logical->x, logical->y, logical->width, logical->height);
        pixman_region32_subtract(&target->damage, &target->damage, &region);
        pixman_region32_fini(&region);
    }

    security_target_reset(target);
    // add secure region before rendering
    if (secure && !(target->options & SCENE_RENDER_ENABLE_SECURITY)) {
        security_target_add_output(target, target->output);
    }

    // skip rendering when damage is empty
    if (pixman_region32_not_empty(&target->damage)) {
        scene_output_render_begin(target);
        scene_render_target_clear_buffer(target, invisible, &target->damage);
    if (!scene_output_render(target)) {
        struct scene_node *root = &scene->tree.node;
        // render each node with damage region and visible region
            scene_node_render(root, root->x, root->y, target);
        }
        scene_output_render_end(target);
    } else {
        // damage is not empty when target->damage is empty
        scene_render_target_clear_buffer(target, invisible, &damage);
    }

    if (no_access) {
        struct security_render_options options = {
            .dst_box = { logical->x, logical->y, logical->width, logical->height },
        };
        pixman_region32_union(&target->damage, &target->damage, &damage);
        security_render(target, &options);
    }

    pixman_region32_fini(&damage);
}

void scene_render_target_add_software_cursors(struct scene_render_target *target)
{
    struct wlr_output *output = target->output->output;
    bool need_render = false;

    pixman_region32_t damage;
    pixman_region32_init(&damage);

    struct wlr_output_cursor *cursor;
    wl_list_for_each(cursor, &output->cursors, link) {
        if ((!cursor->enabled || !cursor->visible || output->hardware_cursor == cursor) &&
            !(target->options & SCENE_RENDER_ENABLE_CURSORS)) {
            continue;
        }

        struct wlr_texture *texture = cursor->texture;
        if (texture == NULL) {
            continue;
        }

        if (!need_render) {
            pixman_region32_copy(&damage, &target->damage);
            pixman_region32_translate(&damage, -target->logical.x, -target->logical.y);
            scene_render_region(&damage, target);
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
        scene_render_box(&box, target);

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
            .transform = target->transform,
        };
        wlr_render_pass_add_texture(target->render_pass, &options);
        pixman_region32_fini(&cursor_damage);
    }

    pixman_region32_fini(&damage);
}

static void region_scale(pixman_region32_t *dst, const pixman_region32_t *src, float scale_x,
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

void scene_render_target_add_blur(struct scene_render_target *target,
                                  const struct scene_render_blur_options *options)
{
    if (!options->blur || options->blur->alpha <= 0.0f || options->blur->offset <= 0 ||
        (options->alpha && *options->alpha <= 0.0f)) {
        return;
    }

    const struct kywc_box *boundary = options->round_mask.boundary;
    if (!boundary && !pixman_region32_not_empty(&options->blur->region)) {
        return;
    }

    struct kywc_box boundary_box = { 0 };
    if (!boundary) {
        pixman_box32_t *blur_bbox = pixman_region32_extents(&options->blur->region);
        boundary_box.x = blur_bbox->x1 + options->node_lx;
        boundary_box.y = blur_bbox->y1 + options->node_ly;
        boundary_box.width = blur_bbox->x2 - blur_bbox->x1;
        boundary_box.height = blur_bbox->y2 - blur_bbox->y1;
        scene_render_target_box(target, &boundary_box);
    } else {
        boundary_box = *boundary;
    }

    struct blur_info blur = {
        .type = options->blur->type,
        .iterations = options->blur->iterations,
        .offset = options->blur->offset,
        .alpha = options->blur->alpha,
    };
    if (pixman_region32_not_empty(&options->blur->region)) {
        pixman_region32_init(&blur.region);
        pixman_region32_copy(&blur.region, &options->blur->region);
        pixman_region32_translate(&blur.region, options->node_lx, options->node_ly);
        scene_render_target_region(target, &blur.region);
    } else {
        pixman_region32_init_rect(&blur.region, boundary_box.x, boundary_box.y, boundary_box.width,
                                  boundary_box.height);
    }

    struct blur_render_options blur_opts = {
        .blur = &blur,
        .alpha = options->alpha,
        .clip = options->clip,
        .dst_box = &boundary_box,
        .radius = options->round_mask.radius,
        .transform = target->transform,
    };
    blur_render_with_target(target, &blur_opts);
    pixman_region32_fini(&blur.region);
}

void scene_render_target_add_texture_with_blur(struct scene_render_target *target,
                                               const struct scene_render_texture_options *opts)
{
    struct wlr_box dst_box = {
        .x = opts->geometry_box->x - target->logical.x,
        .y = opts->geometry_box->y - target->logical.y,
        .width = opts->geometry_box->width,
        .height = opts->geometry_box->height,
    };
    scene_render_box(&dst_box, target);

    pixman_region32_t render_region;
    pixman_region32_init(&render_region);
    pixman_region32_copy(&render_region, &target->damage);
    pixman_region32_translate(&render_region, -target->logical.x, -target->logical.y);
    scene_render_region(&render_region, target);

    pixman_region32_intersect_rect(&render_region, &render_region, dst_box.x, dst_box.y,
                                   dst_box.width, dst_box.height);
    if (!pixman_region32_not_empty(&render_region)) {
        pixman_region32_fini(&render_region);
        return;
    }

    if (opts->buffer && security_check_buffer(opts->buffer)) {
        if (target->options & SCENE_RENDER_ENABLE_SECURITY) {
            pixman_region32_t secure;
            pixman_region32_init(&secure);
            if (security_get_region(opts->buffer, target->client, &secure)) {
                struct security_render_options options = {
                    .dst_box = *opts->geometry_box,
                    .radius = {
                        .rb = opts->radius ? (*opts->radius)[0] : 0,
                        .rt = opts->radius ? (*opts->radius)[1] : 0,
                        .lb = opts->radius ? (*opts->radius)[2] : 0,
                        .lt = opts->radius ? (*opts->radius)[3] : 0,
                    },
                    .clip = opts->content_clip,
                    .texture = opts->texture,
                    .src_box = opts->src,
                    .secure_region = &secure,
                    .src_transform = opts->transform,
                };
                security_render(target, &options);
                pixman_region32_fini(&secure);
                pixman_region32_fini(&render_region);
                return;
            }
            pixman_region32_fini(&secure);
            // render normal if no secure region for the client
        } else {
            struct kywc_fbox src_box =
                kywc_fbox_empty(opts->src)
                    ? (struct kywc_fbox){ 0, 0, opts->texture->width, opts->texture->height }
                    : *opts->src;
            security_target_add_buffer(
                target, opts->buffer, &src_box, opts->transform,
                &(struct kywc_box){ dst_box.x, dst_box.y, dst_box.width, dst_box.height });
        }
    }

    enum wl_output_transform transform = wlr_output_transform_invert(opts->transform);
    transform = wlr_output_transform_compose(transform, target->transform);

    struct render_texture_options tex_opts = {
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
        .mix_options = opts->mix_options,
    };

    if (opts->src) {
        tex_opts.base.src_box.x = opts->src->x;
        tex_opts.base.src_box.y = opts->src->y;
        tex_opts.base.src_box.width = opts->src->width;
        tex_opts.base.src_box.height = opts->src->height;
    }

    PROFILE_RENDER_ZONE(render_pass_get_renderer(target->render_pass), gzone, __func__);
    if (!(target->options & SCENE_RENDER_DISABLE_BLUR) && opts->blur.info) {
        const struct blur_info *info = opts->blur.info;
        struct blur_info blur = {
            .type = info->type,
            .iterations = info->iterations,
            .offset = info->offset,
            .alpha = info->alpha,
        };
        pixman_region32_init(&blur.region);

        /* region in texture */
        pixman_region32_copy(&blur.region, &info->region);
        pixman_region32_translate(&blur.region, opts->blur.offset_x, opts->blur.offset_y);

        float blur_scale = opts->blur.scale ? *opts->blur.scale : 1.0f;
        struct wlr_fbox *src_fbox = &tex_opts.base.src_box;
        struct wlr_fbox effective_src = *src_fbox;
        if (!FLOAT_EQUAL(blur_scale, 1.0f) && !wlr_fbox_empty(src_fbox)) {
            effective_src.y = src_fbox->y / blur_scale;
            effective_src.x = src_fbox->x / blur_scale;
            effective_src.width = src_fbox->width / blur_scale;
            effective_src.height = src_fbox->height / blur_scale;
        }

        if (pixman_region32_not_empty(&blur.region) && !wlr_fbox_empty(&effective_src)) {
            int x = floorf(effective_src.x);
            int y = floorf(effective_src.y);
            int width = ceilf(effective_src.x + effective_src.width) - x;
            int height = ceilf(effective_src.y + effective_src.height) - y;
            pixman_region32_intersect_rect(&blur.region, &blur.region, x, y, width, height);
            pixman_region32_translate(&blur.region, -x, -y);
        }

        /* region in tex geometry box, logic coord to framebuffer coord scale */
        float src_width = wlr_fbox_empty(src_fbox) ? opts->texture->width : src_fbox->width;
        float src_height = wlr_fbox_empty(src_fbox) ? opts->texture->height : src_fbox->height;
        float scale_w = dst_box.width / src_width * blur_scale;
        float scale_h = dst_box.height / src_height * blur_scale;
        if (target->transform & WL_OUTPUT_TRANSFORM_90) {
            scale_w = dst_box.height / src_width * blur_scale;
            scale_h = dst_box.width / src_height * blur_scale;
        }

        struct kywc_box blur_dst = { dst_box.x, dst_box.y, dst_box.width, dst_box.height };
        if (pixman_region32_not_empty(&blur.region)) {
            /* logic coord to framebuffer coord */
            region_scale(&blur.region, &blur.region, scale_w, scale_h);
            pixman_region32_translate(
                &blur.region, round((opts->geometry_box->x - target->logical.x) * target->scale),
                round((opts->geometry_box->y - target->logical.y) * target->scale));
            pixman_region32_translate(&blur.region, target->offset_x, target->offset_y);
            enum wl_output_transform transform = wlr_output_transform_invert(target->transform);
            wlr_region_transform(&blur.region, &blur.region, transform, target->trans_width,
                                 target->trans_height);

            /* blur beyond blur_dst (sdfbox) will be discarded by the fillet shader */
            pixman_box32_t *blur_bbox = pixman_region32_extents(&blur.region);
            blur_dst.x = blur_bbox->x1;
            blur_dst.y = blur_bbox->y1;
            blur_dst.width = blur_bbox->x2 - blur_bbox->x1;
            blur_dst.height = blur_bbox->y2 - blur_bbox->y1;
        }

        struct render_round_corner radius = {
            .rb = opts->blur.radius ? (*opts->blur.radius)[0] * target->scale : 0,
            .rt = opts->blur.radius ? (*opts->blur.radius)[1] * target->scale : 0,
            .lb = opts->blur.radius ? (*opts->blur.radius)[2] * target->scale : 0,
            .lt = opts->blur.radius ? (*opts->blur.radius)[3] * target->scale : 0,
        };

        /* blur region in tex geometry box, logic coord */
        struct blur_render_options blur_opts = {
            .dst_box = &blur_dst,
            .clip = &render_region,
            .radius = opts->blur.radius ? &radius : NULL,
            .blur = pixman_region32_not_empty(&blur.region) ? &blur : NULL,
            .alpha = opts->blur.alpha,
            .transform = target->transform,
        };
        blur_render_with_target(target, &blur_opts);
        pixman_region32_fini(&blur.region);
    }

    render_pass_add_texture(target->render_pass, &tex_opts);
    pixman_region32_fini(&render_region);
    PROFILE_RENDER_ZONE_END(render_pass_get_renderer(target->render_pass));
}
