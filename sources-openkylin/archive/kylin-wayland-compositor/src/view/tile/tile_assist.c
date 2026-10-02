// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#include "effect/snapshot.h"
#include "input/cursor.h"
#include "input/seat.h"
#include "output.h"
#include "painter.h"
#include "scene/decoration.h"
#include "theme.h"
#include "tile_manager_p.h"
#include "util/color.h"
#include "widget/scaled_buffer.h"
#include "widget/widget.h"

#define GAP 16
#define ITEM_HEIGHT 40
#define ITEM_GAP 8
#define SELECT_BORDER_WIDTH 2
#define RESIZE_BORDER 13

enum split_type {
    TYPE_HALF_LEFT_OR_RIGHT = 0,
    TYPE_HALF_TOP_OR_BOTTOM,
    TYPE_THREE_LEFT,
    TYPE_THREE_RIGHT,
    TYPE_THREE_TOP,
    TYPE_THREE_BOTTOM,
    TYPE_QUARTER,
};

enum split_state {
    HALF_SCREEN = 2,
    THREE_SCREEN,
    QUARTER_SCREEN,
};

enum button_state {
    BUTTON_STATE_NONE = 0,
    BUTTON_STATE_HOVER,
    BUTTON_STATE_CLICKED,
};

enum assist_part {
    ASSIST_ROOT = 0,
    ASSIST_FRAME_RECT,
    ASSIST_BUTTON_CLOSE,
    ASSIST_TITLE_ICON,
    ASSIST_TITLE_TEXT,
    ASSIST_THUMBNAIL,
    ASSIST_PART_COUNT,
};

struct item_part {
    struct item *item;
    struct scene_node *node;
    enum assist_part part;
    float scale;
};

struct item {
    struct wl_list link;
    struct scene_tree *tree;

    struct widget *title_text;
    struct item_part part[ASSIST_PART_COUNT];

    struct scene_node *selection;

    struct view *view;
    struct wl_listener view_unmap;
    struct snapshot *snapshot;
    int32_t snapshot_width, snapshot_height;
    int32_t max_text_width, text_height;
};

struct assist {
    struct view *view;
    struct wl_listener view_unmap;

    struct scene_tree *tree;
    struct scene_rect *assist_area, *background;

    struct kywc_box box;
    struct kywc_box full_area;

    enum kywc_tile tiled;
    int32_t index_first, current;
    int32_t total, row, col;
};

static struct tile_assist_manager {
    struct wl_list items;
    struct scene_tree *tree;

    struct workspace *workspace;
    struct seat *seat;
    struct output *output;
    struct tile_assist_color *color;

    struct assist assist[SLOT_TYPE_COUNT];
    struct assist *item_page;
    struct item *selected;
    struct view *active_view;
    struct tiled_layer tiled_layer;

    struct seat_pointer_grab pointer_grab;
    struct seat_keyboard_grab keyboard_grab;
    struct seat_touch_grab touch_grab;

    struct wl_listener new_mapped_view;
    struct wl_listener configured_output;
    struct wl_listener seat_destroy;
    struct wl_listener show_desktop;
    struct wl_listener workspace_activate;

    enum split_state state;
    enum split_type type;

    int32_t total_items;
    int32_t fixed_width;  // (usable_width - 160) / 4
    int32_t fixed_height; // usable_height / 22 * 5

    /* listener highlight view */
    struct wl_listener highlight;
} *manager = NULL;

static void title_text_update(struct item *item)
{
    struct theme *theme = theme_manager_get_theme();
    widget_set_text(item->title_text, item->view->base.title, JUSTIFY_CENTER, TEXT_ATTR_NONE);

    widget_set_font(item->title_text, theme->font_name, theme->font_size);
    widget_set_front_color(item->title_text, theme->active_text_color);

    widget_set_max_size(item->title_text, item->max_text_width, ITEM_HEIGHT);
    widget_set_auto_resize(item->title_text, AUTO_RESIZE_ONLY);

    widget_set_enabled(item->title_text, true);
    widget_update(item->title_text, true);

    int32_t text_width, text_height;
    widget_get_size(item->title_text, &text_width, &text_height);

    item->text_height = text_height;
}

static void icon_buffer_update(struct item_part *part, float scale)
{
    struct wlr_buffer *buf = view_get_icon_buffer(part->item->view, scale);
    if (!buf) {
        return;
    }

    struct scene_buffer *icon_buffer = scene_buffer_from_node(part->node);
    if (icon_buffer->buffer != buf) {
        scene_buffer_set_buffer(icon_buffer, buf);
    }
    if (icon_buffer->buffer != buf) {
        return;
    }

    struct theme *theme = theme_manager_get_theme();
    scene_buffer_set_dest_size(icon_buffer, theme->icon_size, theme->icon_size);
}

static void set_close_buffer(struct item_part *part, enum button_state state)
{
    enum theme_button_type type = THEME_BUTTON_TYPE_CLOSE;
    struct theme *theme = theme_manager_get_theme();
    /* get actual type by current state */
    type += state * 4;

    struct wlr_fbox src;
    struct wlr_buffer *buf = theme_button_buffer_load(theme, part->scale, type, &src, true);
    struct scene_buffer *buffer = scene_buffer_from_node(part->node);
    if (buffer->buffer != buf) {
        scene_buffer_set_buffer(buffer, buf);
    }
    /* shortcut here if set_buffer triggered scaled buffer update */
    if (buffer->buffer != buf) {
        return;
    }

    scene_buffer_set_dest_size(buffer, theme->button_width, theme->button_width);
    scene_buffer_set_source_box(buffer, &src);
}

static void item_buffer_update(struct scene_buffer *buffer, float scale, void *data)
{
    struct item_part *item_part = data;
    item_part->scale = scale;
    if (item_part->part == ASSIST_BUTTON_CLOSE) {
        set_close_buffer(item_part, BUTTON_STATE_NONE);
    } else if (item_part->part == ASSIST_TITLE_ICON) {
        icon_buffer_update(item_part, scale);
    }
}

