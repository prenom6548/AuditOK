// SPDX-FileCopyrightText: 2025 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <drm_fourcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <xf86drm.h>

#include <kywc/log.h>

#include "drm_p.h"

static bool state_create_mode_blob(const struct drm_connector_state *state, uint32_t *blob_id)
{
    if (!state->active) {
        *blob_id = 0;
        return true;
    }

    struct drm_device *drm = state->connector->drm;
    if (drmModeCreatePropertyBlob(drm->fd, &state->mode, sizeof(drmModeModeInfo), blob_id)) {
        kywc_log_errno(KYWC_ERROR, "Unable to create mode property blob");
        return false;
    }

    return true;
}

static bool state_create_rgb_ctm_blob(const struct drm_connector_state *state, uint32_t *blob_id)
{
    const struct drm_color_ctm *ctm = &state->output_state->rgb_ctm;
    if (ctm->matrix[0] == ctm->matrix[4] && ctm->matrix[0] == ctm->matrix[8] &&
        ctm->matrix[0] == (uint64_t)1 << 32) {
        *blob_id = 0;
        kywc_log(KYWC_DEBUG, "Default ctm");
        return true;
    }

    struct drm_device *drm = state->connector->drm;
    if (drmModeCreatePropertyBlob(drm->fd, ctm, sizeof(*ctm), blob_id) != 0) {
        kywc_log_errno(KYWC_ERROR, "Unable to creat clolor CTM property blob");
        return false;
    }

    return true;
}

static bool state_create_gamma_lut_blob(const struct drm_connector_state *state, uint32_t *blob_id)
{
    size_t lut_size = state->output_state->lut_size;
    if (lut_size == 0) {
        *blob_id = 0;
        return true;
    }

    const struct drm_color_lut *lut = state->output_state->lut;
    struct drm_device *drm = state->connector->drm;
    if (drmModeCreatePropertyBlob(drm->fd, lut, lut_size * sizeof(*lut), blob_id) != 0) {
        kywc_log_errno(KYWC_ERROR, "Unable to create gamma LUT property blob");
        return false;
    }

    return true;
}

static bool state_create_fb_damage_clips_blob(const struct drm_connector_state *state,
                                              uint32_t *blob_id)
{
    const pixman_region32_t *damage = &state->base->damage;
    if (!pixman_region32_not_empty(damage)) {
        *blob_id = 0;
        return true;
    }

    pixman_region32_t clipped;
    pixman_region32_init(&clipped);
    uint32_t width = state->primary_fb->wlr_buf->width;
    uint32_t height = state->primary_fb->wlr_buf->height;
    pixman_region32_intersect_rect(&clipped, damage, 0, 0, width, height);

    int rects_len;
    const pixman_box32_t *rects = pixman_region32_rectangles(&clipped, &rects_len);
    struct drm_device *drm = state->connector->drm;
    int ret = drmModeCreatePropertyBlob(drm->fd, rects, sizeof(*rects) * rects_len, blob_id);
    pixman_region32_fini(&clipped);
    if (ret != 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to create FB_DAMAGE_CLIPS property blob");
        return false;
    }

    return true;
}

static bool connector_cursor_is_visible(struct drm_connector *conn)
{
    return conn->cursor_enabled && conn->cursor_x < conn->output.width &&
           conn->cursor_y < conn->output.height && conn->cursor_x + conn->cursor_width >= 0 &&
           conn->cursor_y + conn->cursor_height >= 0;
}

static uint64_t max_bpc_for_format(uint32_t format)
{
    switch (format) {
    case DRM_FORMAT_XRGB2101010:
    case DRM_FORMAT_ARGB2101010:
    case DRM_FORMAT_XBGR2101010:
    case DRM_FORMAT_ABGR2101010:
        return 10;
    case DRM_FORMAT_XBGR16161616F:
    case DRM_FORMAT_ABGR16161616F:
    case DRM_FORMAT_XBGR16161616:
    case DRM_FORMAT_ABGR16161616:
        return 16;
    default:
        return 8;
    }
}

