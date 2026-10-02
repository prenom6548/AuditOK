// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>

#include "output.h"
#include "util/hash_table.h"
#include "view_p.h"

#define GRID_GAP_ROW (50)
#define GRID_GAP_COLUMN (50)

static struct positioner_manager {
    struct wl_list positioners;
    struct wl_event_loop *event_loop;
    struct wl_listener new_output;
    struct wl_listener server_destroy;
} *manager = NULL;

/* per output */
struct positioner {
    struct wl_list link;
    struct wl_list places;
    struct wl_list items;

    char *output_name;
    struct kywc_output *kywc_output;
    struct wl_listener output_on;
    struct wl_listener output_off;
    struct wl_listener output_usable_area;
    struct wl_listener output_destroy;

    struct {
        struct kywc_box box;
        struct wl_list positioners;
        struct wl_list link;
    } catcher;

    struct kywc_box usable_area;
    struct wl_event_source *saved_geometry_timeout;
};

/* per workspace in positioner */
struct place {
    struct wl_list link;
    struct positioner *positioner;

    struct workspace *workspace;
    struct wl_listener workspace_destroy;

    struct wl_list entries;
    struct hash_table *groups;
};

/* entries group with app_id */
struct group {
    const char *app_id;
    struct wl_list entries; // entry.group_link
    uint32_t hash;
};

/* per view insert to place slot */
struct entry {
    struct wl_list link;
    struct place *place;

    struct wl_list group_link;
    struct group *group;

    struct kywc_view *view;
    struct wl_listener view_position;
    struct wl_listener view_size;
    struct wl_listener view_unmap;
    struct wl_listener view_workspace;
    struct wl_listener view_activate;

    int lx, ly;
    bool skip_update;

    struct kywc_box saved_geometry;
    bool skip_clear_saved_geometry;
};

/* need auto resize when output usable area changed */
struct item {
    struct wl_list link;
    struct positioner *positioner;

    struct kywc_view *view;
    struct wl_listener view_output;
    struct wl_listener view_unmap;
};

static void positioner_move_views(struct positioner *pos, struct kywc_box *src_box,
                                  struct positioner *dst_pos, bool skip_update)
{
    struct place *place;
    wl_list_for_each(place, &pos->places, link) {
        struct entry *entry, *tmp;
        wl_list_for_each_safe(entry, tmp, &place->entries, link) {
            /* keep entry's place, so we can store when output re-enabled or plugin */
            entry->skip_update = skip_update;
            /* move to dst */
            view_move_to_output(view_from_kywc_view(entry->view), src_box, &entry->saved_geometry,
                                dst_pos->kywc_output);
        }
    }
}

static void positioner_handle_output_on(struct wl_listener *listener, void *data)
{
    struct positioner *pos = wl_container_of(listener, pos, output_on);
    struct output *output = output_from_kywc_output(pos->kywc_output);
    pos->usable_area = output->usable_area;

    if (!wl_list_empty(&pos->catcher.link)) {
        positioner_move_views(pos, &pos->catcher.box, pos, false);
        wl_list_remove(&pos->catcher.link);
        wl_list_init(&pos->catcher.link);
    }
}

static void positioner_handle_output_off(struct wl_listener *listener, void *data)
{
    struct positioner *src = wl_container_of(listener, src, output_off);

    struct positioner *pos, *dst = NULL;
    wl_list_for_each(pos, &manager->positioners, link) {
        if (pos != src && pos->kywc_output && pos->kywc_output->state.enabled) {
            dst = pos;
            break;
        }
    }

    if (!dst) {
        kywc_log(KYWC_INFO, "No cather for output %s", src->output_name);
        return;
    }

    wl_list_insert(&dst->catcher.positioners, &src->catcher.link);
    src->catcher.box = dst->usable_area;
    positioner_move_views(src, &src->usable_area, dst, true);

    struct positioner *tmp;
    wl_list_for_each_safe(pos, tmp, &src->catcher.positioners, catcher.link) {
        wl_list_remove(&pos->catcher.link);
        wl_list_insert(&dst->catcher.positioners, &pos->catcher.link);
        pos->catcher.box = dst->usable_area;
        positioner_move_views(pos, &src->usable_area, dst, true);
    }
}