static void item_buffer_destroy(struct scene_buffer *buffer, void *data)
{
    /* buffers are destroyed in theme */
}

static void assist_item_show(void)
{
    struct item *item;
    wl_list_for_each(item, &manager->items, link) {
        if (!item->view->base.minimized) {
            scene_node_set_enabled(&item->view->tree->node, false);
        }
        if (!item->tree) {
            continue;
        }

        scene_node_set_enabled(item->selection, false);
        scene_node_set_enabled(&item->tree->node, false);
    }

    int32_t i = 0, col = 0;
    int32_t node_x = 0;
    struct assist *assist = manager->item_page;
    int32_t node_y = (assist->box.height % manager->fixed_height) / 2;

    wl_list_for_each(item, &manager->items, link) {
        if (!item->tree) {
            continue;
        }

        if (i < assist->index_first) {
            i++;
            continue;
        }

        if (i == assist->index_first + assist->total) {
            break;
        }

        if (col != 0 && col % assist->col == 0) {
            node_y += manager->fixed_height;
        }

        node_x = (assist->box.width % manager->fixed_width) / 2 +
                 (col % assist->col) * manager->fixed_width;
        col++;

        scene_node_set_position(&item->tree->node, node_x, node_y);
        scene_node_set_enabled(&item->tree->node, true);

        if (i == assist->current) {
            manager->selected = item;
            scene_node_set_enabled(item->selection, true);
        }

        i++;
    }
}

static void item_destroy(struct item *item)
{
    wl_list_remove(&item->link);
    wl_list_remove(&item->view_unmap.link);

    /* snapshot destroy */
    if (item->snapshot) {
        snapshot_destroy(item->snapshot);
    }

    if (item->tree) {
        manager->total_items--;
        scene_node_set_enabled(&item->tree->node, false);
        scene_node_destroy(&item->tree->node);
    }

    free(item);
}

static void tile_assist_done(void);

static void handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct item *item = wl_container_of(listener, item, view_unmap);
    item_destroy(item);
    if (manager->total_items == 0) {
        tile_assist_done();
        return;
    }
    assist_item_show();
}

static struct item *item_create(struct view *view, bool flip)
{
    struct item *item = calloc(1, sizeof(*item));
    if (!item) {
        return NULL;
    }

    /* The newly opened view is inserted behind the linked list. */
    wl_list_insert(flip ? &manager->items : manager->items.prev, &item->link);

    item->view = view;
    item->tree = NULL;
    item->view_unmap.notify = handle_view_unmap;
    wl_signal_add(&item->view->base.events.unmap, &item->view_unmap);

    return item;
}

static void tile_assist_done(void)
{
    struct item *item, *item_tmp;
    wl_list_for_each_safe(item, item_tmp, &manager->items, link) {
        if (!item->view->base.minimized && !view_manager_get_highlight()) {
            scene_node_set_enabled(&item->view->tree->node, true);
        }
        item_destroy(item);
    }

    for (int32_t i = 0; i < SLOT_TYPE_COUNT; i++) {
        struct assist *assist = &manager->assist[i];
        if (assist->view) {
            wl_list_remove(&assist->view_unmap.link);
        }
        if (assist->tree) {
            scene_node_destroy(&assist->tree->node);
            manager->assist[i].tree = NULL;
        }

        assist->view = NULL;
    }

    if (manager->active_view && manager->active_view->base.mapped) {
        kywc_view_activate(&manager->active_view->base);
        view_set_focus(manager->active_view, manager->active_view->focused_seat);
    }

    wl_list_remove(&manager->new_mapped_view.link);
    wl_list_remove(&manager->configured_output.link);
    wl_list_remove(&manager->seat_destroy.link);
    wl_list_remove(&manager->show_desktop.link);
    wl_list_remove(&manager->workspace_activate.link);
    wl_list_remove(&manager->highlight.link);

    seat_end_pointer_grab(manager->seat, &manager->pointer_grab);
    seat_end_keyboard_grab(manager->seat, &manager->keyboard_grab);
    seat_end_touch_grab(manager->seat, &manager->touch_grab);

    scene_node_destroy(&manager->tree->node);
    free(manager);
    manager = NULL;
}

static void assist_area_update(struct assist *assist, struct view *top_left, struct view *top_right,
                               struct view *bottom_left, struct view *bottom_right)
{
    if (assist->view) {
        return;
    }

    enum kywc_tile tiled = assist->tiled;
    struct kywc_box *usable = &manager->output->usable_area;
    struct kywc_box *full_area = &assist->full_area;
    struct kywc_box back_box = { 0 };
    int remain_w = usable->width - full_area->width;
    int remain_h = usable->height - full_area->height;

