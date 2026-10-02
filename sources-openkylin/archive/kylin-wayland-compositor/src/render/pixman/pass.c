// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <cairo.h>
#include <kywc/log.h>

#include "render/pixman.h"
#include "util/macros.h"

static const struct wlr_render_pass_impl wlr_render_pass_impl;

bool wlr_render_pass_is_pixman(struct wlr_render_pass *render_pass)
{
    return render_pass->impl == &wlr_render_pass_impl;
}

struct pixman_render_pass *
pixman_renderer_pass_from_wlr_render_pass(struct wlr_render_pass *wlr_pass)
{
    assert(wlr_pass->impl == &wlr_render_pass_impl);
    struct pixman_render_pass *pass = wl_container_of(wlr_pass, pass, base);
    return pass;
}

static struct pixman_texture *get_texture(struct wlr_texture *wlr_texture)
{
    assert(wlr_texture_is_pixman_texture(wlr_texture));
    struct pixman_texture *texture = wl_container_of(wlr_texture, texture, wlr_texture);
    return texture;
}

static bool render_pass_handle_submit(struct wlr_render_pass *wlr_pass)
{
    struct pixman_render_pass *pass = pixman_renderer_pass_from_wlr_render_pass(wlr_pass);

    wlr_buffer_end_data_ptr_access(pass->buffer->buffer);
    wlr_buffer_unlock(pass->buffer->buffer);
    free(pass);

    return true;
}

static pixman_op_t get_pixman_blending(enum wlr_render_blend_mode mode)
{
    switch (mode) {
    case WLR_RENDER_BLEND_MODE_PREMULTIPLIED:
        return PIXMAN_OP_OVER;
    case WLR_RENDER_BLEND_MODE_NONE:
        return PIXMAN_OP_SRC;
    }
    abort();
}

static pixman_image_t *create_copy_image(pixman_image_t *src_image, struct wlr_box *src_box)
{
    pixman_format_code_t format = pixman_image_get_format(src_image);
    int stride = src_box->width * (PIXMAN_FORMAT_BPP(format) / 8);

    pixman_image_t *image =
        pixman_image_create_bits_no_clear(format, src_box->width, src_box->height, NULL, stride);
    if (!image) {
        return NULL;
    }

    pixman_image_composite32(PIXMAN_OP_SRC, src_image, NULL, image, src_box->x, src_box->y, 0, 0, 0,
                             0, src_box->width, src_box->height);
    return image;
}

