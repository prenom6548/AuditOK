// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include "output.h"
#include "tile_manager_p.h"
#include "util/macros.h"

#define EXTRA_RANGE 1 // calculate offset

enum ratio_state {
    RATIO_NONE = 0,
    RATIO_POSITION = 1 << 0,
    RATIO_SIZE = 1 << 1,
    RATIO_ALL = (1 << 2) - 1,
};

static struct tiled_manager {
    struct wl_list tiled_outputs;

    struct wl_listener new_mapped_view;
    struct wl_listener new_output;
    struct wl_listener server_destroy;
} *manager = NULL;

static void tile_get_view_geometry(struct view *view, struct kywc_box *geometry,
                                   struct kywc_output *kywc_output, enum kywc_tile tile);

static void item_ratio_update(struct tiled_item *item, enum ratio_state ratio);

static void tile_view_fix_geometry(struct view *view, struct kywc_box *geo,
                                   struct kywc_output *kywc_output)
{
    struct kywc_view *kywc_view = &view->base;
    *geo = kywc_view->geometry;

    struct tiled_output *tiled_output = tiled_output_from_kywc_output(view->output);
    if (!tiled_output) {
        tile_get_view_geometry(view, geo, kywc_output, view->base.tiled);
        return;
    }

    struct tiled_place *place = get_tiled_place(tiled_output, view->current_proxy->workspace);
    struct item_proxy *proxy = item_proxy_from_place(view, place);
    if (!proxy) {
        return;
    }

    proxy->item->kywc_output = kywc_output;
    struct output *new_output = output_from_kywc_output(kywc_output);
    struct kywc_box *dst_box = &new_output->usable_area;
    struct kywc_box old_geo = *geo;
    geo->width = round(dst_box->width * proxy->item->ratio_w) - kywc_view->margin.off_width;
    geo->height = round(dst_box->height * proxy->item->ratio_h) - kywc_view->margin.off_height;
    geo->x = dst_box->x + round(proxy->item->ratio_x * dst_box->width) + kywc_view->margin.off_x;
    geo->y = dst_box->y + round(proxy->item->ratio_y * dst_box->height) + kywc_view->margin.off_y;

    geo->width = MAX(view->base.min_width, geo->width);
    geo->height = MAX(view->base.min_height, geo->height);

    geo->x = MIN(geo->x, dst_box->x + dst_box->width - geo->width);
    geo->y = MIN(geo->y, dst_box->y + dst_box->height - geo->height);

    if (kywc_output == view->output) {
        if (old_geo.width != geo->width || old_geo.height != geo->height) {
            proxy->item->skip_size_update = true;
        }
        if (old_geo.x != geo->x || old_geo.y != geo->y) {
            proxy->item->skip_pos_update = true;
        }
    }
}

static void get_view_geometry(struct view *view, struct kywc_box *geo);

static int calculate_view_width(struct tiled_layer *layer, int slot_1, int slot_2, int slot_3,
                                struct kywc_box *usable, enum kywc_tile tile)
{
    int width = usable->width / 2;
    struct kywc_box box, diagonal = { 0 };
    if (layer->proxy[slot_3] && layer->proxy[slot_3]->item->view->base.tiled != tile) {
        get_view_geometry(layer->proxy[slot_3]->item->view, &diagonal);
        if (slot_3 == SLOT_TYPE_TOP_LEFT || slot_3 == SLOT_TYPE_BOTTOM_LEFT) {
            width = usable->x + usable->width - (diagonal.x + diagonal.width);
        } else {
            width = diagonal.x - usable->x;
        }
    }

    if (layer->proxy[slot_1] && layer->proxy[slot_1]->item->view->base.tiled != tile) {
        get_view_geometry(layer->proxy[slot_1]->item->view, &box);
        if (diagonal.width == 0 || diagonal.height == 0) {
            if (slot_1 == SLOT_TYPE_TOP_LEFT || slot_1 == SLOT_TYPE_BOTTOM_LEFT) {
                width = box.x + box.width - usable->x;
            } else {
                width = usable->x + usable->width - box.x;
            }
        } else if (box.height < diagonal.height && layer->proxy[slot_2]) {
            get_view_geometry(layer->proxy[slot_2]->item->view, &box);
            if (slot_2 == SLOT_TYPE_TOP_LEFT || slot_2 == SLOT_TYPE_BOTTOM_LEFT) {
                if (usable->x + usable->width - (box.x + box.width) < width) {
                    width = usable->x + usable->width - (box.x + box.width);
                }
            } else if (box.x - usable->x < width) {
                width = box.x - usable->x;
            }
            return width;
        }
    }

    if (layer->proxy[slot_2]) {
        get_view_geometry(layer->proxy[slot_2]->item->view, &box);
        if (slot_2 == SLOT_TYPE_TOP_LEFT || slot_2 == SLOT_TYPE_BOTTOM_LEFT) {
            width = usable->x + usable->width - (box.x + box.width);
        } else {
            width = box.x - usable->x;
        }
    }

    return width;
}