    bool w_aligned = false, h_aligned = false;
    if (tiled == KYWC_TILE_LEFT || tiled == KYWC_TILE_RIGHT) {
        int x = tiled == KYWC_TILE_LEFT ? GAP : usable->width - full_area->width + GAP;
        assist->box.width = full_area->width - GAP * 2;
        assist->box.height = full_area->height - GAP * 2;
        assist->box.x = usable->x + x;
        assist->box.y = usable->y + GAP;

        back_box.width = full_area->width - RESIZE_BORDER;
        back_box.height = full_area->height;
        back_box.x = tiled == KYWC_TILE_LEFT ? -GAP : -(GAP - RESIZE_BORDER);
        back_box.y = -GAP;
    } else if (tiled == KYWC_TILE_TOP || tiled == KYWC_TILE_BOTTOM) {
        int y = tiled == KYWC_TILE_TOP ? GAP : usable->height - full_area->height + GAP;
        assist->box.width = full_area->width - GAP * 2;
        assist->box.height = full_area->height - GAP * 2;
        assist->box.x = usable->x + GAP;
        assist->box.y = usable->y + y;

        back_box.width = full_area->width;
        back_box.height = full_area->height - RESIZE_BORDER;
        back_box.x = -GAP;
        back_box.y = tiled == KYWC_TILE_TOP ? -GAP : -(GAP - RESIZE_BORDER);
    } else if (tiled == KYWC_TILE_TOP_LEFT) {
        if (!top_right && bottom_left) {
            w_aligned = true;
            if (bottom_right) {
                w_aligned =
                    bottom_left->base.geometry.height + bottom_left->base.margin.off_height >=
                    bottom_right->base.geometry.height + bottom_right->base.margin.off_height;
            }
        }
        if (top_right && !bottom_left) {
            h_aligned = true;
            if (bottom_right) {
                h_aligned = top_right->base.geometry.width + top_right->base.margin.off_width >=
                            bottom_right->base.geometry.width + bottom_right->base.margin.off_width;
            }
        }
        assist->box.width = w_aligned ? full_area->width - GAP : full_area->width - GAP * 2;
        assist->box.height = h_aligned ? full_area->height - GAP : full_area->height - GAP * 2;
        assist->box.x = usable->x + GAP;
        assist->box.y = usable->y + GAP;

        back_box.width = w_aligned ? full_area->width : full_area->width - RESIZE_BORDER;
        back_box.height = h_aligned ? full_area->height : full_area->height - RESIZE_BORDER;
        back_box.x = -GAP;
        back_box.y = -GAP;
    } else if (tiled == KYWC_TILE_TOP_RIGHT) {
        if (!top_left && bottom_right) {
            w_aligned = true;
            if (bottom_left) {
                w_aligned =
                    bottom_right->base.geometry.height + bottom_right->base.margin.off_height >
                    bottom_left->base.geometry.height + bottom_left->base.margin.off_height;
            }
        }
        if (!bottom_right && top_left) {
            h_aligned = true;
            if (bottom_left) {
                h_aligned = top_left->base.geometry.width + top_left->base.margin.off_width >=
                            bottom_left->base.geometry.width + bottom_left->base.margin.off_width;
            }
        }
        assist->box.width = w_aligned ? full_area->width - GAP : full_area->width - GAP * 2;
        assist->box.height = h_aligned ? full_area->height - GAP : full_area->height - 2 * GAP;
        assist->box.x = w_aligned ? usable->x + remain_w : usable->x + remain_w + GAP;
        assist->box.y = usable->y + GAP;

        back_box.width = w_aligned ? full_area->width : full_area->width - RESIZE_BORDER;
        back_box.height = h_aligned ? full_area->height : full_area->height - RESIZE_BORDER;
        back_box.x = w_aligned ? 0 : -(GAP - RESIZE_BORDER);
        back_box.y = -GAP;
    } else if (tiled == KYWC_TILE_BOTTOM_LEFT) {
        if (!bottom_right && top_left) {
            w_aligned = true;
            if (top_right) {
                w_aligned = top_left->base.geometry.height + top_left->base.margin.off_height >=
                            top_right->base.geometry.height + top_right->base.margin.off_height;
            }
        }
        if (!top_left && bottom_right) {
            h_aligned = true;
            if (top_right) {
                h_aligned =
                    bottom_right->base.geometry.width + bottom_right->base.margin.off_width >
                    top_right->base.geometry.width + top_right->base.margin.off_width;
            }
        }
        assist->box.width = w_aligned ? full_area->width - GAP : full_area->width - GAP * 2;
        assist->box.height = h_aligned ? full_area->height - GAP : full_area->height - GAP * 2;
        assist->box.x = usable->x + GAP;
        assist->box.y = h_aligned ? usable->y + remain_h : usable->y + remain_h + GAP;

        back_box.width = w_aligned ? full_area->width : full_area->width - RESIZE_BORDER;
        back_box.height = h_aligned ? full_area->height : full_area->height - RESIZE_BORDER;
        back_box.x = -GAP;
        back_box.y = h_aligned ? 0 : -(GAP - RESIZE_BORDER);
    } else if (tiled == KYWC_TILE_BOTTOM_RIGHT) {
        if (!bottom_left && top_right) {
            w_aligned = true;
            if (top_left) {
                w_aligned = top_right->base.geometry.height + top_right->base.margin.off_height >
                            top_left->base.geometry.height + top_left->base.margin.off_height;
            }
        }
        if (!top_right && bottom_left) {
            h_aligned = true;
            if (top_left) {
                h_aligned = bottom_left->base.geometry.width + bottom_left->base.margin.off_width >
                            top_left->base.geometry.width + top_left->base.margin.off_width;
            }
        }
        assist->box.width = w_aligned ? full_area->width - GAP : full_area->width - GAP * 2;
        assist->box.height = h_aligned ? full_area->height - GAP : full_area->height - GAP * 2;
        assist->box.x = w_aligned ? usable->x + remain_w : usable->x + remain_w + GAP;
        assist->box.y = h_aligned ? usable->y + remain_h : usable->y + remain_h + GAP;

        back_box.width = w_aligned ? full_area->width : full_area->width - RESIZE_BORDER;
        back_box.height = h_aligned ? full_area->height : full_area->height - RESIZE_BORDER;
        back_box.x = w_aligned ? 0 : -(GAP - RESIZE_BORDER);
        back_box.y = h_aligned ? 0 : -(GAP - RESIZE_BORDER);
    }

    scene_rect_set_size(assist->background, back_box.width, back_box.height);
    scene_rect_set_size(assist->assist_area, assist->box.width, assist->box.height);
    scene_node_set_position(&assist->tree->node, assist->box.x, assist->box.y);
    scene_node_set_position(&assist->background->node, back_box.x, back_box.y);

    assist->row = assist->box.height / manager->fixed_height;
    assist->col = assist->box.width / manager->fixed_width;
    assist->total = assist->row * assist->col;

    if (!assist->total) {
        scene_node_set_enabled(&assist->tree->node, false);
    }
}