static uint64_t pick_max_bpc(struct drm_connector *conn, struct drm_fb *fb)
{
    uint32_t format = DRM_FORMAT_INVALID;
    struct wlr_dmabuf_attributes attribs = { 0 };
    if (wlr_buffer_get_dmabuf(fb->wlr_buf, &attribs)) {
        format = attribs.format;
    }

    uint64_t target_bpc = max_bpc_for_format(format);
    if (target_bpc < conn->max_bpc_bounds[0]) {
        target_bpc = conn->max_bpc_bounds[0];
    }
    if (target_bpc > conn->max_bpc_bounds[1]) {
        target_bpc = conn->max_bpc_bounds[1];
    }
    return target_bpc;
}

static void destroy_blob(struct drm_device *drm, uint32_t id)
{
    if (id == 0) {
        return;
    }

    if (drmModeDestroyPropertyBlob(drm->fd, id)) {
        kywc_log_errno(KYWC_ERROR, "Failed to destroy blob");
    }
}

static void commit_blob(struct drm_device *drm, uint32_t *current, uint32_t next)
{
    if (*current == next) {
        return;
    }
    if (*current != 0) {
        drmModeDestroyPropertyBlob(drm->fd, *current);
    }
    *current = next;
}

static void rollback_blob(struct drm_device *drm, uint32_t *current, uint32_t next)
{
    if (*current == next) {
        return;
    }
    if (next != 0) {
        drmModeDestroyPropertyBlob(drm->fd, next);
    }
}

/* legacy */
static bool legacy_fb_info_match(struct drm_fb *fb1, struct drm_fb *fb2)
{
    struct wlr_dmabuf_attributes dmabuf1 = { 0 }, dmabuf2 = { 0 };
    if (!wlr_buffer_get_dmabuf(fb1->wlr_buf, &dmabuf1) ||
        !wlr_buffer_get_dmabuf(fb2->wlr_buf, &dmabuf2)) {
        return false;
    }

    if (dmabuf1.width != dmabuf2.width || dmabuf1.height != dmabuf2.height ||
        dmabuf1.format != dmabuf2.format || dmabuf1.modifier != dmabuf2.modifier ||
        dmabuf1.n_planes != dmabuf2.n_planes) {
        return false;
    }

    for (int i = 0; i < dmabuf1.n_planes; i++) {
        if (dmabuf1.stride[i] != dmabuf2.stride[i] || dmabuf1.offset[i] != dmabuf2.offset[i]) {
            return false;
        }
    }

    return true;
}

static void fill_empty_gamma_table(size_t size, uint16_t *r, uint16_t *g, uint16_t *b)
{
    assert(0xFFFF < UINT64_MAX / (size - 1));
    for (uint32_t i = 0; i < size; ++i) {
        uint16_t val = (uint64_t)0xFFFF * i / (size - 1);
        r[i] = g[i] = b[i] = val;
    }
}

static bool legacy_crtc_set_gamma(struct drm_crtc *crtc, const struct drm_color_lut *lut,
                                  size_t size)
{
    uint16_t *linear_lut = NULL;
    if (size == 0) {
        // The legacy interface doesn't offer a way to reset the gamma LUT
        size = drm_get_crtc_gamma_lut_size(crtc);
        if (size == 0) {
            return false;
        }
    }

    linear_lut = malloc(3 * size * sizeof(uint16_t));
    if (linear_lut == NULL) {
        kywc_log_errno(KYWC_ERROR, "Allocation failed");
        return false;
    }

    uint16_t *r = linear_lut;
    uint16_t *g = linear_lut + size;
    uint16_t *b = linear_lut + 2 * size;

    if (lut) {
        for (uint32_t i = 0; i < size; ++i) {
            r[i] = lut[i].red;
            g[i] = lut[i].green;
            b[i] = lut[i].blue;
        }
    } else {
        // reset the gamma LUT
        fill_empty_gamma_table(size, r, g, b);
    }

    if (drmModeCrtcSetGamma(crtc->drm->fd, crtc->id, size, r, g, b) != 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to set gamma LUT on CRTC %" PRIu32, crtc->id);
        free(linear_lut);
        return false;
    }

    free(linear_lut);
    return true;
}