static int calculate_view_height(struct tiled_layer *layer, int slot_1, int slot_2, int slot_3,
                                 struct kywc_box *usable, enum kywc_tile tile)
{
    int height = usable->height / 2;
    struct kywc_box box;
    if (layer->proxy[slot_1]) {
        get_view_geometry(layer->proxy[slot_1]->item->view, &box);
        if (slot_1 == SLOT_TYPE_TOP_LEFT || slot_1 == SLOT_TYPE_TOP_RIGHT) {
            height = usable->y + usable->height - (box.y + box.height);
        } else {
            height = box.y - usable->y;
        }
    } else if (layer->proxy[slot_2] && layer->proxy[slot_2]->item->view->base.tiled != tile) {
        get_view_geometry(layer->proxy[slot_2]->item->view, &box);
        if (slot_2 == SLOT_TYPE_TOP_LEFT || slot_2 == SLOT_TYPE_TOP_RIGHT) {
            height = usable->y + usable->height - (box.y + box.height);
        } else {
            height = box.y - usable->y;
        }
    } else if (layer->proxy[slot_3] && layer->proxy[slot_3]->item->view->base.tiled != tile) {
        get_view_geometry(layer->proxy[slot_3]->item->view, &box);
        if (slot_3 == SLOT_TYPE_TOP_LEFT || slot_3 == SLOT_TYPE_TOP_RIGHT) {
            height = box.y + box.height - usable->y;
        } else {
            height = usable->y + usable->height - box.y;
        }
    }

    return height;
}

void tile_calculate_view_geometry(struct tiled_layer *layer, struct output *output,
                                  struct kywc_box *geo, enum kywc_tile tile)
{
    struct kywc_box *usable = &output->usable_area;
    if (tile == KYWC_TILE_LEFT || tile == KYWC_TILE_RIGHT || tile == KYWC_TILE_TOP ||
        tile == KYWC_TILE_BOTTOM) {
        bool left_or_right = tile == KYWC_TILE_LEFT || tile == KYWC_TILE_RIGHT;
        geo->width = left_or_right ? usable->width / 2 : usable->width;
        geo->height = left_or_right ? usable->height : usable->height / 2;
        geo->x = tile == KYWC_TILE_RIGHT ? usable->x + geo->width : usable->x;
        geo->y = tile == KYWC_TILE_BOTTOM ? usable->y + geo->height : usable->y;

        if (tiled_layer_is_empty(layer)) {
            return;
        }

        geo->width = usable->width;
        geo->height = usable->height;
        int remain = 0;
        for (int i = 0; i < SLOT_TYPE_COUNT; i++) {
            if (!layer->proxy[i]) {
                continue;
            }
            struct kywc_box box;
            get_view_geometry(layer->proxy[i]->item->view, &box);
            if (left_or_right) {
                int u_x2 = usable->x + usable->width;
                remain = tile == KYWC_TILE_LEFT ? box.x - usable->x : u_x2 - (box.x + box.width);
                if (remain < geo->width) {
                    geo->width = remain;
                    geo->x = tile == KYWC_TILE_RIGHT ? u_x2 - remain : geo->x;
                }
            } else {
                int u_y2 = usable->y + usable->height;
                remain = tile == KYWC_TILE_TOP ? box.y - usable->y : u_y2 - (box.y + box.height);
                if (remain < geo->height) {
                    geo->height = remain;
                    geo->y = tile == KYWC_TILE_BOTTOM ? u_y2 - remain : geo->y;
                }
            }
        }
        return;
    }

    if (tile == KYWC_TILE_TOP_LEFT) {
        geo->x = usable->x;
        geo->y = usable->y;
        geo->width = calculate_view_width(layer, SLOT_TYPE_BOTTOM_LEFT, SLOT_TYPE_TOP_RIGHT,
                                          SLOT_TYPE_BOTTOM_RIGHT, usable, KYWC_TILE_BOTTOM);
        geo->height = calculate_view_height(layer, SLOT_TYPE_BOTTOM_LEFT, SLOT_TYPE_BOTTOM_RIGHT,
                                            SLOT_TYPE_TOP_RIGHT, usable, KYWC_TILE_RIGHT);
    } else if (tile == KYWC_TILE_TOP_RIGHT) {
        geo->width = calculate_view_width(layer, SLOT_TYPE_BOTTOM_RIGHT, SLOT_TYPE_TOP_LEFT,
                                          SLOT_TYPE_BOTTOM_LEFT, usable, KYWC_TILE_BOTTOM);
        geo->height = calculate_view_height(layer, SLOT_TYPE_BOTTOM_RIGHT, SLOT_TYPE_BOTTOM_LEFT,
                                            SLOT_TYPE_TOP_LEFT, usable, KYWC_TILE_LEFT);
        geo->x = usable->x + usable->width - geo->width;
        geo->y = usable->y;
    } else if (tile == KYWC_TILE_BOTTOM_LEFT) {
        geo->width = calculate_view_width(layer, SLOT_TYPE_TOP_LEFT, SLOT_TYPE_BOTTOM_RIGHT,
                                          SLOT_TYPE_TOP_RIGHT, usable, KYWC_TILE_TOP);
        geo->height = calculate_view_height(layer, SLOT_TYPE_TOP_LEFT, SLOT_TYPE_TOP_RIGHT,
                                            SLOT_TYPE_BOTTOM_RIGHT, usable, KYWC_TILE_RIGHT);
        geo->x = usable->x;
        geo->y = usable->y + usable->height - geo->height;
    } else if (tile == KYWC_TILE_BOTTOM_RIGHT) {
        geo->width = calculate_view_width(layer, SLOT_TYPE_TOP_RIGHT, SLOT_TYPE_BOTTOM_LEFT,
                                          SLOT_TYPE_TOP_LEFT, usable, KYWC_TILE_TOP);
        geo->height = calculate_view_height(layer, SLOT_TYPE_TOP_RIGHT, SLOT_TYPE_TOP_LEFT,
                                            SLOT_TYPE_BOTTOM_LEFT, usable, KYWC_TILE_LEFT);
        geo->x = usable->x + usable->width - geo->width;
        geo->y = usable->y + usable->height - geo->height;
    }
}