static void item_page_assist_update(void)
{
    struct view *top_left = NULL;
    struct view *top_right = NULL;
    struct view *bottom_left = NULL;
    struct view *bottom_right = NULL;
    for (uint32_t i = 0; i < manager->state; i++) {
        if (!manager->assist[i].view) {
            continue;
        }
        enum kywc_tile tiled = manager->assist[i].view->base.tiled;
        if (tiled == KYWC_TILE_LEFT) {
            top_left = bottom_left = manager->assist[i].view;
        } else if (tiled == KYWC_TILE_RIGHT) {
            top_right = bottom_right = manager->assist[i].view;
        } else if (tiled == KYWC_TILE_TOP) {
            top_left = top_right = manager->assist[i].view;
        } else if (tiled == KYWC_TILE_BOTTOM) {
            bottom_left = bottom_right = manager->assist[i].view;
        } else if (tiled == KYWC_TILE_TOP_LEFT) {
            top_left = manager->assist[i].view;
        } else if (tiled == KYWC_TILE_TOP_RIGHT) {
            top_right = manager->assist[i].view;
        } else if (tiled == KYWC_TILE_BOTTOM_LEFT) {
            bottom_left = manager->assist[i].view;
        } else if (tiled == KYWC_TILE_BOTTOM_RIGHT) {
            bottom_right = manager->assist[i].view;
        }
    }

    for (uint32_t i = 0; i < manager->state; i++) {
        assist_area_update(&manager->assist[i], top_left, top_right, bottom_left, bottom_right);
    }

    for (uint32_t i = 0; i < manager->state; i++) {
        if (!manager->assist[i].view && manager->assist[i].total) {
            manager->item_page = &manager->assist[i];
            return;
        }
    }

    manager->item_page = NULL;
}

static void item_page_update(void)
{
    item_page_assist_update();

    if (!manager->item_page) {
        tile_assist_done();
        return;
    }

    struct item *item;
    wl_list_for_each(item, &manager->items, link) {
        if (!item->tree) {
            continue;
        }
        scene_node_reparent(&item->tree->node, manager->item_page->tree);
    }

    assist_item_show();
}

static void handle_tiled_view_unmap(struct wl_listener *listener, void *data)
{
    tile_assist_done();
}

static void assist_area_set_view(struct assist *assist, struct view *view)
{
    assist->view = view;
    assist->view_unmap.notify = handle_tiled_view_unmap;
    wl_signal_add(&assist->view->base.events.unmap, &assist->view_unmap);
    scene_node_set_enabled(&assist->tree->node, false);
}

static void item_view_set_tiled(struct item *item)
{
    if (!item->view->base.minimized) {
        scene_node_set_enabled(&item->view->tree->node, true);
    }

    manager->active_view = item->view;
    kywc_view_activate(&item->view->base);

    int lx, ly;
    scene_node_coords(item->part[ASSIST_THUMBNAIL].node, &lx, &ly);

    if (item->view->base.min_width + item->view->base.margin.off_width >
            manager->item_page->full_area.width ||
        item->view->base.min_height + item->view->base.margin.off_height >
            manager->item_page->full_area.height) {
        item->view->tile_start_geometry =
            (struct kywc_box){ lx, ly, item->snapshot_width, item->snapshot_height };
        kywc_view_set_tiled(&item->view->base, manager->item_page->tiled, &manager->output->base);
        item->view->tile_start_geometry = (struct kywc_box){ 0 };
        item_destroy(item);
        tile_assist_done();
        return;
    }

    item->view->tile_start_geometry =
        (struct kywc_box){ lx, ly, item->snapshot_width, item->snapshot_height };
    struct kywc_box geo = { 0 };
    geo.x = manager->item_page->full_area.x + item->view->base.margin.off_x;
    geo.y = manager->item_page->full_area.y + item->view->base.margin.off_y;
    geo.width = manager->item_page->full_area.width - item->view->base.margin.off_width;
    geo.height = manager->item_page->full_area.height - item->view->base.margin.off_height;
    view_do_tiled(item->view, manager->item_page->tiled, &manager->output->base, &geo);
    item->view->tile_start_geometry = (struct kywc_box){ 0 };
    assist_area_set_view(manager->item_page, item->view);
    item_destroy(item);

    if (manager->total_items == 0) {
        tile_assist_done();
        return;
    }

    item_page_update();
}

static int32_t get_last_index(void)
{
    struct assist *assist = manager->item_page;
    int32_t last_index = 0;

    if (manager->total_items > assist->total) {
        int32_t remainder = manager->total_items % assist->total;
        if (remainder) {
            int32_t row = remainder % assist->col == 0 ? remainder / assist->col
                                                       : remainder / assist->col + 1;
            last_index = manager->total_items - (remainder + (assist->row - row) * assist->col);
        } else {
            last_index = manager->total_items - assist->total;
        }
    }

    return last_index;
}

static void pointer_grab_cancel(struct seat_pointer_grab *pointer_grab)
{
    tile_assist_done();
}

static bool pointer_grab_button(struct seat_pointer_grab *pointer_grab, uint32_t time,
                                uint32_t button, bool pressed)
{
    struct seat *seat = pointer_grab->seat;
    struct input_event_node *inode = input_event_node_from_node(seat->cursor->hover.node);
    struct scene_node *node = input_event_node_root(inode);

    if (!node) {
        struct scene_node *hover =
            scene_node_at(&seat->scene->tree.node, seat->cursor->lx, seat->cursor->ly, NULL, NULL);
        inode = input_event_node_from_node(hover);
        node = input_event_node_root(inode);
    }

    if (node == &manager->item_page->tree->node) {
        inode->impl->click(seat, seat->cursor->hover.node, button, pressed, time, CLICK_STATE_NONE,
                           inode->data);
        return true;
    } else if (pressed) {
        tile_assist_done();
    }

    return !node;
}

static bool pointer_grab_motion(struct seat_pointer_grab *pointer_grab, uint32_t time, double lx,
                                double ly)
{
    return false;
}

static bool pointer_grab_axis(struct seat_pointer_grab *pointer_grab, uint32_t time, bool vertical,
                              double value)
{
    struct seat *seat = pointer_grab->seat;
    struct input_event_node *inode = input_event_node_from_node(seat->cursor->hover.node);
    struct scene_node *node = input_event_node_root(inode);
    if (node != &manager->item_page->tree->node) {
        return false;
    }

    static uint32_t last_time = 0;
    if (time - last_time < 100) {
        return false;
    }
    last_time = time;

    struct assist *assist = manager->item_page;
    int32_t last_index = get_last_index();
    int32_t next_index =
        value < 0 ? assist->index_first - assist->col : assist->index_first + assist->col;
    if (next_index >= 0 && next_index <= last_index) {
        assist->index_first = next_index;
    }

    assist_item_show();
    return true;
}

