// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <ctype.h>

#include <cairo/cairo.h>
#include <pango/pangocairo.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#include <librsvg/rsvg.h>
#pragma GCC diagnostic pop

#include <kywc/boxes.h>

#include "painter_p.h"
#include "util/macros.h"

#define FONT_WEIGHT (500)

void text_get_metrics(const char *font_name, int font_size, int *ascent, int *descent)
{
    PangoFontDescription *desc = pango_font_description_new();
    pango_font_description_set_family(desc, font_name);
    pango_font_description_set_size(desc, font_size * PANGO_SCALE);
    pango_font_description_set_weight(desc, FONT_WEIGHT);

    PangoFontMap *map = pango_cairo_font_map_get_default();
    PangoContext *context = pango_font_map_create_context(map);

    PangoLanguage *language = pango_context_get_language(context);
    PangoFontMetrics *metrics = pango_context_get_metrics(context, desc, language);
    *ascent = pango_font_metrics_get_ascent(metrics) / 1024;
    *descent = pango_font_metrics_get_descent(metrics) / 1024;

    g_object_unref(context);
    pango_font_description_free(desc);
    pango_font_metrics_unref(metrics);
}

void text_get_size(const char *font_name, int font_size, const char *text, bool single_line,
                   int *width, int *height)
{
    PangoFontDescription *desc = pango_font_description_new();
    pango_font_description_set_family(desc, font_name);
    pango_font_description_set_size(desc, font_size * PANGO_SCALE);
    pango_font_description_set_weight(desc, FONT_WEIGHT);

    PangoFontMap *map = pango_cairo_font_map_get_default();
    PangoContext *context = pango_font_map_create_context(map);
    PangoLayout *layout = pango_layout_new(context);
    pango_layout_set_font_description(layout, desc);

    pango_layout_set_text(layout, text, -1);
    pango_layout_set_single_paragraph_mode(layout, single_line);
    pango_layout_set_width(layout, -1);
    pango_layout_get_pixel_size(layout, width, height);

    g_object_unref(layout);
    g_object_unref(context);
    pango_font_description_free(desc);
}

