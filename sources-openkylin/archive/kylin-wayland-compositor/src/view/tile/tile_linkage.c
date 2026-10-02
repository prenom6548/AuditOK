// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <wlr/types/wlr_seat.h>

#include "input/cursor.h"
#include "input/seat.h"
#include "output.h"
#include "scene/stroke_rect.h"
#include "theme.h"
#include "tile_manager_p.h"
#include "util/color.h"
#include "util/macros.h"
#include "view/action.h"

#define LINKAGE_MIN_VIEWS 2
#define LINKAGE_OFFSET 4
#define LINKAGE_PROXY_VIEW_GAP 8
#define LINKAGE_BAR_WIDTH 24
// Acts as height in horizontal orientation, and as width in vertical orientation
#define LINKAGE_BAR_CENTER_DIMENSION_A 10
// Acts as width in horizontal orientation, and as height in vertical orientation
#define LINKAGE_BAR_CENTER_DIMENSION_B 97

struct linkage_item {
    struct wl_list link;
    struct scene_rect *rect;
    struct view *view;
    struct kywc_box geo;
    int32_t x1, y1, x2, y2;
    struct wl_listener view_unmap;
};

struct bar {
    struct scene_tree *tree;
    struct scene_rect *rect;
    struct scene_rect *center;
    struct scene_stroke_rect *border;
    struct kywc_box geo;
};

struct tile_linkage {
    struct scene_tree *tree;
    int32_t edges;
    int32_t min_width, max_width;
    int32_t min_height, max_height;
    /* cursor position before resize */
    double cursor_x, cursor_y;

    struct bar bar;
    struct wl_list items;
    struct linkage_item *active;
    struct output *output;
    /* view position and size before resize */
    struct kywc_box geo;
    struct kywc_box active_pending;

    struct wl_listener active_view_size;
    struct wl_listener active_view_position;

    struct wl_event_source *timer;
};

static void linkage_view_min_size(struct view *view, int32_t *width, int32_t *height)
{
    *width = view->base.min_width;
    *height = view->base.min_height;
    if (!view->base.unconstrained) {
        *width = MAX(view->base.min_width, VIEW_MIN_WIDTH);
        *height = MAX(view->base.min_height, VIEW_MIN_HEIGHT);
    }
}

static bool resize_edges_check(struct view *view, uint32_t edges)
{
    switch (view->base.tiled) {
    case KYWC_TILE_LEFT:
        return edges == KYWC_EDGE_RIGHT;
    case KYWC_TILE_RIGHT:
        return edges == KYWC_EDGE_LEFT;
    case KYWC_TILE_TOP:
        return edges == KYWC_EDGE_BOTTOM;
    case KYWC_TILE_BOTTOM:
        return edges == KYWC_EDGE_TOP;
    case KYWC_TILE_TOP_LEFT:
        return edges == KYWC_EDGE_RIGHT || edges == KYWC_EDGE_BOTTOM;
    case KYWC_TILE_TOP_RIGHT:
        return edges == KYWC_EDGE_LEFT || edges == KYWC_EDGE_BOTTOM;
    case KYWC_TILE_BOTTOM_LEFT:
        return edges == KYWC_EDGE_RIGHT || edges == KYWC_EDGE_TOP;
    case KYWC_TILE_BOTTOM_RIGHT:
        return edges == KYWC_EDGE_LEFT || edges == KYWC_EDGE_TOP;
    default:
        return false;
    }

    return true;
}

static void linkage_item_destroy(struct linkage_item *item)
{
    wl_list_remove(&item->link);
    wl_list_remove(&item->view_unmap.link);
    free(item);
}

static void handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct linkage_item *item = wl_container_of(listener, item, view_unmap);
    linkage_item_destroy(item);
}