static bool pointer_grab_frame(struct seat_pointer_grab *pointer_grab)
{
    return false;
}

static const struct seat_pointer_grab_interface pointer_grab_impl = {
    .motion = pointer_grab_motion,
    .button = pointer_grab_button,
    .axis = pointer_grab_axis,
    .frame = pointer_grab_frame,
    .cancel = pointer_grab_cancel,
};

static bool touch_grab_touch(struct seat_touch_grab *touch_grab, uint32_t time, bool down,
                             uint32_t serial)
{
    seat_feed_pointer_motion(touch_grab->seat, touch_grab->seat->cursor->lx,
                             touch_grab->seat->cursor->ly, true);
    return pointer_grab_button(&manager->pointer_grab, time, BTN_LEFT, down);
}

static bool touch_grab_motion(struct seat_touch_grab *touch_grab, uint32_t time, double lx,
                              double ly, uint32_t serial)
{
    return false;
}

static void touch_grab_cancel(struct seat_touch_grab *touch_grab) {}

static const struct seat_touch_grab_interface touch_grab_impl = {
    .touch = touch_grab_touch,
    .motion = touch_grab_motion,
    .cancel = touch_grab_cancel,
};

static bool keyboard_grab_key(struct seat_keyboard_grab *keyboard_grab, struct keyboard *keyboard,
                              uint32_t time, uint32_t key, bool pressed, uint32_t modifiers)
{
    if (!pressed) {
        return true;
    }

    if (key == KEY_LEFTMETA || key == KEY_RIGHTMETA || key == KEY_ESC) {
        tile_assist_done();
        return true;
    }
    if (key == KEY_ENTER && manager->selected) {
        struct item *select = manager->selected;
        manager->selected = NULL;
        item_view_set_tiled(select);
        return true;
    }

    if (key != KEY_UP && key != KEY_DOWN && key != KEY_LEFT && key != KEY_RIGHT) {
        return true;
    }

    struct assist *assist = manager->item_page;
    int32_t current = 0;
    if (assist->current < 0) {
        current = 0;
    } else if (key == KEY_UP) {
        current = assist->current - assist->col;
        if (current < 0) {
            int32_t row = manager->total_items / assist->col;
            if (assist->current % assist->col < manager->total_items % assist->col) {
                current = row * assist->col + assist->current % assist->col;
            } else {
                current = row * assist->col - (assist->col - assist->current % assist->col);
            }
            assist->index_first = get_last_index();
        } else if (current < assist->index_first) {
            current = assist->current - assist->col;
            assist->index_first = assist->index_first - assist->col;
        }
    } else if (key == KEY_DOWN) {
        current = assist->current + assist->col;
        if (current >= manager->total_items) {
            int32_t last_index = get_last_index();
            if (assist->index_first < last_index) {
                assist->index_first = last_index;
                current = assist->current;
            } else {
                assist->index_first = 0;
                current = assist->current % assist->col;
            }
        } else if (current >= assist->index_first + assist->total) {
            assist->index_first = assist->index_first + assist->col;
        }
    } else if (key == KEY_LEFT) {
        current = assist->current - 1;
        if (current < 0) {
            current = manager->total_items - 1;
            assist->index_first = get_last_index();
        } else if (current < assist->index_first) {
            assist->index_first -= assist->col;
        }
    } else if (key == KEY_RIGHT) {
        current = assist->current + 1;
        if (current > manager->total_items - 1) {
            current = 0;
            assist->index_first = 0;
        } else if (current >= assist->index_first + assist->total) {
            assist->index_first = assist->index_first + assist->col;
        }
    }

    assist->current = current;
    assist_item_show();
    return true;
}

static void keyboard_grab_cancel(struct seat_keyboard_grab *keyboard_grab)
{
    tile_assist_done();
}

static bool item_hover(struct seat *seat, struct scene_node *node, double x, double y,
                       uint32_t time, bool first, bool hold, void *data)
{
    if (!data) {
        return false;
    }

    struct item_part *item_part = data;

    if (item_part->part == ASSIST_BUTTON_CLOSE) {
        set_close_buffer(item_part, BUTTON_STATE_HOVER);
    }

    int32_t i = 0;
    struct item *item_tmp;
    wl_list_for_each(item_tmp, &manager->items, link) {
        if (!item_tmp->tree) {
            continue;
        }
        if (i == manager->item_page->current) {
            scene_node_set_enabled(item_tmp->selection, false);
            break;
        }
        i++;
    }

    scene_node_set_enabled(item_part->item->part[ASSIST_BUTTON_CLOSE].node, true);
    scene_node_set_enabled(item_part->item->selection, true);

    i = 0;
    wl_list_for_each(item_tmp, &manager->items, link) {
        if (!item_tmp->tree) {
            continue;
        }
        if (item_part->item == item_tmp) {
            manager->item_page->current = i;
            break;
        }
        i++;
    }

    return false;
}

static void item_leave(struct seat *seat, struct scene_node *node, bool last, void *data)
{
    if (!data) {
        return;
    }

    struct item_part *item_part = data;

    scene_node_set_enabled(item_part->item->part[ASSIST_BUTTON_CLOSE].node, false);
    if (item_part->part == ASSIST_BUTTON_CLOSE) {
        set_close_buffer(item_part, BUTTON_STATE_NONE);
    }
}

static void item_click(struct seat *seat, struct scene_node *node, uint32_t button, bool pressed,
                       uint32_t time, enum click_state state, void *data)
{
    if (pressed || button != BTN_LEFT) {
        return;
    }

    if (!data) {
        tile_assist_done();
        return;
    }

    struct item_part *item_part = data;

    if (item_part->part != ASSIST_BUTTON_CLOSE) {
        item_view_set_tiled(item_part->item);
        return;
    }

    if (LEFT_BUTTON_PRESSED(button, pressed)) {
        set_close_buffer(item_part, BUTTON_STATE_CLICKED);
    }
    if (LEFT_BUTTON_RELEASED(button, pressed)) {
        kywc_view_close(&item_part->item->view->base);
    }
}