static void positioner_move_items(struct positioner *pos, struct kywc_box *src_box,
                                  struct positioner *dst_pos)
{
    struct item *item, *tmp;
    wl_list_for_each_safe(item, tmp, &pos->items, link) {
        view_move_to_output(view_from_kywc_view(item->view), src_box, NULL, dst_pos->kywc_output);
    }
}

static void entry_save_or_clear_geometry(struct entry *entry, bool save)
{
    if (save && kywc_box_empty(&entry->saved_geometry)) {
        entry->saved_geometry = entry->view->geometry;
        return;
    }
    if (!save && kywc_box_not_empty(&entry->saved_geometry)) {
        entry->saved_geometry = (struct kywc_box){ 0 };
    }
}
static int positioner_handle_saved_geometry_timeout(void *data)
{
    struct positioner *pos = data;
    struct place *place;
    wl_list_for_each(place, &pos->places, link) {
        struct entry *entry;
        wl_list_for_each(entry, &place->entries, link) {
            entry->skip_clear_saved_geometry = false;
        }
    }
    return 0;
}

static void positioner_handle_output_usable_area(struct wl_listener *listener, void *data)
{
    struct positioner *pos = wl_container_of(listener, pos, output_usable_area);
    struct output *output = output_from_kywc_output(pos->kywc_output);

    if (kywc_box_equal(&pos->usable_area, &output->usable_area)) {
        return;
    }

    struct place *place;
    wl_list_for_each(place, &pos->places, link) {
        struct entry *entry;
        wl_list_for_each(entry, &place->entries, link) {
            entry->skip_clear_saved_geometry = true;
            entry_save_or_clear_geometry(entry, true);
        }
    }
    if (!pos->saved_geometry_timeout) {
        pos->saved_geometry_timeout = wl_event_loop_add_timer(
            manager->event_loop, positioner_handle_saved_geometry_timeout, pos);
    }
    uint32_t timeout = view_manager_get_configure_timeout(NULL);
    wl_event_source_timer_update(pos->saved_geometry_timeout, timeout);

    struct kywc_box old = pos->usable_area;
    pos->usable_area = output->usable_area;
    positioner_move_views(pos, &old, pos, true);
    positioner_move_items(pos, &old, pos);

    struct positioner *tmp;
    wl_list_for_each(tmp, &pos->catcher.positioners, catcher.link) {
        tmp->catcher.box = pos->usable_area;
        positioner_move_views(tmp, &old, pos, true);
    }
}

static void positioner_handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct positioner *pos = wl_container_of(listener, pos, output_destroy);

    if (pos->kywc_output->state.enabled) {
        positioner_handle_output_off(&pos->output_off, NULL);
    }

    if (pos->saved_geometry_timeout) {
        wl_event_source_remove(pos->saved_geometry_timeout);
        pos->saved_geometry_timeout = NULL;
    }

    wl_list_remove(&pos->output_destroy.link);
    wl_list_remove(&pos->output_on.link);
    wl_list_remove(&pos->output_off.link);
    wl_list_remove(&pos->output_usable_area.link);

    /* don't destroy positioner */
    pos->kywc_output = NULL;
}

static struct positioner *positioner_from_output(struct kywc_output *kywc_output)
{
    struct positioner *pos;
    wl_list_for_each(pos, &manager->positioners, link) {
        /* we don't destroy positioner when output destroy */
        if (pos->kywc_output == kywc_output || !strcmp(pos->output_name, kywc_output->name)) {
            return pos;
        }
    }
    return NULL;
}

