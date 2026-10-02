// SPDX-FileCopyrightText: 2024 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "presentation-time-protocol.h"
#include "scene_p.h"
#include "util/macros.h"

#define PRESENTATION_VERSION 2

struct presentation_feedback {
    struct wl_list resources; // wl_resource_get_link()

    // Only when the presentation_surface_textured_on_output() or
    // presentation_surface_scanned_out_on_output() helper has been called.
    struct wlr_output *output;
    struct wl_listener output_commit;
    struct wl_listener output_present;
    struct wl_listener output_destroy;

    bool output_committed;
    uint32_t output_commit_seq;
    bool zero_copy;
};

struct presentation_event {
    struct wlr_output *output;
    uint64_t tv_sec;
    uint32_t tv_nsec;
    uint32_t refresh;
    uint64_t seq;
    uint32_t flags; // enum wp_presentation_feedback_kind
};

struct presentation_surface_state {
    struct presentation_feedback *feedback;
};

struct presentation_surface {
    struct wlr_surface *surface;
    struct wl_listener surface_commit;
    struct wlr_addon addon;

    struct presentation_surface_state current, pending;
};

struct presentation {
    struct wl_global *global;
    struct wl_listener display_destroy;
};

static void feedback_handle_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static void feedback_resource_send_presented(struct wl_resource *feedback_resource,
                                             const struct presentation_event *event)
{
    struct wl_client *client = wl_resource_get_client(feedback_resource);
    struct wl_resource *output_resource;
    wl_resource_for_each(output_resource, &event->output->resources) {
        if (wl_resource_get_client(output_resource) == client) {
            wp_presentation_feedback_send_sync_output(feedback_resource, output_resource);
        }
    }

    uint32_t tv_sec_hi = event->tv_sec >> 32;
    uint32_t tv_sec_lo = event->tv_sec & 0xFFFFFFFF;
    uint32_t seq_hi = event->seq >> 32;
    uint32_t seq_lo = event->seq & 0xFFFFFFFF;
    wp_presentation_feedback_send_presented(feedback_resource, tv_sec_hi, tv_sec_lo, event->tv_nsec,
                                            event->refresh, seq_hi, seq_lo, event->flags);

    wl_resource_destroy(feedback_resource);
}

static void feedback_resource_send_discarded(struct wl_resource *feedback_resource)
{
    wp_presentation_feedback_send_discarded(feedback_resource);
    wl_resource_destroy(feedback_resource);
}

static void feedback_unset_output(struct presentation_feedback *feedback)
{
    if (!feedback->output) {
        return;
    }

    feedback->output = NULL;
    wl_list_remove(&feedback->output_commit.link);
    wl_list_remove(&feedback->output_present.link);
    wl_list_remove(&feedback->output_destroy.link);
}

static void presentation_feedback_destroy(struct presentation_feedback *feedback)
{
    if (!feedback) {
        return;
    }

    struct wl_resource *resource, *tmp;
    wl_resource_for_each_safe(resource, tmp, &feedback->resources) {
        feedback_resource_send_discarded(resource);
    }
    assert(wl_list_empty(&feedback->resources));

    feedback_unset_output(feedback);
    free(feedback);
}

static void presentation_surface_addon_destroy(struct wlr_addon *addon)
{
    struct presentation_surface *p_surface = wl_container_of(addon, p_surface, addon);

    wlr_addon_finish(addon);
    presentation_feedback_destroy(p_surface->current.feedback);
    presentation_feedback_destroy(p_surface->pending.feedback);

    wl_list_remove(&p_surface->surface_commit.link);
    free(p_surface);
}

static const struct wlr_addon_interface presentation_surface_addon_impl = {
    .name = "wlr_presentation_surface",
    .destroy = presentation_surface_addon_destroy,
};

static void presentation_surface_handle_surface_commit(struct wl_listener *listener, void *data)
{
    struct presentation_surface *p_surface = wl_container_of(listener, p_surface, surface_commit);

    presentation_feedback_destroy(p_surface->current.feedback);
    p_surface->current.feedback = p_surface->pending.feedback;
    p_surface->pending.feedback = NULL;
}

static void presentation_handle_feedback(struct wl_client *client,
                                         struct wl_resource *presentation_resource,
                                         struct wl_resource *surface_resource, uint32_t id)
{
    struct wlr_surface *surface = wlr_surface_from_resource(surface_resource);

    struct wlr_addon *addon =
        wlr_addon_find(&surface->addons, NULL, &presentation_surface_addon_impl);
    struct presentation_surface *p_surface = NULL;
    if (addon) {
        p_surface = wl_container_of(addon, p_surface, addon);
    } else {
        p_surface = calloc(1, sizeof(*p_surface));
        if (!p_surface) {
            wl_client_post_no_memory(client);
            return;
        }
        wlr_addon_init(&p_surface->addon, &surface->addons, NULL, &presentation_surface_addon_impl);
        p_surface->surface_commit.notify = presentation_surface_handle_surface_commit;
        wl_signal_add(&surface->events.commit, &p_surface->surface_commit);
    }