static const struct input_event_node_impl item_impl = {
    .hover = item_hover,
    .leave = item_leave,
    .click = item_click,
};

static struct scene_node *item_get_root(void *data)
{
    return &manager->item_page->tree->node;
}

static const struct seat_keyboard_grab_interface keyboard_grab_impl = {
    .key = keyboard_grab_key,
    .cancel = keyboard_grab_cancel,
};

static struct scene_node *selected_box_paint(struct scene_tree *parent, int w, int h, int border,
                                             int radius, float *color)
{
    struct scene_decoration *frame = scene_decoration_create(parent);
    float select_color[4];
    color_float_pa(select_color, color);
    scene_decoration_set_margin_color(frame, (float[4]){ 0.f, 0.f, 0.f, 0.f }, select_color);
    scene_decoration_set_surface_size(frame, w, h);
    scene_decoration_set_margin(frame, 0, border);
    scene_decoration_set_mask(frame, DECORATION_MASK_ALL);
    scene_decoration_set_round_corner_radius(frame, (int[4]){ radius, radius, radius, radius });
    return scene_node_from_decoration(frame);
}

static bool item_init(struct item *item)
{
    if (!KYWC_VIEW_IS_RESIZABLE(&item->view->base) || item->view->base.skip_switcher ||
        item->view->parent) {
        return false;
    }

    item->snapshot = snapshot_create_from_view(
        item->view, 1.0f, SNAPSHOT_RENDER_DISABLE_BLUR | SNAPSHOT_ENABLE_STYLE_EFFECT);
    if (!item->snapshot) {
        item_destroy(item);
        return false;
    }

    int32_t item_w = manager->fixed_width;
    int32_t item_h = manager->fixed_height;
    struct theme *theme = theme_manager_get_theme();
    item->tree = scene_tree_create(manager->item_page->tree);
    manager->total_items++;

    item->selection = selected_box_paint(item->tree, item_w - 2 * SELECT_BORDER_WIDTH,
                                         item_h - 2 * SELECT_BORDER_WIDTH, SELECT_BORDER_WIDTH,
                                         theme->window_radius, theme->accent_color);
    scene_node_set_enabled(item->selection, false);
    scene_node_set_position(item->selection, SELECT_BORDER_WIDTH, SELECT_BORDER_WIDTH);

    for (int32_t i = ASSIST_ROOT; i < ASSIST_PART_COUNT; i++) {
        item->part[i].part = i;
        item->part[i].item = item;
        item->part[i].scale = 1.0;
        int32_t x = 0, y = 0;
        if (i == ASSIST_ROOT) {
            item->part[i].node = &item->tree->node;
        } else if (i == ASSIST_BUTTON_CLOSE || i == ASSIST_TITLE_ICON) {
            struct scene_buffer *buf =
                scaled_buffer_create(item->tree, item->part[i].scale, item_buffer_update,
                                     item_buffer_destroy, &item->part[i]);
            item->part[i].node = &buf->node;

            if (i == ASSIST_BUTTON_CLOSE) {
                scene_node_set_enabled(item->part[i].node, false);
                set_close_buffer(&item->part[i], BUTTON_STATE_NONE);
                x = item_w - SELECT_BORDER_WIDTH - ITEM_GAP - theme->button_width;
                y = SELECT_BORDER_WIDTH +
                    (ITEM_HEIGHT - SELECT_BORDER_WIDTH - theme->button_width) * 0.5;
            } else {
                icon_buffer_update(&item->part[i], item->part[i].scale);
                x = SELECT_BORDER_WIDTH + ITEM_GAP;
                y = SELECT_BORDER_WIDTH +
                    (ITEM_HEIGHT - SELECT_BORDER_WIDTH - theme->icon_size) * 0.5;
            }
        } else if (i == ASSIST_TITLE_TEXT) {
            item->title_text = widget_create(item->tree);
            item->part[i].node = scene_node_from_widget(item->title_text);
            item->max_text_width = item_w - 4 * ITEM_GAP - 2 * SELECT_BORDER_WIDTH -
                                   theme->icon_size - theme->button_width;
            title_text_update(item);
            x = SELECT_BORDER_WIDTH + theme->icon_size + 2 * ITEM_GAP;
            y = SELECT_BORDER_WIDTH + (ITEM_HEIGHT - item->text_height) * 0.5;
        } else if (i == ASSIST_FRAME_RECT) {
            struct scene_rect *background =
                scene_rect_create(item->tree, item_w, item_h, (float[4]){ 0.f, 0.f, 0.f, 0.f });
            item->part[i].node = &background->node;
        } else if (i == ASSIST_THUMBNAIL) {
            int32_t tmp_width = item_w - 2 * SELECT_BORDER_WIDTH;
            int32_t tmp_height = item_h - ITEM_HEIGHT - 2 * SELECT_BORDER_WIDTH;
            int32_t view_width = item->view->base.geometry.width;
            int32_t view_height = item->view->base.geometry.height;
            x = SELECT_BORDER_WIDTH;
            y = ITEM_HEIGHT + SELECT_BORDER_WIDTH;

            float tmp_ratio = (float)tmp_width / tmp_height;
            float view_ratio = (float)view_width / view_height;

            if (tmp_ratio < view_ratio) {
                item->snapshot_width = tmp_width;
                item->snapshot_height = tmp_width / view_ratio;
                y += (tmp_height - item->snapshot_height) / 2;
            } else {
                item->snapshot_height = tmp_height;
                item->snapshot_width = tmp_height * view_ratio;
                x += (tmp_width - item->snapshot_width) / 2;
            }

            struct scene_buffer *buffer = snapshot_create_scene_buffer(item->tree, item->snapshot);
            if (!buffer) {
                continue;
            }

            scene_buffer_set_dest_size(buffer, item->snapshot_width, item->snapshot_height);
            item->part[i].node = &buffer->node;
        }

        scene_node_set_position(item->part[i].node, x, y);
        input_event_node_create(item->part[i].node, &item_impl, item_get_root, NULL,
                                &item->part[i]);
    }

    return true;
}