static void handle_new_output(struct wl_listener *listener, void *data)
{
    struct kywc_output *kywc_output = data;

    struct positioner *pos = positioner_from_output(kywc_output);
    if (!pos) {
        if (!(pos = calloc(1, sizeof(*pos)))) {
            return;
        }
        wl_list_init(&pos->places);
        wl_list_init(&pos->items);
        wl_list_insert(&manager->positioners, &pos->link);
        pos->output_name = strdup(kywc_output->name);

        wl_list_init(&pos->catcher.link);
        wl_list_init(&pos->catcher.positioners);
    }

    pos->kywc_output = kywc_output;

    pos->output_on.notify = positioner_handle_output_on;
    wl_signal_add(&kywc_output->events.on, &pos->output_on);
    pos->output_off.notify = positioner_handle_output_off;
    wl_signal_add(&kywc_output->events.off, &pos->output_off);

    struct output *output = output_from_kywc_output(kywc_output);
    pos->output_usable_area.notify = positioner_handle_output_usable_area;
    wl_signal_add(&output->events.usable_area, &pos->output_usable_area);

    pos->output_destroy.notify = positioner_handle_output_destroy;
    wl_signal_add(&kywc_output->events.destroy, &pos->output_destroy);

    if (kywc_output->state.enabled) {
        positioner_handle_output_on(&pos->output_on, NULL);
    }
}

static void place_handle_workspace_destroy(struct wl_listener *listener, void *data)
{
    struct place *place = wl_container_of(listener, place, workspace_destroy);
    wl_list_remove(&place->workspace_destroy.link);
    wl_list_remove(&place->link);
    hash_table_destroy(place->groups);
    free(place);
}

static struct place *positioner_get_place(struct positioner *pos, struct view *view)
{
    if (!pos || !view->current_proxy) {
        return NULL;
    }

    struct workspace *workspace = view->current_proxy->workspace;
    /* get place by workspace */
    struct place *place;
    wl_list_for_each(place, &pos->places, link) {
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
    place->positioner = pos;
    place->groups = hash_table_create_string(place);
    hash_table_set_max_entries(place->groups, 100);

    wl_list_init(&place->entries);
    wl_list_insert(&pos->places, &place->link);

    place->workspace_destroy.notify = place_handle_workspace_destroy;
    wl_signal_add(&workspace->events.destroy, &place->workspace_destroy);

    return place;
}

static void place_insert_entry(struct place *place, struct entry *entry)
{
    entry->place = place;
    wl_list_insert(&place->entries, &entry->link);

    const char *app_id = entry->view->app_id;
    if (!app_id) {
        return;
    }

    struct hash_entry *hash_entry = hash_table_search(place->groups, app_id);
    if (!hash_entry) {
        struct group *group = calloc(1, sizeof(*group));
        if (group) {
            group->app_id = strdup(app_id);
            wl_list_init(&group->entries);
            hash_entry = hash_table_insert(place->groups, group->app_id, group);
        }
    }
    if (!hash_entry) {
        return;
    }

    struct group *group = hash_entry->data;
    group->hash = hash_entry->hash;
    entry->group = group;
    wl_list_insert(&group->entries, &entry->group_link);
}

static void place_remove_entry(struct place *place, struct entry *entry)
{
    struct group *group = entry->group;
    if (group) {
        wl_list_remove(&entry->group_link);
        wl_list_init(&entry->group_link);
        entry->group = NULL;

        if (wl_list_empty(&group->entries)) {
            hash_table_remove_hash(place->groups, group->hash, group->app_id);
            free((void *)group->app_id);
            free(group);
        }
    }

    wl_list_remove(&entry->link);
    entry->place = NULL;
}

static void place_update_entry(struct place *place, struct entry *entry)
{
    if (place == entry->place) {
        return;
    }

    if (entry->place) {
        place_remove_entry(entry->place, entry);
    }
    place_insert_entry(place, entry);
}

static void entry_handle_view_size(struct wl_listener *listener, void *data)
{
    struct entry *entry = wl_container_of(listener, entry, view_size);
    if (!entry->skip_clear_saved_geometry) {
        entry_save_or_clear_geometry(entry, false);
    }
}

static void entry_handle_view_position(struct wl_listener *listener, void *data)
{
    struct entry *entry = wl_container_of(listener, entry, view_position);
    struct kywc_view *kywc_view = entry->view;
    if (!kywc_view->mapped) {
        return;
    }

    bool changed = entry->lx != kywc_view->geometry.x || entry->ly != kywc_view->geometry.y;
    if (changed) {
        entry->lx = kywc_view->geometry.x;
        entry->ly = kywc_view->geometry.y;
    }

    if (entry->skip_update) {
        entry->skip_update = false;
        return;
    }

    if (!changed) {
        return;
    }

    struct view *view = view_from_kywc_view(kywc_view);
    /* the view is on the most output */
    struct kywc_output *kywc_output = view->output;
    int lx = kywc_view->geometry.x - kywc_view->margin.off_x;
    int ly = kywc_view->geometry.y - kywc_view->margin.off_y;

    if (!kywc_output_contains_point(kywc_output, lx, ly)) {
        kywc_output = kywc_output_at_point(lx, ly);
        /* keep original output if the output does not contain (lx, ly) */
        if (!kywc_output_contains_point(kywc_output, lx, ly)) {
            kywc_output = view->output;
        }
    }

    struct positioner *pos = positioner_from_output(kywc_output);
    struct place *place = positioner_get_place(pos, view);
    place_update_entry(place, entry);
}

static void entry_handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct entry *entry = wl_container_of(listener, entry, view_unmap);