static bool legacy_crtc_test(const struct drm_connector_state *state, bool modeset)
{
    struct drm_crtc *crtc = state->connector->crtc;

    if (state->base->committed & WLR_OUTPUT_STATE_BUFFER) {
        int pending_width, pending_height;
        drm_output_pending_resolution(&state->connector->output, state->base, &pending_width,
                                      &pending_height);
        if (state->base->buffer->width != pending_width ||
            state->base->buffer->height != pending_height) {
            kywc_log(KYWC_DEBUG, "Primary buffer size mismatch");
            return false;
        }

        if (!modeset) {
            struct drm_fb *prev_fb = crtc->primary->queued_fb;
            if (!prev_fb) {
                prev_fb = crtc->primary->current_fb;
            }

            /* Legacy is only guaranteed to be able to display a FB if it's been
             * allocated the same way as the previous one. */

            struct drm_fb *pending_fb = state->primary_fb;
            if (prev_fb != NULL && !legacy_fb_info_match(prev_fb, pending_fb)) {
                kywc_log(KYWC_DEBUG,
                         "Cannot change scan-out buffer parameters with legacy KMS API");
                return false;
            }
        }
    }

    return true;
}

static bool legacy_crtc_commit(const struct drm_connector_state *state,
                               struct drm_page_flip *page_flip, uint32_t flags, bool modeset)
{
    struct drm_connector *conn = state->connector;
    struct drm_crtc *crtc = conn->crtc;
    struct drm_device *drm = conn->drm;

    uint32_t fb_id = 0;
    if (state->active) {
        if (state->primary_fb == NULL) {
            kywc_log(KYWC_ERROR, "%s: failed to acquire primary FB", conn->output.name);
            return false;
        }
        fb_id = state->primary_fb->id;
    }

    if (modeset) {
        if (drmModeConnectorSetProperty(drm->fd, conn->id, conn->props[DRM_CONNECTOR_PROP_DPMS].id,
                                        DRM_MODE_DPMS_OFF)) {
            kywc_log_errno(KYWC_ERROR, "Failed to set DPMS property");
            return false;
        }

        uint32_t *conns = NULL;
        size_t conns_len = 0;
        drmModeModeInfo *mode = NULL;
        if (state->active) {
            conns = &conn->id;
            conns_len = 1;
            mode = (drmModeModeInfo *)&state->mode;
        }
        if (drmModeSetCrtc(drm->fd, crtc->id, fb_id, 0, 0, conns, conns_len, mode)) {
            kywc_log_errno(KYWC_ERROR, "Failed to set CRTC");
            return false;
        }

        if (state->active &&
            drmModeConnectorSetProperty(drm->fd, conn->id, conn->props[DRM_CONNECTOR_PROP_DPMS].id,
                                        DRM_MODE_DPMS_ON)) {
            kywc_log_errno(KYWC_ERROR, "Failed to set DPMS property");
            return false;
        }
    }

    if (state->output_state->committed & DRM_OUTPUT_STATE_BRIGHTNESS &&
        conn->props[DRM_CONNECTOR_PROP_BRIGHTNESS].id) {
        if (drmModeConnectorSetProperty(drm->fd, conn->id,
                                        conn->props[DRM_CONNECTOR_PROP_BRIGHTNESS].id,
                                        state->output_state->brightness)) {
            kywc_log_errno(KYWC_ERROR, "Failed to set Brightness property");
            return false;
        }
        conn->brightness = state->output_state->brightness;
    }

    if (state->output_state->committed & DRM_OUTPUT_STATE_RGB_RANGE &&
        conn->props[DRM_CONNECTOR_PROP_RGB_RANGE].id) {
        if (drmModeConnectorSetProperty(drm->fd, conn->id,
                                        conn->props[DRM_CONNECTOR_PROP_RGB_RANGE].id,
                                        state->output_state->rgb_range)) {
            kywc_log_errno(KYWC_ERROR, "Failed to set Broadcast RGB property");
            return false;
        }

        conn->rgb_range = state->output_state->rgb_range;
    }

    if (state->output_state->committed & DRM_OUTPUT_STATE_OVERSCAN &&
        conn->props[DRM_CONNECTOR_PROP_OVERSCAN].id) {
        if (drmModeConnectorSetProperty(drm->fd, conn->id,
                                        conn->props[DRM_CONNECTOR_PROP_OVERSCAN].id,
                                        state->output_state->overscan)) {
            kywc_log_errno(KYWC_ERROR, "Failed to set Overscan property");
            return false;
        }

        conn->overscan = state->output_state->overscan;
    }