static void assist_init(struct assist *assist, enum kywc_tile tile, struct item_proxy *proxy)
{
    assist->tiled = tile;
    assist->index_first = 0;
    assist->current = -1;
    assist->tree = scene_tree_create(manager->tree);

    assist->background = scene_rect_create(assist->tree, 0, 0, (float[4]){ 0.f, 0.f, 0.f, 0.f });
    float color[4];
    struct theme *theme = theme_manager_get_theme();
    color_float_pax(color, theme->active_bg_color, theme->opacity / 100.0);
    assist->assist_area = scene_rect_create(assist->tree, 0, 0, color);

    input_event_node_create(&assist->tree->node, &item_impl, item_get_root, NULL, NULL);

    int radius = theme->window_radius;
    scene_node_set_radius(&assist->assist_area->node, (int[4]){ radius, radius, radius, radius });
    /* add blur */
    pixman_region32_t region;
    pixman_region32_init(&region);
    scene_node_set_blur_region(&assist->assist_area->node, theme->opacity != 100 ? &region : NULL);
    pixman_region32_fini(&region);

    if (proxy) {
        assist_area_set_view(assist, proxy->item->view);
    } else {
        tile_calculate_view_geometry(&manager->tiled_layer, manager->output, &assist->full_area,
                                     tile);
    }
}

static void assist_get_split_type(void)
{
    bool is_half_screen = false;
    bool is_quarter_screen = false;
    enum kywc_tile tile = KYWC_TILE_NONE;

    for (int32_t i = 0; i < SLOT_TYPE_COUNT; i++) {
        if (!manager->tiled_layer.proxy[i]) {
            continue;
        }
        if (manager->tiled_layer.proxy[i]->item->view->base.tiled == KYWC_TILE_LEFT ||
            manager->tiled_layer.proxy[i]->item->view->base.tiled == KYWC_TILE_RIGHT ||
            manager->tiled_layer.proxy[i]->item->view->base.tiled == KYWC_TILE_TOP ||
            manager->tiled_layer.proxy[i]->item->view->base.tiled == KYWC_TILE_BOTTOM) {
            is_half_screen = true;
            tile = manager->tiled_layer.proxy[i]->item->view->base.tiled;
        } else {
            is_quarter_screen = true;
        }
    }

    if (is_half_screen && is_quarter_screen) {
        manager->state = THREE_SCREEN;
        if (tile == KYWC_TILE_LEFT) {
            manager->type = TYPE_THREE_LEFT;
        } else if (tile == KYWC_TILE_RIGHT) {
            manager->type = TYPE_THREE_RIGHT;
        } else if (tile == KYWC_TILE_TOP) {
            manager->type = TYPE_THREE_TOP;
        } else if (tile == KYWC_TILE_BOTTOM) {
            manager->type = TYPE_THREE_BOTTOM;
        }
    } else if (is_half_screen) {
        manager->state = HALF_SCREEN;
        if (tile == KYWC_TILE_LEFT || tile == KYWC_TILE_RIGHT) {
            manager->type = TYPE_HALF_LEFT_OR_RIGHT;
        } else if (tile == KYWC_TILE_TOP || tile == KYWC_TILE_BOTTOM) {
            manager->type = TYPE_HALF_TOP_OR_BOTTOM;
        }
    } else {
        manager->state = QUARTER_SCREEN;
        manager->type = TYPE_QUARTER;
    }
}

static void create_assist_area(void)
{
    assist_get_split_type();
    if (manager->type == TYPE_HALF_LEFT_OR_RIGHT) {
        assist_init(&manager->assist[0], KYWC_TILE_LEFT, manager->tiled_layer.proxy[0]);
        assist_init(&manager->assist[1], KYWC_TILE_RIGHT, manager->tiled_layer.proxy[1]);
    } else if (manager->type == TYPE_HALF_TOP_OR_BOTTOM) {
        assist_init(&manager->assist[0], KYWC_TILE_TOP, manager->tiled_layer.proxy[0]);
        assist_init(&manager->assist[1], KYWC_TILE_BOTTOM, manager->tiled_layer.proxy[2]);
    } else if (manager->type == TYPE_THREE_LEFT) {
        assist_init(&manager->assist[0], KYWC_TILE_LEFT, manager->tiled_layer.proxy[0]);
        assist_init(&manager->assist[1], KYWC_TILE_TOP_RIGHT, manager->tiled_layer.proxy[1]);
        assist_init(&manager->assist[2], KYWC_TILE_BOTTOM_RIGHT, manager->tiled_layer.proxy[3]);
    } else if (manager->type == TYPE_THREE_RIGHT) {
        assist_init(&manager->assist[0], KYWC_TILE_TOP_LEFT, manager->tiled_layer.proxy[0]);
        assist_init(&manager->assist[1], KYWC_TILE_BOTTOM_LEFT, manager->tiled_layer.proxy[2]);
        assist_init(&manager->assist[2], KYWC_TILE_RIGHT, manager->tiled_layer.proxy[1]);
    } else if (manager->type == TYPE_THREE_TOP) {
        assist_init(&manager->assist[0], KYWC_TILE_TOP, manager->tiled_layer.proxy[0]);
        assist_init(&manager->assist[1], KYWC_TILE_BOTTOM_LEFT, manager->tiled_layer.proxy[2]);
        assist_init(&manager->assist[2], KYWC_TILE_BOTTOM_RIGHT, manager->tiled_layer.proxy[3]);
    } else if (manager->type == TYPE_THREE_BOTTOM) {
        assist_init(&manager->assist[0], KYWC_TILE_TOP_LEFT, manager->tiled_layer.proxy[0]);
        assist_init(&manager->assist[1], KYWC_TILE_TOP_RIGHT, manager->tiled_layer.proxy[1]);
        assist_init(&manager->assist[2], KYWC_TILE_BOTTOM, manager->tiled_layer.proxy[2]);
    } else if (manager->type == TYPE_QUARTER) {
        assist_init(&manager->assist[0], KYWC_TILE_TOP_LEFT, manager->tiled_layer.proxy[0]);
        assist_init(&manager->assist[1], KYWC_TILE_TOP_RIGHT, manager->tiled_layer.proxy[1]);
        assist_init(&manager->assist[2], KYWC_TILE_BOTTOM_LEFT, manager->tiled_layer.proxy[2]);
        assist_init(&manager->assist[3], KYWC_TILE_BOTTOM_RIGHT, manager->tiled_layer.proxy[3]);
    }
}