    if (entry->place) {
        place_remove_entry(entry->place, entry);
    }

    entry_save_or_clear_geometry(entry, false);

    wl_list_remove(&entry->view_unmap.link);
    wl_list_remove(&entry->view_position.link);
    wl_list_remove(&entry->view_size.link);
    wl_list_remove(&entry->view_workspace.link);
    wl_list_remove(&entry->view_activate.link);

    free(entry);
}

static void entry_handle_view_workspace(struct wl_listener *listener, void *data)
{
    struct entry *entry = wl_container_of(listener, entry, view_workspace);
    struct kywc_view *kywc_view = entry->view;
    struct view *view = view_from_kywc_view(kywc_view);

    /* view no longer in workspace */
    if (!view->current_proxy) {
        wl_list_remove(&entry->view_position.link);
        wl_list_init(&entry->view_position.link);
        wl_list_remove(&entry->view_size.link);
        wl_list_init(&entry->view_size.link);
        wl_list_remove(&entry->view_activate.link);
        wl_list_init(&entry->view_activate.link);
        if (entry->place) {
            place_remove_entry(entry->place, entry);
        }
        return;
    }

    /* view is not mapped, like xwayland shell */
    if (!kywc_view->mapped) {
        return;
    }

    /* add to workspace again */
    if (!entry->place) {
        entry->lx = kywc_view->geometry.x;
        entry->ly = kywc_view->geometry.y;
        wl_signal_add(&view->events.position, &entry->view_position);
        wl_signal_add(&kywc_view->events.size, &entry->view_size);
        wl_signal_add(&kywc_view->events.activate, &entry->view_activate);

        struct positioner *pos = positioner_from_output(view->output);
        struct place *place = positioner_get_place(pos, view);
        place_insert_entry(place, entry);
    }

    /* we assume that the position of view is not changed */
    struct positioner *pos = entry->place->positioner;
    struct place *new = positioner_get_place(pos, view);
    place_update_entry(new, entry);
}

static void entry_handle_view_activate(struct wl_listener *listener, void *data)
{
    struct entry *entry = wl_container_of(listener, entry, view_activate);
    if (!entry->view->activated || !entry->group) {
        return;
    }

    wl_list_remove(&entry->group_link);
    wl_list_insert(&entry->group->entries, &entry->group_link);
}

static void item_handle_view_output(struct wl_listener *listener, void *data)
{
    struct item *item = wl_container_of(listener, item, view_output);
    struct kywc_view *kywc_view = item->view;
    if (!kywc_view->mapped) {
        return;
    }

    struct view *view = view_from_kywc_view(kywc_view);
    struct kywc_output *kywc_output = view->output;
    if (kywc_output == item->positioner->kywc_output) {
        return;
    }

    struct positioner *pos = positioner_from_output(kywc_output);
    if (!pos) {
        return;
    }

    item->positioner = pos;
    wl_list_remove(&item->link);
    wl_list_insert(&pos->items, &item->link);
}

