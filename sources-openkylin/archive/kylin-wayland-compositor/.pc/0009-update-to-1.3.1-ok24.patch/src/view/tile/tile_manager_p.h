// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _TILE_MANAGER_P_H_
#define _TILE_MANAGER_P_H_

#include "../view_p.h"

#define VIEW_MIN_WIDTH 200
#define VIEW_MIN_HEIGHT 100

enum slot_type {
    SLOT_TYPE_TOP_LEFT = 0,
    SLOT_TYPE_TOP_RIGHT,
    SLOT_TYPE_BOTTOM_LEFT,
    SLOT_TYPE_BOTTOM_RIGHT,
    SLOT_TYPE_COUNT,
};

struct item_proxy {
    struct wl_list place_link;
    struct wl_list item_link;

    struct tiled_item *item;
    struct tiled_layer *layer;
    struct tiled_place *place;
};

struct tiled_item {
    struct wl_list item_proxys;
    struct kywc_output *kywc_output;

    float ratio_x;
    float ratio_y;
    float ratio_w;
    float ratio_h;

    struct kywc_box tiled_geo;

    struct view *view;
    struct wl_listener view_unmap;
    struct wl_listener view_minimize;
    struct wl_listener view_fullscreen;
    struct wl_listener view_tile;
    struct wl_listener view_activate;
    struct wl_listener view_output;
    struct wl_listener view_size;
    struct wl_listener view_position;
    struct wl_listener workspace_enter;
    struct wl_listener workspace_leave;
};

struct tiled_layer {
    struct wl_list link;

    // In the layers table of the place
    bool on_layers;
    struct tiled_place *place;
    struct item_proxy *proxy[SLOT_TYPE_COUNT];
};

struct tiled_place {
    struct wl_list link;
    struct wl_list item_proxys;
    struct wl_list layers;

    struct tiled_output *tiled_output;
    struct workspace *workspace;
    struct view *operating_view;

    struct wl_listener workspace_destroy;
};

struct tiled_output {
    struct wl_list link;
    struct wl_list tiled_places;

    struct tile_linkage *linkage;
    struct seat *assist_seat;
    bool in_assist;

    char *output_name;
    struct kywc_output *kywc_output;
    struct wl_listener output_destroy;
};

void tile_calculate_view_geometry(struct tiled_layer *layer, struct output *output,
                                  struct kywc_box *geo, enum kywc_tile tile);

struct tiled_output *tiled_output_from_kywc_output(struct kywc_output *kywc_output);

struct tiled_place *get_tiled_place(struct tiled_output *tiled_output, struct workspace *workspace);

struct item_proxy *item_proxy_from_place(struct view *view, struct tiled_place *place);

bool tiled_layer_is_empty(struct tiled_layer *layer);

bool view_need_tile_managerd(struct view *view);

void form_virtual_layer_by_tile(struct view *view, struct tiled_place *place,
                                struct tiled_layer *layer, enum kywc_tile tile);

void view_begin_tile_assist(struct view *view, struct seat *seat, struct output *output,
                            struct tiled_layer *tiled_layer);

void tile_linkage_destroy(struct tile_linkage *linkage);

#endif /* _TILE_MANAGER_P_H_ */