static void handle_new_mapped_view(struct wl_listener *listener, void *data)
{
    struct kywc_view *kywc_view = data;
    if (kywc_view->role != KYWC_VIEW_ROLE_NORMAL) {
        return;
    }

    manager->active_view = NULL;
    tile_assist_done();
}

static bool tiled_assist_views_check(struct view *view)
{
    for (int i = 0; i < SLOT_TYPE_COUNT; i++) {
        if (manager->tiled_layer.proxy[i] && manager->tiled_layer.proxy[i]->item->view == view) {
            return true;
        }
    }

    return false;
}

static void tiled_assist_show(void)
{
    create_assist_area();
    item_page_assist_update();

    if (!manager->item_page) {
        tile_assist_done();
        return;
    }

    struct tiled_output *tiled_output = tiled_output_from_kywc_output(manager->active_view->output);
    struct tiled_place *place =
        get_tiled_place(tiled_output, manager->active_view->current_proxy->workspace);
    struct item_proxy *proxy;
    wl_list_for_each_reverse(proxy, &place->item_proxys, place_link) {
        if (manager->active_view == proxy->item->view) {
            continue;
        }
        if (tiled_assist_views_check(proxy->item->view)) {
            continue;
        }

        struct item *item = item_create(proxy->item->view, true);
        if (item) {
            item_init(item);
        }
    }

    if (manager->total_items == 0) {
        tile_assist_done();
        return;
    }

    scene_node_set_enabled(&manager->tree->node, true);

    assist_item_show();
}

static void handle_configured_output(struct wl_listener *listener, void *data)
{
    tile_assist_done();
}

static void handle_seat_destroy(struct wl_listener *listener, void *data)
{
    tile_assist_done();
}

static void handle_show_desktop(struct wl_listener *listener, void *data)
{
    manager->active_view = NULL;
    tile_assist_done();
}

static void handle_workspace_activate(struct wl_listener *listener, void *data)
{
    if (!manager->workspace->activated) {
        tile_assist_done();
    }
}

static bool views_full_check(struct tiled_layer *layer)
{
    int lenght = 0;
    for (int i = 0; i < SLOT_TYPE_COUNT; i++) {
        if (layer->proxy[i]) {
            lenght++;
        }
    }

    return lenght == SLOT_TYPE_COUNT;
}

static void handle_highlight_view(struct wl_listener *listener, void *data)
{
    bool enable = !view_manager_get_highlight();
    scene_node_set_enabled(&manager->tree->node, enable);

    if (enable) {
        for (int32_t i = 0; i < SLOT_TYPE_COUNT; i++) {
            struct assist *assist = &manager->assist[i];
            if (assist->view) {
                scene_node_set_enabled(&assist->view->tree->node, true);
            }
        }

        struct item *item;
        wl_list_for_each(item, &manager->items, link) {
            if (!item->view->base.minimized) {
                scene_node_set_enabled(&item->view->tree->node, false);
            }
        }
    }
}

void view_begin_tile_assist(struct view *view, struct seat *seat, struct output *output,
                            struct tiled_layer *tiled_layer)
{
    if (manager || views_full_check(tiled_layer)) {
        return;
    }
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return;
    }

    wl_list_init(&manager->items);

    view_do_activate(NULL);
    seat_focus_surface(seat, NULL);

    manager->seat = seat;
    manager->workspace = view->current_proxy->workspace;
    manager->tiled_layer = *tiled_layer;
    manager->pointer_grab = (struct seat_pointer_grab){ &pointer_grab_impl, seat, manager };
    seat_start_pointer_grab(seat, &manager->pointer_grab);
    manager->keyboard_grab = (struct seat_keyboard_grab){ &keyboard_grab_impl, seat, manager };
    seat_start_keyboard_grab(seat, &manager->keyboard_grab);
    manager->touch_grab = (struct seat_touch_grab){ &touch_grab_impl, seat, manager, 0 };
    seat_start_touch_grab(seat, &manager->touch_grab);

    struct kywc_box *usable_area = &output->usable_area;
    manager->fixed_width = (usable_area->width - 160) / 4;
    manager->fixed_height = usable_area->height / 22 * 5;
    struct view_layer *layer = view_manager_get_layer(LAYER_SWITCHER, false);
    manager->tree = scene_tree_create(layer->tree);
    scene_node_set_enabled(&manager->tree->node, false);

    manager->new_mapped_view.notify = handle_new_mapped_view;
    kywc_view_add_new_mapped_listener(&manager->new_mapped_view);
    manager->configured_output.notify = handle_configured_output;
    output_manager_add_configured_listener(&manager->configured_output);
    manager->seat_destroy.notify = handle_seat_destroy;
    wl_signal_add(&seat->events.destroy, &manager->seat_destroy);
    manager->show_desktop.notify = handle_show_desktop;
    view_manager_add_show_desktop_listener(&manager->show_desktop);
    manager->workspace_activate.notify = handle_workspace_activate;
    wl_signal_add(&manager->workspace->events.activate, &manager->workspace_activate);
    manager->highlight.notify = handle_highlight_view;
    view_manager_add_highlight_view_listener(&manager->highlight);

    manager->output = output;
    manager->item_page = NULL;
    manager->selected = NULL;
    manager->total_items = 0;
    manager->active_view = view;
    tiled_assist_show();
}