static void draw_text(cairo_surface_t *surface, cairo_t *cairo, struct draw_info *info,
                      struct kywc_fbox *box)
{
    cairo_set_source_rgba(cairo, info->font_rgba[0], info->font_rgba[1], info->font_rgba[2],
                          info->font_rgba[3]);
    cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);

    PangoFontDescription *desc = pango_font_description_new();
    pango_font_description_set_family(desc, info->font_name);
    pango_font_description_set_size(desc, info->font_size * PANGO_SCALE);
    pango_font_description_set_weight(desc, FONT_WEIGHT);
    if (info->text_attrs & TEXT_ATTR_SLANT) {
        pango_font_description_set_style(desc, PANGO_STYLE_ITALIC);
    }

    PangoLayout *layout = pango_cairo_create_layout(cairo);
    pango_layout_set_font_description(layout, desc);
    pango_layout_set_width(layout, -1);

    // no TEXT_ATTR_CHECKED and TEXT_ATTR_SUBMENU when auto_resize
    double reserved = info->auto_resize ? 0 : box->height;
    bool use_rtl_layout = info->text_attrs & TEXT_ATTR_RTL;
    double lx = box->x, lrx = box->x + box->width, gap;
    // reserved space for checked text and submenu
    lx += use_rtl_layout ? reserved * 0.5 : reserved;
    lrx -= use_rtl_layout ? reserved : reserved * 0.5;

    int width, height, left = lrx - lx;

    if (info->text_attrs & TEXT_ATTR_CHECKED) {
        pango_layout_set_text(layout, "✓", -1);
        pango_layout_get_pixel_size(layout, &width, &height);
        gap = MAX(0, (box->height - height) * 0.5);
        cairo_move_to(cairo, use_rtl_layout ? lrx + gap : lx - width - gap, box->y + gap);
        pango_cairo_show_layout(cairo, layout);
    }

    if (info->text_attrs & TEXT_ATTR_SUBMENU) {
        pango_layout_set_text(layout, use_rtl_layout ? "<" : ">", -1);
        pango_layout_get_pixel_size(layout, &width, &height);
        gap = MAX(0, (box->height - height) * 0.5);
        cairo_move_to(cairo, use_rtl_layout ? lx : lrx - width, box->y + gap);
        pango_cairo_show_layout(cairo, layout);
        left -= width + gap;
        use_rtl_layout ? (lx += width + gap) : (lrx -= width + gap);
    }

    if (STRING_VALID(info->shortcut)) {
        pango_layout_set_text(layout, info->shortcut, -1);
        pango_layout_get_pixel_size(layout, &width, &height);
        gap = MAX(0, (box->height - height) * 0.5);
        cairo_move_to(cairo, use_rtl_layout ? lx : lrx - width, box->y + gap);
        pango_cairo_show_layout(cairo, layout);
        left -= width + gap;
        use_rtl_layout ? (lx += width + gap) : (lrx -= width + gap);
    }

    if (info->text_attrs & TEXT_ATTR_ACCEL) {
        gchar *escaped = g_markup_escape_text(info->text, -1);
        pango_layout_set_markup_with_accel(layout, escaped, -1, '_', NULL);
        g_free(escaped);
    } else {
        pango_layout_set_text(layout, info->text, -1);
    }

    pango_layout_get_pixel_size(layout, &width, &height);
    gap = MAX(0, (box->height - height) * 0.5);
    pango_layout_set_width(layout, left * PANGO_SCALE);
    cairo_move_to(cairo, lx, box->y + gap);

    pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_MIDDLE);
    pango_layout_set_alignment(layout, (PangoAlignment)info->align);
    pango_cairo_show_layout(cairo, layout);

    g_object_unref(layout);
    pango_font_description_free(desc);
}

static void draw_color(cairo_surface_t *surface, cairo_t *cairo, struct draw_info *info,
                       struct kywc_fbox *box, bool hover)
{
    double width = box->width;
    double height = box->height;
    double radius = info->corner_radius;

    if (info->solid_rgba) {
        if (info->corner_mask & CORNER_MASK_TOP_LEFT) {
            cairo_arc(cairo, box->x + radius, box->y + radius, radius, ANGLE(-180), ANGLE(-90));
        } else {
            cairo_line_to(cairo, box->x, box->y);
        }
        if (info->corner_mask & CORNER_MASK_TOP_RIGHT) {
            cairo_arc(cairo, box->x + width - radius, box->y + radius, radius, ANGLE(-90),
                      ANGLE(0));
        } else {
            cairo_line_to(cairo, box->x + width, box->y);
        }
        if (info->corner_mask & CORNER_MASK_BOTTOM_RIGHT) {
            cairo_arc(cairo, box->x + width - radius, box->y + height - radius, radius, ANGLE(0),
                      ANGLE(90));
        } else {
            cairo_line_to(cairo, box->x + width, box->y + height);
        }
        if (info->corner_mask & CORNER_MASK_BOTTOM_LEFT) {
            cairo_arc(cairo, box->x + radius, box->y + height - radius, radius, ANGLE(90),
                      ANGLE(180));
        } else {
            cairo_line_to(cairo, box->x, box->y + height);
        }

        cairo_close_path(cairo);
        cairo_set_source_rgba(cairo, info->solid_rgba[0], info->solid_rgba[1], info->solid_rgba[2],
                              info->solid_rgba[3]);
        cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
        cairo_fill(cairo);
    }