static void linkage_adjust_geo_limit(struct tile_linkage *linkage, struct linkage_item *item,
                                     uint32_t edges)
{
    enum kywc_tile tiled = item->view->base.tiled;
    struct kywc_box *usable = &linkage->output->usable_area;
    int ux2 = usable->x + usable->width;
    int uy2 = usable->y + usable->height;
    int min_width, min_height;
    linkage_view_min_size(item->view, &min_width, &min_height);
    min_width += item->view->base.margin.off_width;
    min_height += item->view->base.margin.off_height;

    struct kywc_box active_box = { linkage->active->geo.x - LINKAGE_OFFSET / 2,
                                   linkage->active->geo.y - LINKAGE_OFFSET / 2,
                                   linkage->active->geo.width + LINKAGE_OFFSET,
                                   linkage->active->geo.height + LINKAGE_OFFSET };
    if (!kywc_box_intersects(&item->geo, &active_box)) {
        linkage_item_destroy(item);
        return;
    }

    switch (edges) {
    case KYWC_EDGE_LEFT:
        if (tiled == KYWC_TILE_LEFT || tiled == KYWC_TILE_TOP_LEFT ||
            tiled == KYWC_TILE_BOTTOM_LEFT) {
            if (abs(linkage->active->x1 - item->x2) <= LINKAGE_OFFSET) {
                int remain = ux2 - item->x1 - min_width - (ux2 - linkage->active->x2);
                linkage->max_width = MIN(remain, linkage->max_width);
                return;
            }
        } else if (tiled == KYWC_TILE_RIGHT || tiled == KYWC_TILE_TOP_RIGHT ||
                   tiled == KYWC_TILE_BOTTOM_RIGHT) {
            if (abs(linkage->active->x1 - item->x1) <= LINKAGE_OFFSET) {
                int remain = ux2 - item->x2 + min_width;
                linkage->min_width = MAX(remain, linkage->min_width);
                return;
            }
        }
        break;
    case KYWC_EDGE_RIGHT:
        if (tiled == KYWC_TILE_RIGHT || tiled == KYWC_TILE_TOP_RIGHT ||
            tiled == KYWC_TILE_BOTTOM_RIGHT) {
            if (abs(linkage->active->x2 - item->x1) <= LINKAGE_OFFSET) {
                int remain = item->x2 - min_width - usable->x - (linkage->active->x1 - usable->x);
                linkage->max_width = MIN(remain, linkage->max_width);
                return;
            }
        } else if (tiled == KYWC_TILE_LEFT || tiled == KYWC_TILE_TOP_LEFT ||
                   tiled == KYWC_TILE_BOTTOM_LEFT) {
            if (abs(linkage->active->x2 - item->x2) <= LINKAGE_OFFSET) {
                int remain = item->x1 + min_width - usable->x;
                linkage->min_width = MAX(remain, linkage->min_width);
                return;
            }
        }
        break;
    case KYWC_EDGE_TOP:
        if (tiled == KYWC_TILE_TOP || tiled == KYWC_TILE_TOP_LEFT || tiled == KYWC_TILE_TOP_RIGHT) {
            if (abs(linkage->active->y1 - item->y2) <= LINKAGE_OFFSET) {
                int remain = uy2 - item->y1 - min_height - (uy2 - linkage->active->y2);
                linkage->max_height = MIN(remain, linkage->max_height);
                return;
            }
        } else if (tiled == KYWC_TILE_BOTTOM || tiled == KYWC_TILE_BOTTOM_LEFT ||
                   tiled == KYWC_TILE_BOTTOM_RIGHT) {
            if (abs(linkage->active->y1 - item->y1) <= LINKAGE_OFFSET) {
                int remain = uy2 - (item->y2 - min_height);
                linkage->min_height = MAX(remain, linkage->min_height);
                return;
            }
        }
        break;
    case KYWC_EDGE_BOTTOM:
        if (tiled == KYWC_TILE_BOTTOM || tiled == KYWC_TILE_BOTTOM_LEFT ||
            tiled == KYWC_TILE_BOTTOM_RIGHT) {
            if (abs(linkage->active->y2 - item->y1) <= LINKAGE_OFFSET) {
                int remain = item->y2 - min_height - usable->y - (linkage->active->y1 - usable->y);
                linkage->max_height = MIN(remain, linkage->max_height);
                return;
            }
        } else if (tiled == KYWC_TILE_TOP || tiled == KYWC_TILE_TOP_LEFT ||
                   tiled == KYWC_TILE_TOP_RIGHT) {
            if (abs(linkage->active->y2 - item->y2) <= LINKAGE_OFFSET) {
                int remain = item->y1 - usable->y + min_height;
                linkage->min_height = MAX(remain, linkage->min_height);
                return;
            }
        }
        break;
    default:
        break;
    }

    linkage_item_destroy(item);
}