    struct presentation_feedback *feedback = p_surface->pending.feedback;
    if (!feedback) {
        feedback = calloc(1, sizeof(*feedback));
        if (!feedback) {
            wl_client_post_no_memory(client);
            return;
        }

        wl_list_init(&feedback->resources);
        p_surface->pending.feedback = feedback;
    }

    uint32_t version = wl_resource_get_version(presentation_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &wp_presentation_feedback_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, NULL, feedback, feedback_handle_resource_destroy);
    wl_list_insert(&feedback->resources, wl_resource_get_link(resource));
}

static void presentation_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct wp_presentation_interface presentation_impl = {
    .feedback = presentation_handle_feedback,
    .destroy = presentation_handle_destroy,
};

static void presentation_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &wp_presentation_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &presentation_impl, NULL, NULL);
    wp_presentation_send_clock_id(resource, CLOCK_MONOTONIC);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct presentation *presentation = wl_container_of(listener, presentation, display_destroy);
    wl_list_remove(&presentation->display_destroy.link);
    wl_global_destroy(presentation->global);
    free(presentation);
}

bool presentation_create(struct wl_display *display)
{
    struct presentation *presentation = calloc(1, sizeof(*presentation));
    if (!presentation) {
        return false;
    }

    presentation->global = wl_global_create(
        display, &wp_presentation_interface,
        MIN(wp_presentation_interface.version, PRESENTATION_VERSION), NULL, presentation_bind);
    if (!presentation->global) {
        kywc_log(KYWC_WARN, "WP presentation create failed");
        free(presentation);
        return NULL;
    }

    presentation->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(display, &presentation->display_destroy);

    return true;
}

static void feedback_handle_output_commit(struct wl_listener *listener, void *data)
{
    struct presentation_feedback *feedback = wl_container_of(listener, feedback, output_commit);
    if (feedback->output_committed) {
        return;
    }
    feedback->output_committed = true;
    feedback->output_commit_seq = feedback->output->commit_seq;
}

static void feedback_handle_output_present(struct wl_listener *listener, void *data)
{
    struct presentation_feedback *feedback = wl_container_of(listener, feedback, output_present);
    struct wlr_output_event_present *output_event = data;

    if (!feedback->output_committed || output_event->commit_seq != feedback->output_commit_seq) {
        return;
    }

    if (output_event->presented) {
        struct presentation_event event = {
            .output = output_event->output,
            .tv_sec = output_event->when->tv_sec,
            .tv_nsec = output_event->when->tv_nsec,
            .refresh = output_event->refresh,
            .seq = output_event->seq,
            .flags = output_event->flags,
        };
        struct wl_resource *resource = wl_resource_from_link(feedback->resources.next);
        if (wl_resource_get_version(resource) == 1 &&
            event.output->adaptive_sync_status == WLR_OUTPUT_ADAPTIVE_SYNC_ENABLED) {
            event.refresh = 0;
        }
        if (!feedback->zero_copy) {
            event.flags &= ~WP_PRESENTATION_FEEDBACK_KIND_ZERO_COPY;
        }
        struct wl_resource *res, *tmp;
        wl_resource_for_each_safe(res, tmp, &feedback->resources) {
            feedback_resource_send_presented(res, &event);
        }
    }
    presentation_feedback_destroy(feedback);
}

static void feedback_handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct presentation_feedback *feedback = wl_container_of(listener, feedback, output_destroy);
    presentation_feedback_destroy(feedback);
}

static struct presentation_feedback *presentation_surface_sampled(struct wlr_surface *surface)
{
    struct wlr_addon *addon =
        wlr_addon_find(&surface->addons, NULL, &presentation_surface_addon_impl);
    if (addon) {
        struct presentation_surface *p_surface = wl_container_of(addon, p_surface, addon);
        struct presentation_feedback *sampled = p_surface->current.feedback;
        p_surface->current.feedback = NULL;
        return sampled;
    }
    return NULL;
}

void scene_surface_presented_on_output(struct wlr_surface *surface, struct wlr_output *output,
                                       bool zero_copy)
{
    struct presentation_feedback *feedback = presentation_surface_sampled(surface);
    if (!feedback) {
        return;
    }

    assert(feedback->output == NULL);
    feedback->output = output;
    feedback->zero_copy = zero_copy;

    feedback->output_commit.notify = feedback_handle_output_commit;
    wl_signal_add(&output->events.commit, &feedback->output_commit);
    feedback->output_present.notify = feedback_handle_output_present;
    wl_signal_add(&output->events.present, &feedback->output_present);
    feedback->output_destroy.notify = feedback_handle_output_destroy;
    wl_signal_add(&output->events.destroy, &feedback->output_destroy);
}