    if (hover) {
        double offset = box->height * 0.1;
        double x = box->x + offset;
        double y = box->y + offset;
        double w = box->width - 2 * offset;
        double h = box->height - 2 * offset;
        float radius = info->hover_radius;
        cairo_arc(cairo, x + radius, y + radius, radius, ANGLE(-180), ANGLE(-90));
        cairo_arc(cairo, x + w - radius, y + radius, radius, ANGLE(-90), ANGLE(0));
        cairo_arc(cairo, x + w - radius, y + h - radius, radius, ANGLE(0), ANGLE(90));
        cairo_arc(cairo, x + radius, y + h - radius, radius, ANGLE(90), ANGLE(180));

        cairo_close_path(cairo);
        cairo_set_source_rgba(cairo, info->hover_rgba[0], info->hover_rgba[1], info->hover_rgba[2],
                              info->hover_rgba[3]);
        cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
        cairo_fill(cairo);
    }

    if (!info->border_rgba || !info->border_width || !info->border_mask) {
        return;
    }

    /* border line */
    double half = info->border_width / 2.0;
    cairo_set_source_rgba(cairo, info->border_rgba[0], info->border_rgba[1], info->border_rgba[2],
                          info->border_rgba[3]);
    cairo_set_line_width(cairo, info->border_width);

    if (info->border_mask & BORDER_MASK_TOP) {
        if (info->corner_mask & CORNER_MASK_TOP_LEFT) {
            cairo_move_to(cairo, box->x + half, box->y + radius);
            cairo_arc(cairo, box->x + radius, box->y + radius, radius - half, ANGLE(-180),
                      ANGLE(-90));
        } else {
            cairo_move_to(cairo, box->x + half, box->y + half);
        }
        if (info->corner_mask & CORNER_MASK_TOP_RIGHT) {
            cairo_line_to(cairo, box->x + width - radius, box->y + half);
            cairo_arc(cairo, box->x + width - radius, box->y + radius, radius - half, ANGLE(-90),
                      ANGLE(0));
        } else {
            cairo_line_to(cairo, box->x + width - half, box->y + half);
        }
        cairo_stroke(cairo);
    }

    if (info->border_mask & BORDER_MASK_RIGHT) {
        if (info->corner_mask & CORNER_MASK_TOP_RIGHT) {
            if (info->border_mask & BORDER_MASK_TOP) {
                cairo_move_to(cairo, box->x + width - half, box->y + radius);
            } else {
                cairo_move_to(cairo, box->x + width - radius, box->y + half);
                cairo_arc(cairo, box->x + width - radius, box->y + radius, radius - half,
                          ANGLE(-90), ANGLE(0));
            }
        } else {
            cairo_move_to(cairo, box->x + width - half, box->y + half);
        }
        if (info->corner_mask & CORNER_MASK_BOTTOM_RIGHT) {
            cairo_line_to(cairo, box->x + width - half, box->y + height - radius);
            cairo_arc(cairo, box->x + width - radius, box->y + height - radius, radius - half,
                      ANGLE(0), ANGLE(90));
        } else {
            cairo_line_to(cairo, box->x + width - half, box->y + height - half);
        }
        cairo_stroke(cairo);
    }

    if (info->border_mask & BORDER_MASK_BOTTOM) {
        if (info->corner_mask & CORNER_MASK_BOTTOM_RIGHT) {
            if (info->border_mask & BORDER_MASK_RIGHT) {
                cairo_move_to(cairo, box->x + width - radius, box->y + height - half);
            } else {
                cairo_move_to(cairo, box->x + width - half, box->y + height - radius);
                cairo_arc(cairo, box->x + width - radius, box->y + height - radius, radius - half,
                          ANGLE(0), ANGLE(90));
            }
        } else {
            cairo_move_to(cairo, box->x + width - half, box->y + height - half);
        }
        if (info->corner_mask & CORNER_MASK_BOTTOM_LEFT) {
            cairo_line_to(cairo, box->x + radius, box->y + height - half);
            cairo_arc(cairo, box->x + radius, box->y + height - radius, radius - half, ANGLE(90),
                      ANGLE(180));
        } else {
            cairo_line_to(cairo, box->x + half, box->y + height - half);
        }
        cairo_stroke(cairo);
    }