static void linkage_item_geometry_update(struct tile_linkage *linkage, struct linkage_item *item,
                                         struct kywc_box *pending)
{
    struct view *active = linkage->active->view;
    enum kywc_tile tiled = item->view->base.tiled;

    switch (linkage->edges) {
    case KYWC_EDGE_TOP:
        if (tiled == KYWC_TILE_TOP || tiled == KYWC_TILE_TOP_LEFT || tiled == KYWC_TILE_TOP_RIGHT) {
            item->geo.height = (pending->y - active->base.margin.off_y) - item->geo.y;
        } else if (tiled == KYWC_TILE_BOTTOM || tiled == KYWC_TILE_BOTTOM_LEFT ||
                   tiled == KYWC_TILE_BOTTOM_RIGHT) {
            int item_y2 = item->geo.y + item->geo.height;
            item->geo.y = pending->y - active->base.margin.off_y;
            item->geo.height = item_y2 - item->geo.y;
        }
        break;
    case KYWC_EDGE_BOTTOM:
        if (tiled == KYWC_TILE_BOTTOM || tiled == KYWC_TILE_BOTTOM_LEFT ||
            tiled == KYWC_TILE_BOTTOM_RIGHT) {
            int item_y2 = item->geo.y + item->geo.height;
            item->geo.y = pending->y - active->base.margin.off_y + pending->height +
                          active->base.margin.off_height;
            item->geo.height = item_y2 - item->geo.y;
        } else if (tiled == KYWC_TILE_TOP || tiled == KYWC_TILE_TOP_LEFT ||
                   tiled == KYWC_TILE_TOP_RIGHT) {
            int pending_y2 = pending->y - active->base.margin.off_y + pending->height +
                             active->base.margin.off_height;
            item->geo.height = pending_y2 - item->geo.y;
        }
        break;
    case KYWC_EDGE_LEFT:
        if (tiled == KYWC_TILE_LEFT || tiled == KYWC_TILE_TOP_LEFT ||
            tiled == KYWC_TILE_BOTTOM_LEFT) {
            item->geo.width = pending->x - active->base.margin.off_x - item->geo.x;
        } else if (tiled == KYWC_TILE_RIGHT || tiled == KYWC_TILE_TOP_RIGHT ||
                   tiled == KYWC_TILE_BOTTOM_RIGHT) {
            int item_x2 = item->geo.x + item->geo.width;
            item->geo.x = pending->x - active->base.margin.off_x;
            item->geo.width = item_x2 - item->geo.x;
        }
        break;
    case KYWC_EDGE_RIGHT:
        if (tiled == KYWC_TILE_LEFT || tiled == KYWC_TILE_TOP_LEFT ||
            tiled == KYWC_TILE_BOTTOM_LEFT) {
            int pending_x2 = pending->x - active->base.margin.off_x + pending->width +
                             active->base.margin.off_width;
            item->geo.width = pending_x2 - item->geo.x;
        } else if (tiled == KYWC_TILE_RIGHT || tiled == KYWC_TILE_TOP_RIGHT ||
                   tiled == KYWC_TILE_BOTTOM_RIGHT) {
            int item_x2 = item->geo.x + item->geo.width;
            item->geo.x = pending->x - active->base.margin.off_x + pending->width +
                          active->base.margin.off_width;
            item->geo.width = item_x2 - item->geo.x;
        }
        break;
    default:
        break;
    }

