// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <wlr/render/drm_format_set.h>
#include <wlr/types/wlr_buffer.h>

#include <kywc/log.h>

#include "render/allocator.h"

#define SWAPCHAIN_CAP 4

struct swapchain_slot {
    struct wlr_buffer *buffer;
    bool acquired; // waiting for release
    int age;

    struct wl_listener release;
};

struct swapchain {
    struct wlr_allocator *allocator;

    int width, height;
    uint32_t flags;
    enum allocator_buffer_type type;
    struct wlr_drm_format format;

    struct swapchain_slot slots[SWAPCHAIN_CAP];

    struct wl_listener allocator_destroy;
};

static void swapchain_handle_allocator_destroy(struct wl_listener *listener, void *data)
{
    struct swapchain *swapchain = wl_container_of(listener, swapchain, allocator_destroy);
    swapchain->allocator = NULL;
    wl_list_remove(&swapchain->allocator_destroy.link);
    wl_list_init(&swapchain->allocator_destroy.link);
}

static bool drm_format_copy(struct wlr_drm_format *dst, const struct wlr_drm_format *src)
{
    assert(src->len <= src->capacity);

    uint64_t *modifiers = malloc(sizeof(*modifiers) * src->len);
    if (!modifiers) {
        return false;
    }

    memcpy(modifiers, src->modifiers, sizeof(*modifiers) * src->len);

    wlr_drm_format_finish(dst);
    dst->capacity = src->len;
    dst->len = src->len;
    dst->format = src->format;
    dst->modifiers = modifiers;
    return true;
}

struct swapchain *allocator_create_swapchain(struct wlr_allocator *alloc,
                                             enum allocator_buffer_type type, int width, int height,
                                             const struct wlr_drm_format *format, uint32_t flags)
{
    struct swapchain *swapchain = calloc(1, sizeof(*swapchain));
    if (!swapchain) {
        return NULL;
    }

    swapchain->allocator = alloc;
    swapchain->type = type;
    swapchain->width = width;
    swapchain->height = height;
    swapchain->flags = flags;

    if (!drm_format_copy(&swapchain->format, format)) {
        free(swapchain);
        return NULL;
    }

    swapchain->allocator_destroy.notify = swapchain_handle_allocator_destroy;
    wl_signal_add(&alloc->events.destroy, &swapchain->allocator_destroy);

    return swapchain;
}

static void slot_reset(struct swapchain_slot *slot)
{
    if (slot->acquired) {
        wl_list_remove(&slot->release.link);
    }
    wlr_buffer_drop(slot->buffer);
    *slot = (struct swapchain_slot){ 0 };
}

void swapchain_destroy(struct swapchain *swapchain)
{
    if (!swapchain) {
        return;
    }

    for (size_t i = 0; i < SWAPCHAIN_CAP; i++) {
        slot_reset(&swapchain->slots[i]);
    }

    wl_list_remove(&swapchain->allocator_destroy.link);
    wlr_drm_format_finish(&swapchain->format);
    free(swapchain);
}

static void slot_handle_release(struct wl_listener *listener, void *data)
{
    struct swapchain_slot *slot = wl_container_of(listener, slot, release);
    wl_list_remove(&slot->release.link);
    slot->acquired = false;
}

static struct wlr_buffer *slot_acquire(struct swapchain *swapchain, struct swapchain_slot *slot,
                                       int *age)
{
    assert(!slot->acquired);
    assert(slot->buffer != NULL);

    slot->acquired = true;

    slot->release.notify = slot_handle_release;
    wl_signal_add(&slot->buffer->events.release, &slot->release);

    if (age != NULL) {
        *age = slot->age;
    }

    return wlr_buffer_lock(slot->buffer);
}

struct wlr_buffer *swapchain_acquire(struct swapchain *swapchain, int *age)
{
    struct swapchain_slot *free_slot = NULL;
    for (size_t i = 0; i < SWAPCHAIN_CAP; i++) {
        struct swapchain_slot *slot = &swapchain->slots[i];
        if (slot->acquired) {
            continue;
        }
        if (slot->buffer != NULL) {
            return slot_acquire(swapchain, slot, age);
        }
        free_slot = slot;
    }
    if (!free_slot) {
        kywc_log(KYWC_ERROR, "No free output buffer slot");
        return NULL;
    }

    if (!swapchain->allocator) {
        return NULL;
    }

    kywc_log(KYWC_INFO, "Allocating new swapchain buffer");
    free_slot->buffer =
        allocator_create_buffer(swapchain->allocator, swapchain->type, swapchain->width,
                                swapchain->height, &swapchain->format, swapchain->flags);
    if (!free_slot->buffer) {
        kywc_log(KYWC_ERROR, "Failed to allocate buffer");
        return NULL;
    }
    return slot_acquire(swapchain, free_slot, age);
}

static bool swapchain_has_buffer(struct swapchain *swapchain, struct wlr_buffer *buffer)
{
    for (size_t i = 0; i < SWAPCHAIN_CAP; i++) {
        struct swapchain_slot *slot = &swapchain->slots[i];
        if (slot->buffer == buffer) {
            return true;
        }
    }
    return false;
}

void swapchain_set_buffer_submitted(struct swapchain *swapchain, struct wlr_buffer *buffer)
{
    assert(buffer != NULL);

    if (!swapchain_has_buffer(swapchain, buffer)) {
        return;
    }

    // See the algorithm described in:
    // https://www.khronos.org/registry/EGL/extensions/EXT/EGL_EXT_buffer_age.txt
    for (size_t i = 0; i < SWAPCHAIN_CAP; i++) {
        struct swapchain_slot *slot = &swapchain->slots[i];
        if (slot->buffer == buffer) {
            slot->age = 1;
        } else if (slot->age > 0) {
            slot->age++;
        }
    }
}