    if (state->output_state->committed & DRM_OUTPUT_STATE_SCALING_MODE &&
        conn->props[DRM_CONNECTOR_PROP_SCALING_MODE].id) {
        if (drmModeConnectorSetProperty(drm->fd, conn->id,
                                        conn->props[DRM_CONNECTOR_PROP_SCALING_MODE].id,
                                        state->output_state->scaling_mode)) {
            kywc_log_errno(KYWC_ERROR, "Failed to set Scaling mode property %d",
                           state->output_state->scaling_mode);
            return false;
        }

        conn->scaling_mode = state->output_state->scaling_mode;
    }

    if (state->output_state->committed & DRM_OUTPUT_STATE_GAMMA_LUT) {
        if (!legacy_crtc_set_gamma(crtc, state->output_state->lut, state->output_state->lut_size)) {
            return false;
        }
    }

    if (state->base->committed & WLR_OUTPUT_STATE_ADAPTIVE_SYNC_ENABLED) {
        if (!conn->adaptive_sync_supported && state->base->adaptive_sync_enabled) {
            return false;
        }
        if (crtc->props[DRM_CRTC_PROP_VRR_ENABLE].id &&
            drmModeObjectSetProperty(drm->fd, crtc->id, DRM_MODE_OBJECT_CRTC,
                                     crtc->props[DRM_CRTC_PROP_VRR_ENABLE].id,
                                     state->base->adaptive_sync_enabled) != 0) {
            kywc_log_errno(KYWC_ERROR, "Failed with drmModeObjectSetProperty(VRR_ENABLED)");
            return false;
        }
        conn->output.adaptive_sync_status = state->base->adaptive_sync_enabled
                                                ? WLR_OUTPUT_ADAPTIVE_SYNC_ENABLED
                                                : WLR_OUTPUT_ADAPTIVE_SYNC_DISABLED;
        kywc_log(KYWC_DEBUG, "VRR %s", state->base->adaptive_sync_enabled ? "enabled" : "disabled");
    }

    if (crtc->cursor && state->active && connector_cursor_is_visible(conn)) {
        struct drm_fb *cursor_fb = state->cursor_fb;
        if (cursor_fb == NULL) {
            kywc_log(KYWC_DEBUG, "Failed to acquire cursor FB");
            return false;
        }

        drmModeFB *drm_fb = drmModeGetFB(drm->fd, cursor_fb->id);
        if (drm_fb == NULL) {
            kywc_log_errno(KYWC_DEBUG, "Failed to get cursor BO handle: drmModeGetFB failed");
            return false;
        }
        uint32_t cursor_handle = drm_fb->handle;
        uint32_t cursor_width = drm_fb->width;
        uint32_t cursor_height = drm_fb->height;
        drmModeFreeFB(drm_fb);

        int ret = drmModeSetCursor(drm->fd, crtc->id, cursor_handle, cursor_width, cursor_height);
        int set_errno = errno;
        if (drmCloseBufferHandle(drm->fd, cursor_handle) != 0) {
            kywc_log_errno(KYWC_ERROR, "Failed with drmCloseBufferHandle");
        }
        if (ret != 0) {
            kywc_log(KYWC_DEBUG, "Failed with drmModeSetCursor failed: %s", strerror(set_errno));
            return false;
        }

        if (drmModeMoveCursor(drm->fd, crtc->id, conn->cursor_x, conn->cursor_y) != 0) {
            kywc_log_errno(KYWC_ERROR, "Failed with drmModeMoveCursor");
            return false;
        }
    } else {
        if (drmModeSetCursor(drm->fd, crtc->id, 0, 0, 0)) {
            kywc_log_errno_ratelimited(KYWC_WARN, "Failed with drmModeSetCursor");
            return false;
        }
    }

    if (flags & DRM_MODE_PAGE_FLIP_EVENT) {
        if (drmModePageFlip(drm->fd, crtc->id, fb_id, flags, page_flip)) {
            kywc_log_errno_ratelimited(KYWC_ERROR, "Failed with drmModePageFlip(crtc_id: %d)",
                                       crtc->id);
            return false;
        }
    }

    return true;
}