static void item_save_tiled_geometry(struct view *view, struct kywc_box *geometry)
{
    struct tiled_output *view_output = tiled_output_from_kywc_output(view->output);
    if (view_output) {
        struct tiled_place *place = get_tiled_place(view_output, view->current_proxy->workspace);
        struct item_proxy *proxy = item_proxy_from_place(view, place);
        if (proxy) {
            proxy->item->tiled_geo = *geometry;
        }
    }
}

static void tile_get_view_geometry(struct view *view, struct kywc_box *geometry,
                                   struct kywc_output *kywc_output, enum kywc_tile tile)
{
    struct tiled_layer layer = { 0 };
    struct tiled_output *tiled_output = tiled_output_from_kywc_output(kywc_output);
    if (tiled_output) {
        struct tiled_place *place = get_tiled_place(tiled_output, view->current_proxy->workspace);
        form_virtual_layer_by_tile(view, place, &layer, tile);
    }

    /* When calculating the tile range, if it is still in the layer, it will not be calculated. */
    for (int i = 0; i < SLOT_TYPE_COUNT; i++) {
        if (layer.proxy[i] && layer.proxy[i]->item->view == view) {
            return;
        }
    }

    struct output *output = output_from_kywc_output(kywc_output);
    tile_calculate_view_geometry(&layer, output, geometry, tile);
    int32_t min_width = view->base.min_width + view->base.margin.off_width;
    int32_t min_height = view->base.min_height + view->base.margin.off_height;
    bool use_min_width = geometry->width < min_width;
    bool use_min_height = geometry->height < min_height;
    geometry->width = MAX(min_width, geometry->width);
    geometry->height = MAX(min_height, geometry->height);

    struct kywc_box *usable = &output->usable_area;
    if (use_min_width && (tile == KYWC_TILE_RIGHT || tile == KYWC_TILE_TOP_RIGHT ||
                          tile == KYWC_TILE_BOTTOM_RIGHT)) {
        geometry->x = usable->x + usable->width - geometry->width;
    }
    if (use_min_height && (tile == KYWC_TILE_BOTTOM || tile == KYWC_TILE_BOTTOM_LEFT ||
                           tile == KYWC_TILE_BOTTOM_RIGHT)) {
        geometry->y = usable->y + usable->height - geometry->height;
    }

    geometry->x = MAX(geometry->x, usable->x);
    geometry->y = MAX(geometry->y, usable->y);

    geometry->x += view->base.margin.off_x;
    geometry->y += view->base.margin.off_y;
    geometry->width -= view->base.margin.off_width;
    geometry->height -= view->base.margin.off_height;

    item_save_tiled_geometry(view, geometry);
}

static void remove_all_item_proxy(struct tiled_item *item);

static void update_all_item_proxy(struct tiled_item *item);

static void tile_view_apply_tiling(struct view *view, enum kywc_tile tile,
                                   struct kywc_output *kywc_output)
{
    struct kywc_output *output = kywc_output ? kywc_output : view->output;
    struct tiled_output *tiled_output = tiled_output_from_kywc_output(output);
    struct item_proxy *proxy = NULL;

    if (tiled_output) {
        struct tiled_place *place = get_tiled_place(tiled_output, view->current_proxy->workspace);
        proxy = item_proxy_from_place(view, place);
        if (proxy) {
            remove_all_item_proxy(proxy->item);
        }
    }

    struct kywc_box geo = { 0 };
    view_get_tiled_geometry(view, &geo, output, tile);

    if (proxy && view->base.tiled == tile && output == view->output &&
        kywc_box_equal(&view->base.geometry, &geo)) {
        update_all_item_proxy(proxy->item);
        return;
    }

    if (tile) {
        scene_node_set_enabled(&view->tree->node, false);
    }
    view_do_tiled(view, tile, output, &geo);
}

static bool item_need_tile_assist(struct tiled_item *item)
{
    return item->tiled_geo.x == item->view->base.geometry.x &&
           item->tiled_geo.y == item->view->base.geometry.y &&
           item->tiled_geo.width == item->view->base.geometry.width &&
           item->tiled_geo.height == item->view->base.geometry.height;
}

static void view_try_show_assist(struct view *view)
{
    struct tiled_output *tiled_output = tiled_output_from_kywc_output(view->output);
    if (!tiled_output || !tiled_output->in_assist || !tiled_output->assist_seat) {
        return;
    }

    struct tiled_place *view_place = get_tiled_place(tiled_output, view->current_proxy->workspace);
    struct item_proxy *proxy = item_proxy_from_place(view, view_place);
    if (!proxy) {
        return;
    }

    if (item_need_tile_assist(proxy->item)) {
        if (proxy->layer) {
            tiled_output->in_assist = false;
            struct seat *seat = tiled_output->assist_seat;
            tiled_output->assist_seat = NULL;
            view_begin_tile_assist(view, seat, output_from_kywc_output(view->output), proxy->layer);
        }
    }
}

static void tile_view_tiled_assist(struct view *view, struct seat *seat,
                                   struct kywc_output *kywc_output)
{
    struct tiled_output *tiled_output = tiled_output_from_kywc_output(kywc_output);
    if (!tiled_output) {
        return;
    }