    scene_rect_set_size(item->rect, item->geo.width - LINKAGE_PROXY_VIEW_GAP * 2,
                        item->geo.height - LINKAGE_PROXY_VIEW_GAP * 2);
    scene_node_set_position(&item->rect->node, item->geo.x + LINKAGE_PROXY_VIEW_GAP,
                            item->geo.y + LINKAGE_PROXY_VIEW_GAP);
}

void tile_linkage_destroy(struct tile_linkage *linkage)
{
    struct linkage_item *item, *tmp;
    wl_list_for_each_safe(item, tmp, &linkage->items, link) {
        linkage_item_destroy(item);
    }

    if (linkage->active) {
        wl_list_remove(&linkage->active_view_size.link);
        wl_list_remove(&linkage->active_view_position.link);
    }
    if (linkage->timer) {
        wl_event_source_remove(linkage->timer);
    }
    if (linkage->tree) {
        scene_node_destroy(&linkage->tree->node);
    }
    free(linkage);
}

static void tile_linkage_results_display(struct tile_linkage *linkage, bool cancel)
{
    struct linkage_item *item, *tmp;
    wl_list_for_each_safe(item, tmp, &linkage->items, link) {
        if (item == linkage->active || !item->rect) {
            continue;
        }

        if (!cancel) {
            linkage_item_geometry_update(linkage, item, &linkage->active_pending);
            struct kywc_box pending = {
                item->geo.x + item->view->base.margin.off_x,
                item->geo.y + item->view->base.margin.off_y,
                item->geo.width - item->view->base.margin.off_width,
                item->geo.height - item->view->base.margin.off_height,
            };
            view_do_resize(item->view, &pending);
        }
        scene_node_set_enabled(&item->rect->node, false);
        scene_node_set_enabled(&item->view->tree->node, true);
    }
}

void tile_linkage_resize_done(struct view *view, bool cancel)
{
    struct tiled_output *tiled_output = tiled_output_from_kywc_output(view->output);
    if (!tiled_output || !tiled_output->linkage) {
        return;
    }

    tile_linkage_results_display(tiled_output->linkage, cancel);
    tile_linkage_destroy(tiled_output->linkage);

    tiled_output->linkage = NULL;
}

static void paint_linkage_bar(struct tile_linkage *linkage)
{
    if (!linkage->bar.tree->node.enabled) {
        return;
    }

    scene_node_set_input_bypassed(&linkage->tree->node, true);
    struct kywc_box center = { 0 };
    if (linkage->edges == KYWC_EDGE_LEFT || linkage->edges == KYWC_EDGE_RIGHT) {
        int min_y = linkage->output->usable_area.y + linkage->output->usable_area.height;
        int max_y = linkage->output->usable_area.y;
        struct linkage_item *item;
        wl_list_for_each(item, &linkage->items, link) {
            min_y = MIN(min_y, item->y1);
            max_y = MAX(item->y2, max_y);
        }

        linkage->bar.geo.width = LINKAGE_BAR_WIDTH;
        linkage->bar.geo.height = max_y - min_y;
        linkage->bar.geo.x =
            linkage->edges == KYWC_EDGE_LEFT ? linkage->active->x1 : linkage->active->x2;
        linkage->bar.geo.x -= LINKAGE_BAR_WIDTH / 2;
        linkage->bar.geo.y = min_y;

        center.width = LINKAGE_BAR_CENTER_DIMENSION_A;
        center.height = LINKAGE_BAR_CENTER_DIMENSION_B;
        center.x = (linkage->bar.geo.width - center.width) / 2;
        center.y = (linkage->bar.geo.height - center.height) / 2;
    } else if (linkage->edges == KYWC_EDGE_TOP || linkage->edges == KYWC_EDGE_BOTTOM) {
        int min_x = linkage->output->usable_area.x + linkage->output->usable_area.width;
        int max_x = linkage->output->usable_area.x;
        struct linkage_item *item;
        wl_list_for_each(item, &linkage->items, link) {
            min_x = MIN(min_x, item->x1);
            max_x = MAX(item->x2, max_x);
        }

        linkage->bar.geo.width = max_x - min_x;
        linkage->bar.geo.height = LINKAGE_BAR_WIDTH;
        linkage->bar.geo.x = min_x;
        linkage->bar.geo.y =
            linkage->edges == KYWC_EDGE_TOP ? linkage->active->y1 : linkage->active->y2;
        linkage->bar.geo.y -= LINKAGE_BAR_WIDTH / 2;

        center.width = LINKAGE_BAR_CENTER_DIMENSION_B;
        center.height = LINKAGE_BAR_CENTER_DIMENSION_A;
        center.x = (linkage->bar.geo.width - center.width) / 2;
        center.y = (linkage->bar.geo.height - center.height) / 2;
    }

    struct theme *theme = theme_manager_get_theme();
    float color[4];
    color_float_pax(color, theme->active_bg_color, theme->opacity / 100.0);
    linkage->bar.rect = scene_rect_create(linkage->bar.tree, linkage->bar.geo.width,
                                          linkage->bar.geo.height, color);

    /* add blur */
    pixman_region32_t region;
    pixman_region32_init(&region);
    scene_node_set_blur_region(&linkage->bar.rect->node, theme->opacity != 100 ? &region : NULL);
    pixman_region32_fini(&region);

    color_float_pax(color, theme->active_border_color, theme->opacity / 100.0);
    linkage->bar.border = scene_stroke_rect_create(linkage->bar.tree, linkage->bar.geo.width,
                                                   linkage->bar.geo.height, color, 1);

    color_float_pa(color, theme->accent_color);
    linkage->bar.center = scene_rect_create(linkage->bar.tree, center.width, center.height, color);
    int radius = theme->normal_radius;
    scene_node_set_radius(&linkage->bar.center->node, (int[4]){ radius, radius, radius, radius });

    scene_node_set_position(&linkage->bar.tree->node, linkage->bar.geo.x, linkage->bar.geo.y);
    scene_node_set_position(&linkage->bar.center->node, center.x, center.y);
}