static bool legacy_kms(struct drm_device *drm, const struct drm_device_state *state,
                       struct drm_page_flip *page_flip, uint32_t flags, bool test_only)
{
    if (!legacy_crtc_test(state->conn_state, state->modeset)) {
        return false;
    }

    if (test_only) {
        return true;
    }

    if (!legacy_crtc_commit(state->conn_state, page_flip, flags, state->modeset)) {
        return false;
    }
    return true;
}

/* atomic */
struct atomic {
    drmModeAtomicReq *req;
    bool failed;
};

static char *atomic_commit_flags_str(uint32_t flags)
{
    const char *const l[] = {
        (flags & DRM_MODE_PAGE_FLIP_EVENT) ? "PAGE_FLIP_EVENT" : NULL,
        (flags & DRM_MODE_PAGE_FLIP_ASYNC) ? "PAGE_FLIP_ASYNC" : NULL,
        (flags & DRM_MODE_ATOMIC_TEST_ONLY) ? "ATOMIC_TEST_ONLY" : NULL,
        (flags & DRM_MODE_ATOMIC_NONBLOCK) ? "ATOMIC_NONBLOCK" : NULL,
        (flags & DRM_MODE_ATOMIC_ALLOW_MODESET) ? "ATOMIC_ALLOW_MODESET" : NULL,
    };

    char *buf = NULL;
    size_t size = 0;
    FILE *f = open_memstream(&buf, &size);
    if (f == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < sizeof(l) / sizeof(l[0]); i++) {
        if (l[i] == NULL) {
            continue;
        }
        if (ftell(f) > 0) {
            fprintf(f, " | ");
        }
        fprintf(f, "%s", l[i]);
    }

    if (ftell(f) == 0) {
        fprintf(f, "none");
    }

    fclose(f);

    return buf;
}

static void atomic_begin(struct atomic *atom)
{
    *atom = (struct atomic){ 0 };

    atom->req = drmModeAtomicAlloc();
    if (!atom->req) {
        kywc_log_errno(KYWC_ERROR, "Allocation failed");
        atom->failed = true;
    }
}

static void atomic_add(struct atomic *atom, uint32_t id, uint32_t prop, uint64_t val)
{
    if (!atom->failed && drmModeAtomicAddProperty(atom->req, id, prop, val) < 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to add atomic DRM property");
        atom->failed = true;
    }
}

static bool atomic_commit(struct atomic *atom, struct drm_device *drm,
                          struct drm_page_flip *page_flip, uint32_t flags)
{
    if (atom->failed) {
        return false;
    }

    int ret = drmModeAtomicCommit(drm->fd, atom->req, flags, page_flip);
    if (ret != 0) {
        kywc_log_errno_ratelimited(flags & DRM_MODE_ATOMIC_TEST_ONLY ? KYWC_WARN : KYWC_ERROR,
                                   "Atomic commit failed");
        char *flags_str = atomic_commit_flags_str(flags);
        kywc_log_ratelimited(KYWC_DEBUG, "(Atomic commit flags: %s)",
                             flags_str ? flags_str : "<error>");
        free(flags_str);
        return false;
    }

    return true;
}

static void atomic_finish(struct atomic *atom)
{
    drmModeAtomicFree(atom->req);
}

static void drm_kms_apply_atomic_commit(struct drm_connector_state *state)
{
    struct drm_connector *conn = state->connector;
    struct drm_crtc *crtc = conn->crtc;
    if (!crtc->own_mode_id) {
        crtc->mode_id = 0; // don't try to delete previous master's blobs
    }
    crtc->own_mode_id = true;
    commit_blob(conn->drm, &crtc->mode_id, state->mode_id);
    commit_blob(conn->drm, &crtc->gamma_lut, state->gamma_lut);
    commit_blob(conn->drm, &crtc->ctm, state->rgb_ctm);

    conn->output.adaptive_sync_status =
        state->vrr_enabled ? WLR_OUTPUT_ADAPTIVE_SYNC_ENABLED : WLR_OUTPUT_ADAPTIVE_SYNC_DISABLED;

    conn->brightness = state->brightness;
    conn->rgb_range = state->rgb_range;
    conn->overscan = state->overscan;
    conn->scaling_mode = state->scaling_mode;

    destroy_blob(conn->drm, state->fb_damage_clips);
}