static void render_pass_handle_add_texture(struct wlr_render_pass *wlr_pass,
                                           const struct wlr_render_texture_options *options)
{
    struct pixman_render_pass *pass = pixman_renderer_pass_from_wlr_render_pass(wlr_pass);
    struct pixman_texture *texture = get_texture(options->texture);
    struct pixman_buffer *buffer = pass->buffer;
    bool need_copy = buffer->image == texture->image;

    if (!need_copy && !pixman_begin_data_ptr_access(texture->buffer, &texture->image,
                                                    WLR_BUFFER_DATA_PTR_ACCESS_READ)) {
        return;
    }

    struct wlr_fbox src_fbox;
    wlr_render_texture_options_get_src_box(options, &src_fbox);
    struct wlr_box src_box = {
        .x = roundf(src_fbox.x),
        .y = roundf(src_fbox.y),
        .width = roundf(src_fbox.width),
        .height = roundf(src_fbox.height),
    };

    pixman_image_t *dst_image = texture->image;
    if (need_copy) {
        dst_image = create_copy_image(dst_image, &src_box);
        if (!dst_image) {
            return;
        }
        src_box.x = src_box.y = 0;
    }

    pixman_op_t op = get_pixman_blending(options->blend_mode);
    pixman_image_set_clip_region32(buffer->image, (pixman_region32_t *)options->clip);

    struct wlr_box dst_box;
    wlr_render_texture_options_get_dst_box(options, &dst_box);

    pixman_image_t *mask = NULL;
    float alpha = wlr_render_texture_options_get_alpha(options);
    if (alpha != 1) {
        mask = pixman_image_create_solid_fill(&(struct pixman_color){
            .alpha = 0xFFFF * alpha,
        });
    }

    // Rotate the source size into destination coordinates
    struct wlr_box src_box_transformed;
    wlr_box_transform(&src_box_transformed, &src_box, options->transform, buffer->buffer->width,
                      buffer->buffer->height);

    if (options->transform != WL_OUTPUT_TRANSFORM_NORMAL ||
        src_box_transformed.width != dst_box.width ||
        src_box_transformed.height != dst_box.height) {
        // Cosinus/sinus values are exact integers for enum wl_output_transform entries
        int tr_cos = 1, tr_sin = 0, tr_x = 0, tr_y = 0;
        switch (options->transform) {
        case WL_OUTPUT_TRANSFORM_NORMAL:
        case WL_OUTPUT_TRANSFORM_FLIPPED:
            break;
        case WL_OUTPUT_TRANSFORM_90:
        case WL_OUTPUT_TRANSFORM_FLIPPED_90:
            tr_cos = 0;
            tr_sin = 1;
            tr_y = src_box.width;
            break;
        case WL_OUTPUT_TRANSFORM_180:
        case WL_OUTPUT_TRANSFORM_FLIPPED_180:
            tr_cos = -1;
            tr_sin = 0;
            tr_x = src_box.width;
            tr_y = src_box.height;
            break;
        case WL_OUTPUT_TRANSFORM_270:
        case WL_OUTPUT_TRANSFORM_FLIPPED_270:
            tr_cos = 0;
            tr_sin = -1;
            tr_x = src_box.height;
            break;
        }

        // Pixman transforms are generally the opposite of what you expect because they
        // apply to the coordinate system rather than the image.  The comments here
        // refer to what happens to the image, so all the code between
        // pixman_transform_init_identity() and pixman_image_set_transform() is probably
        // best read backwards.  Also this means translations are in the opposite
        // direction, imagine them as moving the origin around rather than moving the
        // image.
        //
        // Beware that this doesn't work quite the same as wp_viewporter: We apply crop
        // before transform and scale, whereas it defines crop in post-transform-scale
        // coordinates.  But this only applies to internal wlroots code - the viewporter
        // extension code makes sure that to clients everything works as it should.

        struct pixman_transform transform;
        pixman_transform_init_identity(&transform);

        // Apply scaling to get to the dst_box size.  Because the scaling is applied last
        // it depends on the whether the rotation swapped width and height, which is why
        // we use src_box_transformed instead of src_box.
        pixman_transform_scale(
            &transform, NULL,
            pixman_double_to_fixed(src_box_transformed.width / (double)dst_box.width),
            pixman_double_to_fixed(src_box_transformed.height / (double)dst_box.height));

        // pixman rotates about the origin which again leaves everything outside of the
        // viewport.  Translate the result so that its new top-left corner is back at the
        // origin.
        pixman_transform_translate(&transform, NULL, -pixman_int_to_fixed(tr_x),
                                   -pixman_int_to_fixed(tr_y));

        // Apply the rotation
        pixman_transform_rotate(&transform, NULL, pixman_int_to_fixed(tr_cos),
                                pixman_int_to_fixed(tr_sin));

        // Apply flip before rotation
        if (options->transform >= WL_OUTPUT_TRANSFORM_FLIPPED) {
            // The flip leaves everything left of the Y axis which is outside the
            // viewport. So translate everything back into the viewport.
            pixman_transform_translate(&transform, NULL, -pixman_int_to_fixed(src_box.width),
                                       pixman_int_to_fixed(0));
            // Flip by applying a scale of -1 to the X axis
            pixman_transform_scale(&transform, NULL, pixman_int_to_fixed(-1),
                                   pixman_int_to_fixed(1));
        }

        // Apply the translation for source crop so the origin is now at the top-left of
        // the region we're actually using.  Do this last so all the other transforms
        // apply on top of this.
        pixman_transform_translate(&transform, NULL, pixman_int_to_fixed(src_box.x),
                                   pixman_int_to_fixed(src_box.y));

        pixman_image_set_transform(dst_image, &transform);

        switch (options->filter_mode) {
        case WLR_SCALE_FILTER_BILINEAR:
            pixman_image_set_filter(dst_image, PIXMAN_FILTER_BILINEAR, NULL, 0);
            break;
        case WLR_SCALE_FILTER_NEAREST:
            pixman_image_set_filter(dst_image, PIXMAN_FILTER_NEAREST, NULL, 0);
            break;
        }

        // Now composite the result onto the pass buffer.  We specify a source origin of 0,0
        // because the x,y part of source crop is already done using the transform. The
        // width,height part of source crop is done here by the width and height we pass:
        // because of the scaling, cropping at the end by dst_box.{width,height} is
        // equivalent to if we cropped at the start by src_box.{width,height}.
        pixman_image_composite32(op, dst_image, mask, buffer->image, 0, 0, // source x,y
                                 0, 0,                                     // mask x,y
                                 dst_box.x, dst_box.y,                     // dest x,y
                                 dst_box.width, dst_box.height             // composite width,height
        );

        pixman_image_set_transform(dst_image, NULL);
    } else {
        // No transforms or crop needed, just a straight blit from the source
        pixman_image_set_transform(dst_image, NULL);
        pixman_image_composite32(op, dst_image, mask, buffer->image, src_box.x, src_box.y, 0, 0,
                                 dst_box.x, dst_box.y, src_box.width, src_box.height);
    }

    pixman_image_set_clip_region32(buffer->image, NULL);

    if (need_copy) {
        pixman_image_unref(dst_image);
    } else {
        wlr_buffer_end_data_ptr_access(texture->buffer);
    }

    if (mask != NULL) {
        pixman_image_unref(mask);
    }
}