static void item_handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct item *item = wl_container_of(listener, item, view_unmap);

    wl_list_remove(&item->link);
    wl_list_remove(&item->view_unmap.link);
    wl_list_remove(&item->view_output.link);

    free(item);
}

static void positioner_add_item(struct positioner *pos, struct view *view)
{
    if (!pos || view->base.role != KYWC_VIEW_ROLE_SYSTEMWINDOW) {
        return;
    }

    struct item *item = calloc(1, sizeof(*item));
    if (!item) {
        return;
    }

    item->view = &view->base;
    item->view_output.notify = item_handle_view_output;
    wl_signal_add(&view->events.output, &item->view_output);
    item->view_unmap.notify = item_handle_view_unmap;
    wl_signal_add(&view->base.events.unmap, &item->view_unmap);

    item->positioner = pos;
    wl_list_insert(&pos->items, &item->link);
}

static void entry_fix_geometry(struct entry *entry, struct kywc_box *box, bool has_parent)
{
    struct kywc_view *kywc_view = entry->view;
    struct view *view = view_from_kywc_view(kywc_view);

    struct kywc_output *kywc_output = view->output;
    if (has_parent) {
        kywc_output = view->parent->output;
    } else if (kywc_view->has_initial_position) {
        kywc_output = kywc_output_at_point(view->pending.geometry.x, view->pending.geometry.y);
        /* use the default output if position is not in the layout */
        if (!kywc_output_contains_point(kywc_output, view->pending.geometry.x,
                                        view->pending.geometry.y)) {
            kywc_output = view->output;
        }
    }

    struct output *output = output_from_kywc_output(kywc_output);
    struct kywc_box *usable_area = &output->usable_area;
    bool need_resize = false;

    /* show view in the center of output or parent */
    struct kywc_box geo = {
        .width = kywc_view->geometry.width + kywc_view->margin.off_width,
        .height = kywc_view->geometry.height + kywc_view->margin.off_height,
    };
    if (kywc_view->has_initial_position) {
        geo.x = view->pending.geometry.x;
        geo.y = view->pending.geometry.y;
    } else {
        geo.x = box->x + (box->width - kywc_view->geometry.width) / 2 - kywc_view->margin.off_x;
        geo.y = box->y + (box->height - kywc_view->geometry.height) / 2 - kywc_view->margin.off_y;
    }

    /* make sure the view is in the usable area of output */
    if (geo.width > usable_area->width) {
        geo.x = usable_area->x;
        geo.width = usable_area->width;
        need_resize = true;
    } else if (geo.x < usable_area->x) {
        geo.x = usable_area->x;
    } else if (geo.x + geo.width > usable_area->x + usable_area->width) {
        geo.x = usable_area->x + usable_area->width - geo.width;
    }

    if (geo.height > usable_area->height) {
        geo.y = usable_area->y;
        geo.height = usable_area->height;
        need_resize = true;
    } else if (geo.y < usable_area->y) {
        geo.y = usable_area->y;
    } else if (geo.y + geo.height > usable_area->y + usable_area->height) {
        geo.y = usable_area->y + usable_area->height - geo.height;
    }

    geo.x += kywc_view->margin.off_x;
    geo.y += kywc_view->margin.off_y;

    if (need_resize && KYWC_VIEW_IS_RESIZABLE(kywc_view)) {
        geo.width -= kywc_view->margin.off_width;
        geo.height -= kywc_view->margin.off_height;
        view_do_resize(view, &geo);
    } else {
        view_do_move(view, geo.x, geo.y);
    }
}

static struct entry *group_get_last_entry(struct group *group)
{
    struct entry *entry;
    wl_list_for_each(entry, &group->entries, group_link) {
        if (!entry->view->minimized) {
            return entry;
        }
    }
    return NULL;
}