static void drm_kms_rollback_atomic_commit(struct drm_connector_state *state)
{
    struct drm_connector *conn = state->connector;
    struct drm_crtc *crtc = conn->crtc;

    rollback_blob(conn->drm, &crtc->mode_id, state->mode_id);
    rollback_blob(conn->drm, &crtc->gamma_lut, state->gamma_lut);
    rollback_blob(conn->drm, &crtc->ctm, state->rgb_ctm);

    destroy_blob(conn->drm, state->fb_damage_clips);
}

static bool atomic_connector_prepare(struct drm_connector_state *state, bool modeset)
{
    struct drm_connector *conn = state->connector;
    struct drm_crtc *crtc = conn->crtc;

    uint32_t mode_id = crtc->mode_id;
    if (modeset) {
        if (!state_create_mode_blob(state, &mode_id)) {
            return false;
        }
    }

    uint32_t gamma_lut = crtc->gamma_lut;
    if (state->output_state->committed & DRM_OUTPUT_STATE_GAMMA_LUT) {
        // Fallback to legacy gamma interface when gamma properties are not
        // available (can happen on older Intel GPUs that support gamma but not
        // degamma).
        if (crtc->props[DRM_CRTC_PROP_GAMMA_LUT].id == 0) {
            if (!legacy_crtc_set_gamma(crtc, state->output_state->lut,
                                       state->output_state->lut_size)) {
                return false;
            }
        } else {
            if (!state_create_gamma_lut_blob(state, &gamma_lut)) {
                return false;
            }
        }
    }

    uint32_t rgb_ctm = crtc->ctm;
    if (state->output_state->committed & DRM_OUTPUT_STATE_RGB_CTM &&
        crtc->props[DRM_CRTC_PROP_CTM].id) {
        if (!state_create_rgb_ctm_blob(state, &rgb_ctm)) {
            return false;
        }
    }

    uint32_t fb_damage_clips = 0;
    if ((state->base->committed & WLR_OUTPUT_STATE_DAMAGE) &&
        crtc->primary->props[DRM_PLANE_PROP_FB_DAMAGE_CLIPS].id != 0) {
        state_create_fb_damage_clips_blob(state, &fb_damage_clips);
    }

    bool prev_vrr_enabled = conn->output.adaptive_sync_status == WLR_OUTPUT_ADAPTIVE_SYNC_ENABLED;
    bool vrr_enabled = prev_vrr_enabled;
    if ((state->base->committed & WLR_OUTPUT_STATE_ADAPTIVE_SYNC_ENABLED)) {
        if (!conn->adaptive_sync_supported) {
            return false;
        }
        vrr_enabled = state->base->adaptive_sync_enabled;
    }

    uint32_t brightness = conn->brightness;
    if (state->output_state->committed & DRM_OUTPUT_STATE_BRIGHTNESS &&
        conn->props[DRM_CONNECTOR_PROP_BRIGHTNESS].id) {
        brightness = state->output_state->brightness;
    }

    uint32_t rgb_range = conn->rgb_range;
    if (state->output_state->committed & DRM_OUTPUT_STATE_RGB_RANGE &&
        conn->props[DRM_CONNECTOR_PROP_RGB_RANGE].id) {
        rgb_range = state->output_state->rgb_range;
    }

    uint32_t overscan = conn->overscan;
    if (state->output_state->committed & DRM_OUTPUT_STATE_OVERSCAN &&
        conn->props[DRM_CONNECTOR_PROP_OVERSCAN].id) {
        overscan = state->output_state->overscan;
    }

    uint32_t scaling_mode = conn->scaling_mode;
    if (state->output_state->committed & DRM_OUTPUT_STATE_SCALING_MODE &&
        conn->props[DRM_CONNECTOR_PROP_SCALING_MODE].id) {
        scaling_mode = state->output_state->scaling_mode;
    }

    state->mode_id = mode_id;
    state->gamma_lut = gamma_lut;
    state->fb_damage_clips = fb_damage_clips;
    state->vrr_enabled = vrr_enabled;
    state->rgb_ctm = rgb_ctm;
    state->brightness = brightness;
    state->rgb_range = rgb_range;
    state->overscan = overscan;
    state->scaling_mode = scaling_mode;

    return true;
}