static int handle_linkage_bar_show(void *data)
{
    struct tile_linkage *linkage = data;
    paint_linkage_bar(linkage);
    return 0;
}

static struct linkage_item *linkage_item_create(struct view *view)
{
    struct linkage_item *item = calloc(1, sizeof(*item));
    if (!item) {
        return NULL;
    }

    item->view = view;
    item->x1 = view->base.geometry.x - view->base.margin.off_x;
    item->y1 = view->base.geometry.y - view->base.margin.off_y;
    item->x2 = item->x1 + view->base.geometry.width + view->base.margin.off_width;
    item->y2 = item->y1 + view->base.geometry.height + view->base.margin.off_height;
    struct kywc_box box = { item->x1, item->y1, item->x2 - item->x1, item->y2 - item->y1 };
    item->geo = box;

    item->view_unmap.notify = handle_view_unmap;
    wl_signal_add(&item->view->base.events.unmap, &item->view_unmap);

    return item;
}

static void tile_linkage_geo_update(struct tile_linkage *linkage)
{
    if (!linkage) {
        return;
    }
    struct linkage_item *item;
    wl_list_for_each(item, &linkage->items, link) {
        if (item != linkage->active && item->rect) {
            linkage_item_geometry_update(linkage, item, &linkage->active->view->base.geometry);
        }
    }
}

static void handle_view_size_changed(struct wl_listener *listener, void *data)
{
    struct tile_linkage *linkage = wl_container_of(listener, linkage, active_view_size);

    tile_linkage_geo_update(linkage);
}

static void handle_view_position_changed(struct wl_listener *listener, void *data)
{
    struct tile_linkage *linkage = wl_container_of(listener, linkage, active_view_position);

    tile_linkage_geo_update(linkage);
}