    tiled_output->in_assist = true;
    tiled_output->assist_seat = seat;
    view_try_show_assist(view);
}

struct item_proxy *item_proxy_from_place(struct view *view, struct tiled_place *place)
{
    if (!place) {
        return NULL;
    }

    struct item_proxy *proxy;
    wl_list_for_each(proxy, &place->item_proxys, place_link) {
        if (proxy->item->view == view) {
            return proxy;
        }
    }
    return NULL;
}

struct tiled_output *tiled_output_from_kywc_output(struct kywc_output *kywc_output)
{
    if (!manager) {
        return NULL;
    }

    struct tiled_output *tiled_output;
    wl_list_for_each(tiled_output, &manager->tiled_outputs, link) {
        if (tiled_output->kywc_output == kywc_output ||
            !strcmp(tiled_output->output_name, kywc_output->name)) {
            return tiled_output;
        }
    }
    return NULL;
}

static void handle_workspace_destroy(struct wl_listener *listener, void *data)
{
    struct tiled_place *place = wl_container_of(listener, place, workspace_destroy);

    wl_list_remove(&place->workspace_destroy.link);
    wl_list_remove(&place->link);

    free(place);
}

struct tiled_place *get_tiled_place(struct tiled_output *tiled_output, struct workspace *workspace)
{
    if (!tiled_output) {
        return NULL;
    }

    struct tiled_place *place;
    wl_list_for_each(place, &tiled_output->tiled_places, link) {
        if (place->workspace == workspace) {
            return place;
        }
    }

    /* create a new place for this workspace */
    place = calloc(1, sizeof(*place));
    if (!place) {
        return NULL;
    }

    place->workspace = workspace;
    place->tiled_output = tiled_output;

    wl_list_init(&place->item_proxys);
    wl_list_init(&place->layers);
    wl_list_insert(&tiled_output->tiled_places, &place->link);

    place->workspace_destroy.notify = handle_workspace_destroy;
    wl_signal_add(&workspace->events.destroy, &place->workspace_destroy);

    return place;
}

static void get_view_geometry(struct view *view, struct kywc_box *geo)
{
    geo->x = view->base.geometry.x - view->base.margin.off_x;
    geo->y = view->base.geometry.y - view->base.margin.off_y;
    geo->width = view->base.geometry.width + view->base.margin.off_width;
    geo->height = view->base.geometry.height + view->base.margin.off_height;
}

static void get_min_view_geometry(struct view *view, struct kywc_box *geo, struct kywc_box *usable,
                                  int tile)
{
    geo->width = (view->base.min_width ? view->base.min_width : VIEW_MIN_WIDTH) +
                 view->base.margin.off_width;
    geo->height = (view->base.min_height ? view->base.min_height : VIEW_MIN_HEIGHT) +
                  view->base.margin.off_height;
    geo->x = usable->x;
    geo->y = usable->y;
    if (tile == KYWC_TILE_LEFT) {
        geo->height = usable->height;
    } else if (tile == KYWC_TILE_RIGHT) {
        geo->height = usable->height;
        geo->x = usable->x + usable->width - geo->width;
    } else if (tile == KYWC_TILE_TOP) {
        geo->width = usable->width;
    } else if (tile == KYWC_TILE_BOTTOM) {
        geo->width = usable->width;
        geo->y = usable->y + usable->height - geo->height;
    } else if (tile == KYWC_TILE_TOP_RIGHT) {
        geo->x = usable->x + usable->width - geo->width;
    } else if (tile == KYWC_TILE_BOTTOM_LEFT) {
        geo->y = usable->y + usable->height - geo->height;
    } else if (tile == KYWC_TILE_BOTTOM_RIGHT) {
        geo->x = usable->x + usable->width - geo->width;
        geo->y = usable->y + usable->height - geo->height;
    }
}

static bool item_proxy_is_suit(struct item_proxy *target, struct tiled_place *place, int tile)
{
    struct kywc_box target_box;
    get_view_geometry(target->item->view, &target_box);
    struct output *output = output_from_kywc_output(place->tiled_output->kywc_output);
    struct kywc_box *usable = &output->usable_area;
    struct kywc_box extend_box = {
        .x = usable->x - EXTRA_RANGE,
        .y = usable->y - EXTRA_RANGE,
        .width = usable->width + EXTRA_RANGE * 2,
        .height = usable->height + EXTRA_RANGE * 2,
    };
    if (!kywc_box_contains_point(&extend_box, target_box.x, target_box.y) ||
        !kywc_box_contains_point(&extend_box, target_box.x + target_box.width - 1,
                                 target_box.y + target_box.height - 1)) {
        return false;
    }

    if (place->operating_view) {
        struct item_proxy *operating = item_proxy_from_place(place->operating_view, place);
        struct kywc_box operating_box = { 0 };
        if (operating && operating->layer) {
            get_view_geometry(place->operating_view, &operating_box);
        } else {
            get_min_view_geometry(place->operating_view, &operating_box, usable, tile);
        }
        if (kywc_box_intersects(&target_box, &operating_box)) {
            return false;
        }
    }

    struct item_proxy *proxy;
    wl_list_for_each(proxy, &place->item_proxys, place_link) {
        if (proxy == target) {
            break;
        }

        if (place->operating_view && proxy->item->view == place->operating_view) {
            continue;
        }

        struct kywc_box item_box = { 0 };
        get_view_geometry(proxy->item->view, &item_box);
        kywc_box_adjust(&item_box, EXTRA_RANGE, EXTRA_RANGE, -2 * EXTRA_RANGE, -2 * EXTRA_RANGE);
        if (kywc_box_intersects(&item_box, &target_box)) {
            return false;
        }
    }

    return true;
}