static void render_pass_handle_add_texture_repeat(struct wlr_render_pass *wlr_pass,
                                                  const struct render_texture_options *options)
{
    struct pixman_render_pass *pass = pixman_renderer_pass_from_wlr_render_pass(wlr_pass);
    struct pixman_texture *texture = get_texture(options->base.texture);
    struct pixman_buffer *buffer = pass->buffer;
    bool need_copy = buffer->image == texture->image;

    if (!need_copy && !pixman_begin_data_ptr_access(texture->buffer, &texture->image,
                                                    WLR_BUFFER_DATA_PTR_ACCESS_READ)) {
        return;
    }

    int tex_w = texture->buffer->width;
    int tex_h = texture->buffer->height;

    pixman_image_t *src_image = texture->image;
    if (need_copy) {
        // Copy full texture instead of only copying the src_box region
        struct wlr_box full_tex = { 0, 0, tex_w, tex_h };
        src_image = create_copy_image(src_image, &full_tex);
        if (!src_image) {
            return;
        }
    }

    pixman_repeat_t repeat;
    switch (options->repeat_mode) {
    case RENDER_TEXTURE_REPEAT_NORAML:
        repeat = PIXMAN_REPEAT_NORMAL;
        break;
    case RENDER_TEXTURE_REPEAT_MIRROR:
        repeat = PIXMAN_REPEAT_REFLECT;
        break;
    case RENDER_TEXTURE_REPEAT_DECAL:
        repeat = PIXMAN_REPEAT_NONE;
        break;
    case RENDER_TEXTURE_REPEAT_NONE:
    case RENDER_TEXTURE_REPEAT_CLAMP:
    default:
        repeat = PIXMAN_REPEAT_PAD;
        break;
    }
    pixman_image_set_repeat(src_image, repeat);

    pixman_op_t op = get_pixman_blending(options->base.blend_mode);
    pixman_image_set_clip_region32(buffer->image, (pixman_region32_t *)options->base.clip);

    struct wlr_box dst_box;
    wlr_render_texture_options_get_dst_box(&options->base, &dst_box);

    // For premultiplied alpha
    pixman_image_t *mask = NULL;
    float alpha = wlr_render_texture_options_get_alpha(&options->base);
    if (alpha != 1) {
        mask = pixman_image_create_solid_fill(&(struct pixman_color){
            .alpha = 0xFFFF * alpha,
        });
    }

    // src_box represents translation and scaling
    if (options->base.src_box.x != 0.0 || options->base.src_box.y != 0.0 ||
        options->base.src_box.width != tex_w || options->base.src_box.height != tex_h ||
        options->base.transform != WL_OUTPUT_TRANSFORM_NORMAL) {
        float tx = options->base.src_box.x;
        float ty = options->base.src_box.y;
        float sx = options->base.src_box.width / tex_w;
        float sy = options->base.src_box.height / tex_h;

        int tr_cos = 1, tr_sin = 0, tr_x = 0, tr_y = 0;
        switch (options->base.transform) {
        case WL_OUTPUT_TRANSFORM_NORMAL:
        case WL_OUTPUT_TRANSFORM_FLIPPED:
            break;
        case WL_OUTPUT_TRANSFORM_90:
        case WL_OUTPUT_TRANSFORM_FLIPPED_90:
            tr_cos = 0;
            tr_sin = 1;
            tr_y = tex_w;
            break;
        case WL_OUTPUT_TRANSFORM_180:
        case WL_OUTPUT_TRANSFORM_FLIPPED_180:
            tr_cos = -1;
            tr_sin = 0;
            tr_x = tex_w;
            tr_y = tex_h;
            break;
        case WL_OUTPUT_TRANSFORM_270:
        case WL_OUTPUT_TRANSFORM_FLIPPED_270:
            tr_cos = 0;
            tr_sin = -1;
            tr_x = tex_h;
            break;
        }

        struct pixman_transform transform;
        pixman_transform_init_identity(&transform);
        // View order = Inverse transform order: Flip → Rotate → Scale → Translate

        // Translate Inverse
        pixman_transform_translate(&transform, NULL, pixman_double_to_fixed(-tx),
                                   pixman_double_to_fixed(-ty));
        // Scale Inverse
        pixman_transform_scale(&transform, NULL, pixman_double_to_fixed(1.0 / sx),
                               pixman_double_to_fixed(1.0 / sy));
        // Rotation origin correction. Make texture top-left return to origin after rotation
        pixman_transform_translate(&transform, NULL, -pixman_int_to_fixed(tr_x),
                                   -pixman_int_to_fixed(tr_y));
        // Rotate
        pixman_transform_rotate(&transform, NULL, pixman_int_to_fixed(tr_cos),
                                pixman_int_to_fixed(tr_sin));
        // Flip
        if (options->base.transform >= WL_OUTPUT_TRANSFORM_FLIPPED) {
            pixman_transform_translate(&transform, NULL, -pixman_int_to_fixed(tex_w),
                                       pixman_int_to_fixed(0));
            pixman_transform_scale(&transform, NULL, pixman_int_to_fixed(-1),
                                   pixman_int_to_fixed(1));
        }

        pixman_image_set_transform(src_image, &transform);

        switch (options->base.filter_mode) {
        case WLR_SCALE_FILTER_BILINEAR:
            pixman_image_set_filter(src_image, PIXMAN_FILTER_BILINEAR, NULL, 0);
            break;
        case WLR_SCALE_FILTER_NEAREST:
            pixman_image_set_filter(src_image, PIXMAN_FILTER_NEAREST, NULL, 0);
            break;
        }

        pixman_image_composite32(op, src_image, mask, buffer->image, 0, 0, 0, 0, dst_box.x,
                                 dst_box.y, dst_box.width, dst_box.height);

        pixman_image_set_transform(src_image, NULL);
    } else {
        pixman_image_set_transform(src_image, NULL);
        pixman_image_composite32(op, src_image, mask, buffer->image, 0, 0, 0, 0, dst_box.x,
                                 dst_box.y, dst_box.width, dst_box.height);
    }

    pixman_image_set_clip_region32(buffer->image, NULL);
    pixman_image_set_repeat(src_image, PIXMAN_REPEAT_NONE);

    if (need_copy) {
        pixman_image_unref(src_image);
    } else {
        wlr_buffer_end_data_ptr_access(texture->buffer);
    }

    if (mask != NULL) {
        pixman_image_unref(mask);
    }
}