static bool cursor_plane_support_hotspots(const struct drm_plane *plane)
{
    return plane->props[DRM_PLANE_PROP_HOTSPOT_X].id && plane->props[DRM_PLANE_PROP_HOTSPOT_Y].id;
}

static void plane_disable(struct atomic *atom, struct drm_plane *plane)
{
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_FB_ID].id, 0);
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_CRTC_ID].id, 0);
}

static void plane_set_properties(struct atomic *atom, struct drm_plane *plane, struct drm_fb *fb,
                                 uint32_t crtc_id, int32_t x, int32_t y)
{
    if (fb == NULL) {
        kywc_log(KYWC_ERROR, "Failed to acquire FB for plane %" PRIu32, plane->id);
        atom->failed = true;
        return;
    }

    uint32_t width = fb->wlr_buf->width;
    uint32_t height = fb->wlr_buf->height;

    width = plane->enable_yuyv ? width * 2 : width;

    // The src_* properties are in 16.16 fixed point
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_SRC_X].id, 0);
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_SRC_Y].id, 0);
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_SRC_W].id, (uint64_t)width << 16);
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_SRC_H].id, (uint64_t)height << 16);
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_CRTC_W].id, width);
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_CRTC_H].id, height);
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_FB_ID].id, fb->id);
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_CRTC_ID].id, crtc_id);
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_CRTC_X].id, (uint64_t)x);
    atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_CRTC_Y].id, (uint64_t)y);
}

static void plane_set_color_properties(struct atomic *atom, struct drm_plane *plane,
                                       uint32_t color_encode, uint32_t color_range)
{
    if (plane->type != DRM_PLANE_TYPE_PRIMARY || !plane->enable_yuyv) {
        return;
    }

    uint32_t prop_encode = plane->props[DRM_PLANE_PROP_COLOR_ENCODING].id;
    if (prop_encode) {
        atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_COLOR_ENCODING].id, color_encode);
    }
    uint32_t prop_range = plane->props[DRM_PLANE_PROP_COLOR_RANGE].id;
    if (prop_range) {
        atomic_add(atom, plane->id, plane->props[DRM_PLANE_PROP_COLOR_RANGE].id, color_range);
    }
}