static struct item_proxy *item_proxy_from_place_layers(struct tiled_place *place, int slot)
{
    struct tiled_layer *layer;
    wl_list_for_each(layer, &place->layers, link) {
        if (layer->proxy[slot]) {
            return layer->proxy[slot];
        }
    }
    return NULL;
}

static void item_proxy_remove_layer(struct item_proxy *proxy);

static void layer_set_slot_proxy(struct tiled_layer *layer, struct item_proxy *proxy, int slot)
{
    if (!proxy) {
        return;
    }

    if (!layer->on_layers) {
        layer->proxy[slot] = proxy;
        return;
    }

    item_proxy_remove_layer(proxy);
    layer->proxy[slot] = proxy;
    proxy->layer = layer;
}

static void tiled_layer_slot_update(struct tiled_layer *layer, int slot, int tile)
{
    layer->proxy[slot] = NULL;
    struct item_proxy *proxy = item_proxy_from_place_layers(layer->place, slot);
    if (!proxy) {
        return;
    }

    if (item_proxy_is_suit(proxy, layer->place, tile)) {
        layer_set_slot_proxy(layer, proxy, slot);
    }
}

static void layer_update_related_slots(struct tiled_layer *layer, int tile)
{
    if (tile == KYWC_TILE_LEFT) {
        tiled_layer_slot_update(layer, SLOT_TYPE_TOP_RIGHT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_BOTTOM_RIGHT, tile);
    } else if (tile == KYWC_TILE_RIGHT) {
        tiled_layer_slot_update(layer, SLOT_TYPE_TOP_LEFT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_BOTTOM_LEFT, tile);
    } else if (tile == KYWC_TILE_TOP) {
        tiled_layer_slot_update(layer, SLOT_TYPE_BOTTOM_LEFT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_BOTTOM_RIGHT, tile);
    } else if (tile == KYWC_TILE_BOTTOM) {
        tiled_layer_slot_update(layer, SLOT_TYPE_TOP_LEFT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_TOP_RIGHT, tile);
    } else if (tile == KYWC_TILE_TOP_LEFT) {
        tiled_layer_slot_update(layer, SLOT_TYPE_TOP_RIGHT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_BOTTOM_LEFT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_BOTTOM_RIGHT, tile);
    } else if (tile == KYWC_TILE_TOP_RIGHT) {
        tiled_layer_slot_update(layer, SLOT_TYPE_TOP_LEFT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_BOTTOM_LEFT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_BOTTOM_RIGHT, tile);
    } else if (tile == KYWC_TILE_BOTTOM_LEFT) {
        tiled_layer_slot_update(layer, SLOT_TYPE_TOP_LEFT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_TOP_RIGHT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_BOTTOM_RIGHT, tile);
    } else if (tile == KYWC_TILE_BOTTOM_RIGHT) {
        tiled_layer_slot_update(layer, SLOT_TYPE_TOP_LEFT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_TOP_RIGHT, tile);
        tiled_layer_slot_update(layer, SLOT_TYPE_BOTTOM_LEFT, tile);
    }
}

static void set_place_operating_view(struct tiled_place *place, struct view *view)
{
    place->operating_view = view;
}

void form_virtual_layer_by_tile(struct view *view, struct tiled_place *place,
                                struct tiled_layer *layer, enum kywc_tile tile)
{
    struct item_proxy *proxy = item_proxy_from_place(view, place);
    if (proxy && proxy->layer) {
        memcpy(layer, proxy->layer, sizeof(struct tiled_layer));
    }

    set_place_operating_view(place, view);
    layer->place = place;
    layer->on_layers = false;
    layer_update_related_slots(layer, tile);
}

static struct tiled_layer *tiled_layer_create(struct item_proxy *proxy)
{
    if (proxy->layer) {
        return proxy->layer;
    }

    struct tiled_layer *layer = calloc(1, sizeof(*layer));
    if (!layer) {
        return NULL;
    }

    wl_list_insert(&proxy->place->layers, &layer->link);

    proxy->layer = layer;
    layer->on_layers = true;
    layer->place = proxy->place;
    set_place_operating_view(proxy->place, proxy->item->view);

    return layer;
}

static void tiled_layer_update(struct tiled_layer *layer, struct item_proxy *proxy)
{
    if (!layer) {
        return;
    }

    if (proxy->item->view->base.tiled == KYWC_TILE_LEFT) {
        layer->proxy[SLOT_TYPE_TOP_LEFT] = proxy;
        layer->proxy[SLOT_TYPE_BOTTOM_LEFT] = proxy;
    } else if (proxy->item->view->base.tiled == KYWC_TILE_RIGHT) {
        layer->proxy[SLOT_TYPE_TOP_RIGHT] = proxy;
        layer->proxy[SLOT_TYPE_BOTTOM_RIGHT] = proxy;
    } else if (proxy->item->view->base.tiled == KYWC_TILE_TOP) {
        layer->proxy[SLOT_TYPE_TOP_LEFT] = proxy;
        layer->proxy[SLOT_TYPE_TOP_RIGHT] = proxy;
    } else if (proxy->item->view->base.tiled == KYWC_TILE_BOTTOM) {
        layer->proxy[SLOT_TYPE_BOTTOM_LEFT] = proxy;
        layer->proxy[SLOT_TYPE_BOTTOM_RIGHT] = proxy;
    } else if (proxy->item->view->base.tiled == KYWC_TILE_TOP_LEFT) {
        layer->proxy[SLOT_TYPE_TOP_LEFT] = proxy;
    } else if (proxy->item->view->base.tiled == KYWC_TILE_TOP_RIGHT) {
        layer->proxy[SLOT_TYPE_TOP_RIGHT] = proxy;
    } else if (proxy->item->view->base.tiled == KYWC_TILE_BOTTOM_LEFT) {
        layer->proxy[SLOT_TYPE_BOTTOM_LEFT] = proxy;
    } else if (proxy->item->view->base.tiled == KYWC_TILE_BOTTOM_RIGHT) {
        layer->proxy[SLOT_TYPE_BOTTOM_RIGHT] = proxy;
    }

    layer_update_related_slots(layer, proxy->item->view->base.tiled);
}