static void entry_fix_position(struct entry *entry, struct kywc_box *box, struct group *group)
{
    struct entry *last = group_get_last_entry(group);
    if (!last) {
        entry_fix_geometry(entry, box, false);
        return;
    }

    struct kywc_view *kywc_view = entry->view;
    struct view *view = view_from_kywc_view(kywc_view);
    int width = kywc_view->geometry.width + kywc_view->margin.off_width;
    int height = kywc_view->geometry.height + kywc_view->margin.off_height;
    /* fallback to the fix_geometry */
    if (width > box->width || height > box->height) {
        entry_fix_geometry(entry, box, false);
        return;
    }

    /* find a suitable position for the entry */
    int lx = last->lx - kywc_view->margin.off_x;
    int ly = last->ly - kywc_view->margin.off_y;

    for (int i = 0; i < 2; i++) {
        if (lx < box->x) {
            lx = box->x;
        }
        if (ly < box->y) {
            ly = box->y;
        }
        if (lx + width > box->x + box->width) {
            lx = box->x;
            ly = box->y;
        }
        if (ly + height > box->y + box->height) {
            ly = box->y;
        }
        if (i == 0) {
            lx += GRID_GAP_COLUMN;
            ly += GRID_GAP_ROW;
        }
    }

    lx += kywc_view->margin.off_x;
    ly += kywc_view->margin.off_y;
    view_do_move(view, lx, ly);
}

void positioner_add_new_view(struct view *view)
{
    if (!manager) {
        return;
    }

    assert(view->base.mapped == false);

    struct positioner *pos = positioner_from_output(view->output);
    struct place *place = positioner_get_place(pos, view);
    /* no output or workspace */
    if (!place) {
        positioner_add_item(pos, view);
        return;
    }

    struct entry *entry = calloc(1, sizeof(*entry));
    if (!entry) {
        return;
    }

    wl_list_init(&entry->link);
    wl_list_init(&entry->group_link);

    struct kywc_view *kywc_view = &view->base;
    entry->view = kywc_view;
    entry->lx = kywc_view->geometry.x;
    entry->ly = kywc_view->geometry.y;

    /* view alwayas has a workspace in view_init or xdg_activation */
    entry->view_position.notify = entry_handle_view_position;
    wl_signal_add(&view->events.position, &entry->view_position);
    entry->view_size.notify = entry_handle_view_size;
    wl_signal_add(&kywc_view->events.size, &entry->view_size);
    entry->view_activate.notify = entry_handle_view_activate;
    wl_signal_add(&view->base.events.activate, &entry->view_activate);
    entry->view_unmap.notify = entry_handle_view_unmap;
    wl_signal_add(&kywc_view->events.unmap, &entry->view_unmap);
    entry->view_workspace.notify = entry_handle_view_workspace;
    wl_signal_add(&view->events.workspace, &entry->view_workspace);

    if (kywc_view->maximized || kywc_view->minimized || kywc_view->fullscreen) {
        place_insert_entry(place, entry);
        return;
    }

    bool has_parent = view->parent && view->parent->base.mapped;
    struct kywc_box *box = has_parent ? &view->parent->base.geometry : &pos->usable_area;

    if (has_parent) {
        entry_fix_geometry(entry, box, true);
    } else if (kywc_view->has_initial_position) {
        entry_fix_geometry(entry, box, false);
    } else {
        struct hash_entry *hash_entry = NULL;
        if (kywc_view->app_id) {
            hash_entry = hash_table_search(place->groups, kywc_view->app_id);
        }
        if (hash_entry && hash_entry->data) {
            struct group *group = hash_entry->data;
            assert(!wl_list_empty(&group->entries));
            entry_fix_position(entry, box, group);
        } else {
            entry_fix_geometry(entry, box, false);
        }
    }

    /* view is not mapped current, entry->place must be NULL */
    assert(entry->place == NULL);
    place_insert_entry(place, entry);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->server_destroy.link);
    wl_list_remove(&manager->new_output.link);

    struct positioner *pos, *tmp;
    wl_list_for_each_safe(pos, tmp, &manager->positioners, link) {
        wl_list_remove(&pos->link);
        free(pos->output_name);
        free(pos);
    }

    free(manager);
    manager = NULL;
}

bool positioner_manager_create(struct view_manager *view_manager)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    wl_list_init(&manager->positioners);
    manager->event_loop = view_manager->server->event_loop;

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(view_manager->server, &manager->server_destroy);

    manager->new_output.notify = handle_new_output;
    kywc_output_add_new_listener(&manager->new_output);

    return true;
}