    if (info->border_mask & BORDER_MASK_LEFT) {
        if (info->corner_mask & CORNER_MASK_BOTTOM_LEFT) {
            if (info->border_mask & BORDER_MASK_BOTTOM) {
                cairo_move_to(cairo, box->x + half, box->y + height - radius);
            } else {
                cairo_move_to(cairo, box->x + radius, box->y + height - half);
                cairo_arc(cairo, box->x + radius, box->y + height - radius, radius - half,
                          ANGLE(90), ANGLE(180));
            }
        } else {
            cairo_move_to(cairo, box->x + half, box->y + height - half);
        }
        if (info->corner_mask & CORNER_MASK_TOP_LEFT) {
            if (info->border_mask & BORDER_MASK_TOP) {
                cairo_line_to(cairo, box->x + half, box->y + radius);
            } else {
                cairo_line_to(cairo, box->x + half, box->y + radius);
                cairo_arc(cairo, box->x + radius, box->y + radius, radius - half, ANGLE(-180),
                          ANGLE(-90));
            }
        } else {
            cairo_line_to(cairo, box->x + half, box->y + half);
        }
        cairo_stroke(cairo);
    }
}

static bool draw_svg(cairo_t *cairo, const char *data, size_t size, struct kywc_fbox *box)
{
    // check signature, this an xml, so skip spaces from the start
    while (size && isspace(*data) != 0) {
        ++data;
        --size;
    }

    const uint8_t signature[] = { '<' };
    if (size <= sizeof(signature) || memcmp(data, signature, sizeof(signature))) {
        return false;
    }

    GError *err = NULL;
    RsvgHandle *svg = rsvg_handle_new_from_data((guint8 *)data, size, &err);
    if (!svg) {
        kywc_log(KYWC_ERROR, "Invalid SVG format");
        if (err && err->message) {
            kywc_log(KYWC_ERROR, "%s: %s", data, err->message);
        }
        return false;
    }

    RsvgRectangle viewport = {
        .x = box->x, .y = box->y, .width = box->width, .height = box->height
    };

    // render svg to surface
    gboolean ok = rsvg_handle_render_document(svg, cairo, &viewport, &err);
    g_object_unref(svg);
    if (!ok && err && err->message) {
        kywc_log(KYWC_ERROR, "%s", err->message);
    }
    return ok;
}

bool render_buffer(struct painter_buffer *buffer, struct draw_info *info)
{
    if (buffer->own_data && !info->image) {
        memset(buffer->data, 0x0, buffer->base.height * buffer->stride);
    }

    cairo_surface_t *surface = cairo_image_surface_create_for_data(
        buffer->data, CAIRO_FORMAT_ARGB32, buffer->base.width, buffer->base.height, buffer->stride);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        return false;
    }

    cairo_surface_set_device_scale(surface, buffer->scale, buffer->scale);
    cairo_t *cairo = cairo_create(surface);

    double half = info->height / 2.0;
    struct kywc_fbox upper = { 0, 0, info->width, half };
    struct kywc_fbox lower = { 0, half, info->width, half };
    struct kywc_fbox whole = { 0, 0, info->width, info->height };

    /* svg picture */
    if (info->svg.data && info->svg.size > 0) {
        draw_svg(cairo, info->svg.data, info->svg.size, &whole);
    }

    /* corner, solid and border */
    if (info->hover_rgba) {
        draw_color(surface, cairo, info, &upper, false);
        draw_color(surface, cairo, info, &lower, true);
    } else {
        draw_color(surface, cairo, info, &whole, false);
    }

    /* text */
    if (STRING_VALID(info->text) && info->font_rgba) {
        if (info->hover_rgba) {
            draw_text(surface, cairo, info, &upper);
            draw_text(surface, cairo, info, &lower);
        } else {
            draw_text(surface, cairo, info, &whole);
        }
    }

    cairo_surface_flush(surface);
    cairo_destroy(cairo);
    cairo_surface_destroy(surface);

    return true;
}
