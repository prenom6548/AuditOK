// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include "libkywc_p.h"

static struct ky_cursor *cursor_from_kywc_cursor(kywc_cursor *kywc_cursor)
{
    struct ky_cursor *cursor = wl_container_of(kywc_cursor, cursor, base);
    return cursor;
}

void ky_cursor_destroy(struct ky_cursor *cursor)
{
    if (cursor->impl && cursor->impl->destroy) {
        cursor->impl->destroy(&cursor->base);
    }

    if (cursor->destroy) {
        cursor->destroy(cursor);
    }

    wl_list_remove(&cursor->link);

    free(cursor);
}

struct ky_cursor_manager *ky_cursor_manager_create(kywc_context *ctx)
{
    struct ky_cursor_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return NULL;
    }

    manager->ctx = ctx;
    wl_list_init(&manager->cursors);

    return manager;
}

void ky_cursor_manager_destroy(struct ky_cursor_manager *manager)
{
    if (!manager) {
        return;
    }

    struct ky_cursor *cursor, *c_tmp;
    wl_list_for_each_safe(cursor, c_tmp, &manager->cursors, link) {
        ky_cursor_destroy(cursor);
    }

    if (manager->destroy) {
        manager->destroy(manager);
    }

    free(manager);
}

void ky_cursor_enter(struct ky_cursor *cursor)
{
    if (cursor->impl && cursor->impl->enter) {
        cursor->impl->enter(&cursor->base);
    }
}

void ky_cursor_leave(struct ky_cursor *cursor)
{
    if (cursor->impl && cursor->impl->leave) {
        cursor->impl->leave(&cursor->base);
    }
}

void ky_cursor_update_position(struct ky_cursor *cursor, uint32_t x, uint32_t y)
{
    if (cursor->impl && cursor->impl->position) {
        cursor->impl->position(&cursor->base, x, y);
    }
}

void ky_cursor_update_hotspot(struct ky_cursor *cursor, uint32_t x, uint32_t y)
{
    if (cursor->impl && cursor->impl->hotspot) {
        cursor->impl->hotspot(&cursor->base, x, y);
    }
}

kywc_cursor *kywc_cursor_create(kywc_context *ctx, struct wl_seat *seat,
                                const struct kywc_cursor_interface *impl, void *data)
{
    return kywc_cursor_create_from_thumbnail(ctx, seat, NULL, impl, data);
}

kywc_cursor *kywc_cursor_create_from_thumbnail(kywc_context *ctx, struct wl_seat *seat,
                                               kywc_thumbnail *thumbnail,
                                               const struct kywc_cursor_interface *impl, void *data)
{
    if (!ctx || !ctx->cursor) {
        return NULL;
    }

    struct ky_cursor *cursor = calloc(1, sizeof(*cursor));
    if (!cursor) {
        return NULL;
    }

    struct ky_thumbnail *ky_thumbnail = NULL;
    struct ky_cursor_manager *manager = ctx->cursor;
    cursor->manager = manager;
    wl_list_insert(&manager->cursors, &cursor->link);

    cursor->impl = impl;
    cursor->user_data = data;

    if (thumbnail) {
        ky_thumbnail = thumbnail_from_kywc_thumbnail(thumbnail);
    }

    if (manager->create_cursor) {
        manager->create_cursor(manager, cursor, seat, ky_thumbnail);
    }

    return &cursor->base;
}

void kywc_cursor_set_user_data(kywc_cursor *cursor, void *data)
{
    struct ky_cursor *ky_cursor = cursor_from_kywc_cursor(cursor);
    ky_cursor->user_data = data;
}

void *kywc_cursor_get_user_data(kywc_cursor *cursor)
{
    struct ky_cursor *ky_cursor = cursor_from_kywc_cursor(cursor);
    return ky_cursor->user_data;
}

void kywc_cursor_destroy(kywc_cursor *cursor)
{
    struct ky_cursor *ky_cursor = cursor_from_kywc_cursor(cursor);
    ky_cursor_destroy(ky_cursor);
}