static bool linkage_add_item(struct tile_linkage *linkage, struct tiled_layer layer)
{
    if (layer.proxy[1] && layer.proxy[1]->item->view->base.tiled == KYWC_TILE_TOP) {
        layer.proxy[1] = NULL;
    }
    if (layer.proxy[2] && layer.proxy[2]->item->view->base.tiled == KYWC_TILE_LEFT) {
        layer.proxy[2] = NULL;
    }
    if (layer.proxy[3] && ((layer.proxy[3]->item->view->base.tiled == KYWC_TILE_RIGHT) ||
                           (layer.proxy[3]->item->view->base.tiled == KYWC_TILE_BOTTOM))) {
        layer.proxy[3] = NULL;
    }

    for (int i = 0; i < SLOT_TYPE_COUNT; i++) {
        if (!layer.proxy[i]) {
            continue;
        }
        struct linkage_item *item = linkage_item_create(layer.proxy[i]->item->view);
        if (!item) {
            continue;
        }
        if (layer.proxy[i]->item->view == layer.place->operating_view) {
            linkage->active = item;
            wl_signal_add(&item->view->base.events.size, &linkage->active_view_size);
            wl_signal_add(&item->view->base.events.position, &linkage->active_view_position);
        }
        wl_list_insert(&linkage->items, &item->link);
    }

    return !!linkage->active;
}

static struct tile_linkage *tile_linkage_create(struct view *view, uint32_t edges,
                                                struct tiled_output *tiled_output)
{
    if (!view_need_tile_managerd(view)) {
        return NULL;
    }
    if (!resize_edges_check(view, edges)) {
        return NULL;
    }

    struct tiled_place *place = get_tiled_place(tiled_output, view->current_proxy->workspace);
    if (!place) {
        return NULL;
    }
    struct item_proxy *item_proxy = item_proxy_from_place(view, place);
    if (!item_proxy) {
        return NULL;
    }

    struct tiled_layer layer = { 0 };
    form_virtual_layer_by_tile(view, place, &layer, view->base.tiled);
    if (tiled_layer_is_empty(&layer)) {
        return NULL;
    }

    struct tile_linkage *linkage = calloc(1, sizeof(*linkage));
    if (!linkage) {
        return NULL;
    }

    linkage->output = output_from_kywc_output(view->output);
    wl_list_init(&linkage->items);

    linkage->active_view_size.notify = handle_view_size_changed;
    linkage->active_view_position.notify = handle_view_position_changed;

    if (!linkage_add_item(linkage, layer)) {
        tile_linkage_destroy(linkage);
        return NULL;
    }

    linkage_view_min_size(linkage->active->view, &linkage->min_width, &linkage->min_height);
    linkage->max_width = linkage->output->usable_area.width;
    linkage->max_height = linkage->output->usable_area.height;
    struct linkage_item *item, *tmp;
    wl_list_for_each_safe(item, tmp, &linkage->items, link) {
        if (item != linkage->active) {
            linkage_adjust_geo_limit(linkage, item, edges);
        }
    }
    linkage->max_width -= linkage->active->view->base.margin.off_width;
    linkage->max_height -= linkage->active->view->base.margin.off_height;

    struct view_layer *view_layer = view_manager_get_layer(LAYER_POPUP, false);
    if (wl_list_length(&linkage->items) < LINKAGE_MIN_VIEWS || !view_layer) {
        tile_linkage_destroy(linkage);
        return NULL;
    }

    tiled_output->linkage = linkage;
    linkage->edges = edges;
    linkage->tree = scene_tree_create(view_layer->tree);
    linkage->bar.tree = scene_tree_create(linkage->tree);

    return linkage;
}

static void tile_linkage_bar_hide(struct tile_linkage *linkage)
{
    scene_node_set_enabled(&linkage->bar.tree->node, false);
}

void tile_linkage_bar_show(struct view *view, uint32_t edges)
{
    struct tiled_output *tiled_output = tiled_output_from_kywc_output(view->output);
    if (!tiled_output) {
        return;
    }

    struct tile_linkage *linkage = tiled_output->linkage;
    if (linkage) {
        if (edges == KYWC_EDGE_NONE || !(linkage->edges & edges)) {
            tile_linkage_destroy(linkage);
            tiled_output->linkage = NULL;
        }
        return;
    }

    linkage = tile_linkage_create(view, edges, tiled_output);
    if (!linkage) {
        return;
    }
    tiled_output->linkage = linkage;

    struct seat *seat = input_manager_get_default_seat();
    struct wl_event_loop *loop = wl_display_get_event_loop(seat->wlr_seat->display);
    linkage->timer = wl_event_loop_add_timer(loop, handle_linkage_bar_show, linkage);
    wl_event_source_timer_update(linkage->timer, 300);
}