static void render_pass_handle_add_rect(struct wlr_render_pass *wlr_pass,
                                        const struct wlr_render_rect_options *options)
{
    struct pixman_render_pass *pass = pixman_renderer_pass_from_wlr_render_pass(wlr_pass);
    struct pixman_buffer *buffer = pass->buffer;
    struct wlr_box box;
    wlr_render_rect_options_get_box(options, pass->buffer->buffer, &box);

    pixman_op_t op = get_pixman_blending(options->color.a == 1 ? WLR_RENDER_BLEND_MODE_NONE
                                                               : options->blend_mode);

    struct pixman_color color = {
        .red = options->color.r * 0xFFFF,
        .green = options->color.g * 0xFFFF,
        .blue = options->color.b * 0xFFFF,
        .alpha = options->color.a * 0xFFFF,
    };

    pixman_image_t *fill = pixman_image_create_solid_fill(&color);

    pixman_image_set_clip_region32(buffer->image, (pixman_region32_t *)options->clip);
    pixman_image_composite32(op, fill, NULL, buffer->image, 0, 0, 0, 0, box.x, box.y, box.width,
                             box.height);
    pixman_image_set_clip_region32(buffer->image, NULL);

    pixman_image_unref(fill);
}

static const struct wlr_render_pass_impl wlr_render_pass_impl = {
    .submit = render_pass_handle_submit,
    .add_texture = render_pass_handle_add_texture,
    .add_rect = render_pass_handle_add_rect,
};