bool tiled_layer_is_empty(struct tiled_layer *layer)
{
    for (int i = 0; i < SLOT_TYPE_COUNT; i++) {
        if (layer->proxy[i]) {
            return false;
        }
    }
    return true;
}

static void tiled_layer_destroy(struct tiled_layer *layer)
{
    wl_list_remove(&layer->link);
    free(layer);
}

static void item_proxy_remove_layer(struct item_proxy *proxy)
{
    if (!proxy->layer) {
        return;
    }

    struct tiled_layer *layer = proxy->layer;
    proxy->layer = NULL;

    for (int i = 0; i < SLOT_TYPE_COUNT; i++) {
        if (layer->proxy[i] && layer->proxy[i] == proxy) {
            layer->proxy[i] = NULL;
        }
    }
    if (tiled_layer_is_empty(layer)) {
        tiled_layer_destroy(layer);
    }
}

static void remove_all_item_proxy(struct tiled_item *item)
{
    struct item_proxy *proxy;
    wl_list_for_each(proxy, &item->item_proxys, item_link) {
        item_proxy_remove_layer(proxy);
    }
}

static void update_all_item_proxy(struct tiled_item *item)
{
    struct item_proxy *proxy;
    wl_list_for_each(proxy, &item->item_proxys, item_link) {
        struct tiled_layer *layer = tiled_layer_create(proxy);
        tiled_layer_update(layer, proxy);
    }
}

bool view_need_tile_managerd(struct view *view)
{
    return view->base.tiled && view->base.tiled <= KYWC_TILE_BOTTOM_RIGHT;
}

static void item_proxy_destroy(struct item_proxy *proxy);

static void tiled_item_destroy(struct tiled_item *item)
{
    struct item_proxy *proxy, *tmp;
    wl_list_for_each_safe(proxy, tmp, &item->item_proxys, item_link) {
        item_proxy_destroy(proxy);
    }

    wl_list_remove(&item->view_tile.link);
    wl_list_remove(&item->view_unmap.link);
    wl_list_remove(&item->view_minimize.link);
    wl_list_remove(&item->view_fullscreen.link);
    wl_list_remove(&item->view_activate.link);
    wl_list_remove(&item->view_output.link);
    wl_list_remove(&item->view_size.link);
    wl_list_remove(&item->view_position.link);
    wl_list_remove(&item->workspace_enter.link);
    wl_list_remove(&item->workspace_leave.link);
    free(item);
}

static void handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct tiled_item *item = wl_container_of(listener, item, view_unmap);

    tiled_item_destroy(item);
}

static void handle_view_minimize(struct wl_listener *listener, void *data)
{
    struct tiled_item *item = wl_container_of(listener, item, view_minimize);

    if (!view_need_tile_managerd(item->view)) {
        return;
    }

    remove_all_item_proxy(item);
    if (!item->view->base.minimized) {
        update_all_item_proxy(item);
    }
}

static void handle_view_fullscreen(struct wl_listener *listener, void *data)
{
    struct tiled_item *item = wl_container_of(listener, item, view_fullscreen);

    if (!view_need_tile_managerd(item->view)) {
        return;
    }

    if (item->view->base.fullscreen) {
        remove_all_item_proxy(item);
    } else {
        update_all_item_proxy(item);
    }
}

static void handle_view_tile(struct wl_listener *listener, void *data)
{
    struct tiled_item *item = wl_container_of(listener, item, view_tile);

    remove_all_item_proxy(item);
    scene_node_set_enabled(&item->view->tree->node, true);

    if (!view_need_tile_managerd(item->view)) {
        struct tiled_output *tiled_output = tiled_output_from_kywc_output(item->view->output);
        tiled_output->in_assist = false;
        return;
    }
    item_ratio_update(item, RATIO_ALL);

    update_all_item_proxy(item);
    view_try_show_assist(item->view);
}

static void handle_view_activate(struct wl_listener *listener, void *data)
{
    struct tiled_item *item = wl_container_of(listener, item, view_activate);
    if (!item->view->base.activated) {
        return;
    }

    struct item_proxy *proxy;
    wl_list_for_each(proxy, &item->item_proxys, item_link) {
        wl_list_remove(&proxy->place_link);
        wl_list_insert(&proxy->place->item_proxys, &proxy->place_link);
    }

    if (view_need_tile_managerd(item->view)) {
        remove_all_item_proxy(item);
        update_all_item_proxy(item);
    }
}