static void atomic_connector_add(struct atomic *atom, struct drm_connector_state *state,
                                 bool modeset)
{
    struct drm_connector *conn = state->connector;
    struct drm_crtc *crtc = conn->crtc;
    bool active = state->active;

    atomic_add(atom, conn->id, conn->props[DRM_CONNECTOR_PROP_CRTC_ID].id, active ? crtc->id : 0);

    uint32_t link_state = conn->props[DRM_CONNECTOR_PROP_LINK_STATUS].id;
    if (modeset && active && link_state) {
        atomic_add(atom, conn->id, link_state, DRM_MODE_LINK_STATUS_GOOD);
    }
    uint32_t content_type = conn->props[DRM_CONNECTOR_PROP_CONTENT_TYPE].id;
    if (active && content_type) {
        atomic_add(atom, conn->id, content_type, DRM_MODE_CONTENT_TYPE_GRAPHICS);
    }
    uint32_t max_bpc = conn->props[DRM_CONNECTOR_PROP_MAX_BPC].id;
    if (modeset && active && max_bpc && conn->max_bpc_bounds[1]) {
        atomic_add(atom, conn->id, max_bpc, pick_max_bpc(conn, state->primary_fb));
    }

    atomic_add(atom, crtc->id, crtc->props[DRM_CRTC_PROP_MODE_ID].id, state->mode_id);
    atomic_add(atom, crtc->id, crtc->props[DRM_CRTC_PROP_ACTIVE].id, active);

    if (!active) {
        plane_disable(atom, crtc->primary);
        if (crtc->cursor) {
            plane_disable(atom, crtc->cursor);
        }
        return;
    }

    uint32_t brightness = conn->props[DRM_CONNECTOR_PROP_BRIGHTNESS].id;
    if (brightness) {
        atomic_add(atom, conn->id, brightness, state->brightness);
    }

    uint32_t rgb_range = conn->props[DRM_CONNECTOR_PROP_RGB_RANGE].id;
    if (rgb_range) {
        atomic_add(atom, conn->id, rgb_range, state->rgb_range);
    }

    uint32_t overscan = conn->props[DRM_CONNECTOR_PROP_OVERSCAN].id;
    if (overscan) {
        atomic_add(atom, conn->id, overscan, state->overscan);
    }

    uint32_t scaling_mode = conn->props[DRM_CONNECTOR_PROP_SCALING_MODE].id;
    if (scaling_mode) {
        atomic_add(atom, conn->id, scaling_mode, state->scaling_mode);
    }

    uint32_t gamma_lut = crtc->props[DRM_CRTC_PROP_GAMMA_LUT].id;
    if (gamma_lut) {
        atomic_add(atom, crtc->id, gamma_lut, state->gamma_lut);
    }

    uint32_t rgb_ctm = crtc->props[DRM_CRTC_PROP_CTM].id;
    if (rgb_ctm) {
        atomic_add(atom, crtc->id, rgb_ctm, state->rgb_ctm);
    }

    uint32_t vrr_enabled = crtc->props[DRM_CRTC_PROP_VRR_ENABLE].id;
    if (vrr_enabled) {
        atomic_add(atom, crtc->id, vrr_enabled, state->vrr_enabled);
    }

    plane_set_color_properties(atom, crtc->primary, DRM_COLOR_ENCODING_BT601,
                               DRM_COLOR_RANGE_LIMITTED);
    plane_set_properties(atom, crtc->primary, state->primary_fb, crtc->id, 0, 0);
    uint32_t fb_damage_clips = crtc->primary->props[DRM_PLANE_PROP_FB_DAMAGE_CLIPS].id;
    if (fb_damage_clips) {
        atomic_add(atom, crtc->primary->id, fb_damage_clips, state->fb_damage_clips);
    }

    if (!crtc->cursor) {
        return;
    }

    if (connector_cursor_is_visible(conn)) {
        plane_set_properties(atom, crtc->cursor, state->cursor_fb, crtc->id, conn->cursor_x,
                             conn->cursor_y);
        if (cursor_plane_support_hotspots(crtc->cursor)) {
            uint32_t hotspot_x = crtc->cursor->props[DRM_PLANE_PROP_HOTSPOT_X].id;
            uint32_t hotspot_y = crtc->cursor->props[DRM_PLANE_PROP_HOTSPOT_Y].id;
            atomic_add(atom, crtc->cursor->id, hotspot_x, conn->cursor_hotspot_x);
            atomic_add(atom, crtc->cursor->id, hotspot_y, conn->cursor_hotspot_y);
        }
    } else {
        plane_disable(atom, crtc->cursor);
    }
}

static bool atomic_kms(struct drm_device *drm, const struct drm_device_state *state,
                       struct drm_page_flip *page_flip, uint32_t flags, bool test_only)
{
    bool ok = false;
    if (!atomic_connector_prepare(state->conn_state, state->modeset)) {
        goto out;
    }

    struct atomic atom;
    atomic_begin(&atom);

    atomic_connector_add(&atom, state->conn_state, state->modeset);
    if (test_only) {
        flags |= DRM_MODE_ATOMIC_TEST_ONLY;
    }
    if (state->modeset) {
        flags |= DRM_MODE_ATOMIC_ALLOW_MODESET;
    }
    if (!test_only && state->nonblock) {
        flags |= DRM_MODE_ATOMIC_NONBLOCK;
    }

    ok = atomic_commit(&atom, drm, page_flip, flags);
    atomic_finish(&atom);

out:
    if (ok && !test_only) {
        drm_kms_apply_atomic_commit(state->conn_state);
    } else {
        drm_kms_rollback_atomic_commit(state->conn_state);
    }
    return ok;
}

bool drm_kms_commit(struct drm_device *drm, const struct drm_device_state *state,
                    struct drm_page_flip *page_flip, uint32_t flags, bool test_only)
{
    switch (drm->mode) {
    case DRM_KMS_MODE_ATOMIC:
        return atomic_kms(drm, state, page_flip, flags, test_only);
        break;
    case DRM_KMS_MODE_LEGACY:
        return legacy_kms(drm, state, page_flip, flags, test_only);
        break;
    default:
        break;
    }
    return true;
}