static void pixman_render_pass_add_texture(struct wlr_render_pass *wlr_pass,
                                           const struct render_texture_options *options)
{
    if (options->repeat_mode != RENDER_TEXTURE_REPEAT_NONE) {
        render_pass_handle_add_texture_repeat(wlr_pass, options);
        return;
    }

    wlr_pass->impl->add_texture(wlr_pass, &options->base);
}

static bool pixman_create_rounded_region(pixman_region32_t *region, const struct wlr_box *box,
                                         const struct render_round_corner *radius)
{
    int width = box->width, height = box->height;
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_A1, width, height);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        return false;
    }

    cairo_t *cairo = cairo_create(surface);

    if (radius->lt > 0) {
        cairo_arc(cairo, radius->lt, radius->lt, radius->lt, ANGLE(-180), ANGLE(-90));
    } else {
        cairo_line_to(cairo, 0, 0);
    }
    if (radius->rt > 0) {
        cairo_arc(cairo, width - radius->rt, radius->rt, radius->rt, ANGLE(-90), ANGLE(0));
    } else {
        cairo_line_to(cairo, width, 0);
    }
    if (radius->rb > 0) {
        cairo_arc(cairo, width - radius->rb, height - radius->rb, radius->rb, ANGLE(0), ANGLE(90));
    } else {
        cairo_line_to(cairo, width, height);
    }
    if (radius->lb > 0) {
        cairo_arc(cairo, radius->lb, height - radius->lb, radius->lb, ANGLE(90), ANGLE(180));
    } else {
        cairo_line_to(cairo, 0, height);
    }

    cairo_close_path(cairo);
    cairo_set_source_rgba(cairo, 0, 0, 0, 1);
    cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
    cairo_fill(cairo);
    cairo_surface_flush(surface);

    void *data = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);
    pixman_image_t *image =
        pixman_image_create_bits_no_clear(PIXMAN_a1, width, height, data, stride);
    if (!image) {
        cairo_destroy(cairo);
        cairo_surface_destroy(surface);
        return false;
    }

    pixman_region32_init_from_image(region, image);
    pixman_region32_translate(region, box->x, box->y);

    pixman_image_unref(image);
    cairo_destroy(cairo);
    cairo_surface_destroy(surface);

    return true;
}