static void handle_view_output(struct wl_listener *listener, void *data)
{
    struct tiled_item *item = wl_container_of(listener, item, view_output);
    struct tiled_output *tiled_output = tiled_output_from_kywc_output(item->view->output);

    struct item_proxy *proxy;
    wl_list_for_each(proxy, &item->item_proxys, item_link) {
        struct tiled_place *new = get_tiled_place(tiled_output, proxy->place->workspace);
        if (!new) {
            continue;
        }
        wl_list_remove(&proxy->place_link);
        wl_list_init(&proxy->place_link);
        wl_list_insert(&new->item_proxys, &proxy->place_link);
        proxy->place = new;
    }

    remove_all_item_proxy(item);
    item->kywc_output = item->view->output;

    if (view_need_tile_managerd(item->view)) {
        update_all_item_proxy(item);
    }
}

static void item_ratio_update(struct tiled_item *item, enum ratio_state ratio)
{
    struct kywc_view *view = &item->view->base;
    struct kywc_box *geo = &view->geometry;
    struct output *output = output_from_kywc_output(item->view->output);
    struct kywc_box *usable = &output->usable_area;

    if (ratio & RATIO_POSITION) {
        if (usable->width > 0) {
            item->ratio_x = (float)(geo->x - view->margin.off_x - usable->x) / usable->width;
        }
        if (usable->height > 0) {
            item->ratio_y = (float)(geo->y - view->margin.off_y - usable->y) / usable->height;
        }
    }
    if (ratio & RATIO_SIZE) {
        if (usable->width > 0) {
            item->ratio_w = (float)(geo->width + view->margin.off_width) / usable->width;
        }
        if (usable->height > 0) {
            item->ratio_h = (float)(geo->height + view->margin.off_height) / usable->height;
        }
    }
}

static void handle_view_size(struct wl_listener *listener, void *data)
{
    struct tiled_item *item = wl_container_of(listener, item, view_size);
    if (!view_need_tile_managerd(item->view)) {
        return;
    }

    if (item->kywc_output != item->view->output || item->skip_size_update) {
        item->skip_size_update = false;
        return;
    }

    item_ratio_update(item, RATIO_SIZE);

    view_try_show_assist(item->view);
}

static void handle_view_position(struct wl_listener *listener, void *data)
{
    struct tiled_item *item = wl_container_of(listener, item, view_position);
    if (!view_need_tile_managerd(item->view)) {
        return;
    }

    if (item->skip_pos_update) {
        item->skip_pos_update = false;
        return;
    }

    item_ratio_update(item, item->view->pending.configure_action & VIEW_ACTION_RESIZE
                                ? RATIO_POSITION
                                : RATIO_ALL);

    view_try_show_assist(item->view);
}

static void item_proxy_create(struct tiled_item *item, struct tiled_place *place)
{
    struct item_proxy *proxy = calloc(1, sizeof(*proxy));
    if (!proxy) {
        return;
    }

    proxy->place = place;
    proxy->item = item;
    wl_list_insert(&place->item_proxys, &proxy->place_link);
    wl_list_insert(&item->item_proxys, &proxy->item_link);

    if (view_need_tile_managerd(item->view)) {
        struct tiled_layer *layer = tiled_layer_create(proxy);
        tiled_layer_update(layer, proxy);
    }
}

static void item_proxy_destroy(struct item_proxy *proxy)
{
    item_proxy_remove_layer(proxy);
    wl_list_remove(&proxy->place_link);
    wl_list_remove(&proxy->item_link);
    free(proxy);
}

static void handle_view_workspace_enter(struct wl_listener *listener, void *data)
{
    struct tiled_item *item = wl_container_of(listener, item, workspace_enter);
    struct workspace *workspace = data;
    struct tiled_output *tiled_output = tiled_output_from_kywc_output(item->view->output);
    struct tiled_place *place = get_tiled_place(tiled_output, workspace);
    if (place && !item_proxy_from_place(item->view, place)) {
        item_proxy_create(item, place);
    }
}

static void handle_view_workspace_leave(struct wl_listener *listener, void *data)
{
    struct tiled_item *item = wl_container_of(listener, item, workspace_leave);
    struct workspace *workspace = data;
    struct tiled_output *tiled_output = tiled_output_from_kywc_output(item->view->output);
    struct tiled_place *place = get_tiled_place(tiled_output, workspace);
    struct item_proxy *proxy = item_proxy_from_place(item->view, place);
    if (proxy) {
        item_proxy_destroy(proxy);
    }
}