static void tile_linkage_blur_display(struct tile_linkage *linkage, double lx, double ly)
{
    tile_linkage_bar_hide(linkage);
    linkage->cursor_x = lx;
    linkage->cursor_y = ly;
    linkage->geo = linkage->active->view->base.geometry;
    linkage->active_pending = linkage->geo;

    struct theme *theme = theme_manager_get_theme();
    int radius = theme->window_radius;
    float color[4];
    color_float_pax(color, theme->active_bg_color, theme->opacity / 100.0);
    struct linkage_item *item;
    wl_list_for_each(item, &linkage->items, link) {
        if (item == linkage->active) {
            continue;
        }

        scene_node_set_enabled(&item->view->tree->node, false);
        item->rect = scene_rect_create(linkage->tree, item->geo.width - LINKAGE_PROXY_VIEW_GAP * 2,
                                       item->geo.height - LINKAGE_PROXY_VIEW_GAP * 2, color);
        scene_node_set_radius(&item->rect->node, (int[4]){ radius, radius, radius, radius });
        /* add blur */
        pixman_region32_t region;
        pixman_region32_init(&region);
        scene_node_set_blur_region(&item->rect->node, theme->opacity != 100 ? &region : NULL);
        scene_node_set_position(&item->rect->node, item->geo.x + LINKAGE_PROXY_VIEW_GAP,
                                item->geo.y + LINKAGE_PROXY_VIEW_GAP);
        pixman_region32_fini(&region);
    }
}

static void tile_linkage_view_do_resize(struct tile_linkage *linkage, struct view *view, double lx,
                                        double ly)
{
    struct kywc_box view_box = linkage->active->view->base.geometry;
    double dx = lx - linkage->cursor_x;
    double dy = ly - linkage->cursor_y;

    if (linkage->edges & KYWC_EDGE_TOP) {
        view_box.height = linkage->geo.height - dy;
    } else if (linkage->edges & KYWC_EDGE_BOTTOM) {
        view_box.height = linkage->geo.height + dy;
    }
    view_box.height = CLAMP(view_box.height, linkage->min_height, linkage->max_height);

    if (linkage->edges & KYWC_EDGE_LEFT) {
        view_box.width = linkage->geo.width - dx;
    } else if (linkage->edges & KYWC_EDGE_RIGHT) {
        view_box.width = linkage->geo.width + dx;
    }
    view_box.width = CLAMP(view_box.width, linkage->min_width, linkage->max_width);

    if (linkage->edges & KYWC_EDGE_TOP) {
        /* anchor bottom edge */
        view_box.y = linkage->geo.y + linkage->geo.height - view_box.height;
    }
    if (linkage->edges & KYWC_EDGE_LEFT) {
        /* anchor right edge */
        view_box.x = linkage->geo.x + linkage->geo.width - view_box.width;
    }

    interactive_resize_constraints(view, linkage->output, &view_box, linkage->edges);
    view_do_resize(linkage->active->view, &view_box);
    linkage->active_pending = view_box;
}

bool tile_linkage_view_resize(struct view *view, uint32_t edges, double lx, double ly)
{
    struct tiled_output *tiled_output = tiled_output_from_kywc_output(view->output);
    if (!tiled_output) {
        return false;
    }

    struct tile_linkage *linkage = tiled_output->linkage;
    if (!linkage && !edges) {
        return false;
    } else if (!linkage) {
        linkage = tile_linkage_create(view, edges, tiled_output);
        if (!linkage) {
            return false;
        }
        tiled_output->linkage = linkage;
    }

    if (edges) {
        tile_linkage_blur_display(linkage, lx, ly);
    } else {
        tile_linkage_view_do_resize(linkage, view, lx, ly);
    }
    return true;
}