static void pixman_render_pass_add_rect(struct wlr_render_pass *wlr_pass,
                                        const struct render_rect_options *options)
{
    // Stroke implemente by subtract clip region
    if (options->stroke_width > 0.001f) {
        struct wlr_render_rect_options base = options->base;

        // Gradient fallback to pure color
        if (options->linear_gradient) {
            memcpy(&base.color, options->linear_gradient->color_stops[0].color, sizeof(float) * 4);
        }

        pixman_region32_t clip;
        options->base.clip ? pixman_region32_copy(&clip, base.clip)
                           : pixman_region32_init_rect(&clip, base.box.x, base.box.y,
                                                       base.box.width, base.box.height);

        pixman_region32_t sub_clip;
        pixman_region32_init_rect(&sub_clip, base.box.x + options->stroke_width,
                                  base.box.y + options->stroke_width,
                                  base.box.width - options->stroke_width * 2.0,
                                  base.box.height - options->stroke_width * 2.0);

        pixman_region32_subtract(&clip, &clip, &sub_clip);

        base.clip = &clip;
        wlr_pass->impl->add_rect(wlr_pass, &base);
        pixman_region32_fini(&clip);
        return;
    }

    if (!options->base.clip && render_pass_options_has_radius(&options->radius)) {
        pixman_region32_t clip;
        if (pixman_create_rounded_region(&clip, &options->base.box, &options->radius)) {
            struct wlr_render_rect_options base = options->base;
            base.clip = &clip;
            wlr_pass->impl->add_rect(wlr_pass, &base);
            pixman_region32_fini(&clip);
            return;
        }
    }

    wlr_pass->impl->add_rect(wlr_pass, &options->base);
}

static const struct render_pass_impl render_pass_impl = {
    .add_texture = pixman_render_pass_add_texture,
    .add_rect = pixman_render_pass_add_rect,
};

struct pixman_render_pass *pixman_begin_render_pass(struct pixman_buffer *buffer)
{
    struct pixman_render_pass *pass = calloc(1, sizeof(*pass));
    if (pass == NULL) {
        return NULL;
    }

    pass->impl = &render_pass_impl;
    wlr_render_pass_init(&pass->base, &wlr_render_pass_impl);

    if (!pixman_begin_data_ptr_access(buffer->buffer, &buffer->image,
                                      WLR_BUFFER_DATA_PTR_ACCESS_READ |
                                          WLR_BUFFER_DATA_PTR_ACCESS_WRITE)) {
        free(pass);
        return NULL;
    }

    wlr_buffer_lock(buffer->buffer);
    pass->buffer = buffer;

    return pass;
}

void pixman_render_pass_save(struct pixman_render_pass *pass)
{
    wlr_buffer_end_data_ptr_access(pass->buffer->buffer);
}

void pixman_render_pass_restore(struct pixman_render_pass *pass)
{
    void *data = NULL;
    uint32_t drm_format;
    size_t stride;
    if (!wlr_buffer_begin_data_ptr_access(pass->buffer->buffer,
                                          WLR_BUFFER_DATA_PTR_ACCESS_READ |
                                              WLR_BUFFER_DATA_PTR_ACCESS_WRITE,
                                          &data, &drm_format, &stride)) {
        kywc_log(KYWC_ERROR, "pixman render pass restore: failed to begin data ptr access");
    }
}