static void handle_new_mapped_view(struct wl_listener *listener, void *data)
{
    struct kywc_view *kywc_view = data;
    struct view *view = view_from_kywc_view(kywc_view);
    if (!view->current_proxy) {
        return;
    }

    struct tiled_item *item = calloc(1, sizeof(*item));
    if (!item) {
        return;
    }

    wl_list_init(&item->item_proxys);

    item->view = view;
    item->kywc_output = view->output;
    item->view_unmap.notify = handle_view_unmap;
    wl_signal_add(&kywc_view->events.unmap, &item->view_unmap);
    item->view_minimize.notify = handle_view_minimize;
    wl_signal_add(&kywc_view->events.minimize, &item->view_minimize);
    item->view_fullscreen.notify = handle_view_fullscreen;
    wl_signal_add(&kywc_view->events.fullscreen, &item->view_fullscreen);
    item->view_tile.notify = handle_view_tile;
    wl_signal_add(&kywc_view->events.tile, &item->view_tile);
    item->view_activate.notify = handle_view_activate;
    wl_signal_add(&kywc_view->events.activate, &item->view_activate);
    item->view_output.notify = handle_view_output;
    wl_signal_add(&view->events.output, &item->view_output);
    item->view_size.notify = handle_view_size;
    wl_signal_add(&kywc_view->events.size, &item->view_size);
    item->view_position.notify = handle_view_position;
    wl_signal_add(&view->events.position, &item->view_position);
    item->workspace_enter.notify = handle_view_workspace_enter;
    wl_signal_add(&view->events.workspace_enter, &item->workspace_enter);
    item->workspace_leave.notify = handle_view_workspace_leave;
    wl_signal_add(&view->events.workspace_leave, &item->workspace_leave);

    struct tiled_output *tiled_output = tiled_output_from_kywc_output(item->view->output);
    struct tiled_place *place = get_tiled_place(tiled_output, view->current_proxy->workspace);
    struct item_proxy *proxy = item_proxy_from_place(item->view, place);
    if (place && !proxy) {
        item_proxy_create(item, place);
    }
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->new_mapped_view.link);
    wl_list_remove(&manager->server_destroy.link);
    wl_list_remove(&manager->new_output.link);

    struct tiled_output *tiled_output, *tmp;
    wl_list_for_each_safe(tiled_output, tmp, &manager->tiled_outputs, link) {
        wl_list_remove(&tiled_output->link);
        if (tiled_output->linkage) {
            tile_linkage_destroy(tiled_output->linkage);
            tiled_output->linkage = NULL;
        }
        free(tiled_output->output_name);
        free(tiled_output);
    }
    free(manager);
    manager = NULL;
}

static void handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct tiled_output *tiled_output = wl_container_of(listener, tiled_output, output_destroy);
    wl_list_remove(&tiled_output->output_destroy.link);
    tiled_output->kywc_output = NULL;
    if (tiled_output->linkage) {
        tile_linkage_destroy(tiled_output->linkage);
        tiled_output->linkage = NULL;
    }
}

static void handle_new_output(struct wl_listener *listener, void *data)
{
    struct kywc_output *kywc_output = data;
    struct tiled_output *tiled_output = tiled_output_from_kywc_output(kywc_output);
    if (!tiled_output) {
        if (!(tiled_output = calloc(1, sizeof(*tiled_output)))) {
            return;
        }
        wl_list_init(&tiled_output->tiled_places);
        wl_list_insert(&manager->tiled_outputs, &tiled_output->link);
        tiled_output->output_name = strdup(kywc_output->name);
    }

    tiled_output->kywc_output = kywc_output;
    tiled_output->output_destroy.notify = handle_output_destroy;
    wl_signal_add(&kywc_output->events.destroy, &tiled_output->output_destroy);
}

bool tile_manager_create(struct view_manager *view_manager)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    wl_list_init(&manager->tiled_outputs);

    view_manager->impl.fix_geometry = tile_view_fix_geometry;
    view_manager->impl.get_tiled_geometry = tile_get_view_geometry;
    view_manager->impl.set_tiled = tile_view_apply_tiling;
    view_manager->impl.show_tile_assist = tile_view_tiled_assist;

    manager->new_mapped_view.notify = handle_new_mapped_view;
    kywc_view_add_new_mapped_listener(&manager->new_mapped_view);
    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(view_manager->server, &manager->server_destroy);
    manager->new_output.notify = handle_new_output;
    kywc_output_add_new_listener(&manager->new_output);

    return true;
}

bool tile_manager_check_resize_edges(struct view *view, uint32_t resize_edges)
{
    if (!view || !view->base.tiled) {
        return true;
    }

    bool resizable = false;
    if (view->base.tiled == KYWC_TILE_TOP && resize_edges == KYWC_EDGE_BOTTOM) {
        resizable = true;
    } else if (view->base.tiled == KYWC_TILE_BOTTOM && resize_edges == KYWC_EDGE_TOP) {
        resizable = true;
    } else if (view->base.tiled == KYWC_TILE_LEFT && resize_edges == KYWC_EDGE_RIGHT) {
        resizable = true;
    } else if (view->base.tiled == KYWC_TILE_RIGHT && resize_edges == KYWC_EDGE_LEFT) {
        resizable = true;
    } else if (view->base.tiled == KYWC_TILE_TOP_LEFT &&
               (resize_edges == KYWC_EDGE_BOTTOM || resize_edges == KYWC_EDGE_RIGHT ||
                resize_edges == (KYWC_EDGE_BOTTOM | KYWC_EDGE_RIGHT))) {
        resizable = true;
    } else if (view->base.tiled == KYWC_TILE_TOP_RIGHT &&
               (resize_edges == KYWC_EDGE_BOTTOM || resize_edges == KYWC_EDGE_LEFT ||
                resize_edges == (KYWC_EDGE_BOTTOM | KYWC_EDGE_LEFT))) {
        resizable = true;
    } else if (view->base.tiled == KYWC_TILE_BOTTOM_LEFT &&
               (resize_edges == KYWC_EDGE_TOP || resize_edges == KYWC_EDGE_RIGHT ||
                resize_edges == (KYWC_EDGE_TOP | KYWC_EDGE_RIGHT))) {
        resizable = true;
    } else if (view->base.tiled == KYWC_TILE_BOTTOM_RIGHT &&
               (resize_edges == KYWC_EDGE_TOP || resize_edges == KYWC_EDGE_LEFT ||
                resize_edges == (KYWC_EDGE_TOP | KYWC_EDGE_LEFT))) {
        resizable = true;
    }

    return resizable;
}
