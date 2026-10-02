// SPDX-FileCopyrightText: 2025 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <wlr/interfaces/wlr_output.h>
#include <wlr/render/allocator.h>
#include <wlr/render/swapchain.h>

#include <kywc/log.h>
#include <util/macros.h>

#include "drm_p.h"
#include "output.h"
#include "render/drm_syncobj.h"
#include "render/pixel_format.h"
#include "render/pixman.h"
#include "render/renderer.h"

// Output state which needs a KMS commit to be applied
static const uint32_t COMMIT_OUTPUT_STATE = WLR_OUTPUT_STATE_BUFFER | WLR_OUTPUT_STATE_MODE |
                                            WLR_OUTPUT_STATE_ENABLED | WLR_OUTPUT_STATE_GAMMA_LUT |
                                            WLR_OUTPUT_STATE_ADAPTIVE_SYNC_ENABLED;

static const uint32_t SUPPORTED_OUTPUT_STATE =
    WLR_OUTPUT_STATE_BACKEND_OPTIONAL | COMMIT_OUTPUT_STATE;

static const int32_t subpixel_map[] = {
    [DRM_MODE_SUBPIXEL_UNKNOWN] = WL_OUTPUT_SUBPIXEL_UNKNOWN,
    [DRM_MODE_SUBPIXEL_HORIZONTAL_RGB] = WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB,
    [DRM_MODE_SUBPIXEL_HORIZONTAL_BGR] = WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR,
    [DRM_MODE_SUBPIXEL_VERTICAL_RGB] = WL_OUTPUT_SUBPIXEL_VERTICAL_RGB,
    [DRM_MODE_SUBPIXEL_VERTICAL_BGR] = WL_OUTPUT_SUBPIXEL_VERTICAL_BGR,
    [DRM_MODE_SUBPIXEL_NONE] = WL_OUTPUT_SUBPIXEL_NONE,
};

static struct drm_device *drm_device_get_parent(struct drm_device *drm)
{
    struct drm_device *drm_parent = NULL;
    if (wlr_backend_is_drm(drm->wlr_backend)) {
        struct drm_backend *backend = drm_backend_from_wlr_backend(drm->wlr_backend)->parent;
        drm_parent = backend ? backend->drm : NULL;
    }
    return drm_parent;
}

static bool drm_check_features(struct drm_device *drm)
{
    if (drmGetCap(drm->fd, DRM_CAP_CURSOR_WIDTH, &drm->cursor_width)) {
        drm->cursor_width = 64;
    }
    if (drmGetCap(drm->fd, DRM_CAP_CURSOR_HEIGHT, &drm->cursor_height)) {
        drm->cursor_height = 64;
    }

    if (drmSetClientCap(drm->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1)) {
        kywc_log(KYWC_ERROR, "DRM universal planes unsupported");
        return false;
    }

    uint64_t cap;
    if (drmGetCap(drm->fd, DRM_CAP_CRTC_IN_VBLANK_EVENT, &cap) || !cap) {
        kywc_log(KYWC_ERROR, "DRM_CRTC_IN_VBLANK_EVENT unsupported");
        return false;
    }

    if (drmGetCap(drm->fd, DRM_CAP_TIMESTAMP_MONOTONIC, &cap) || !cap) {
        kywc_log(KYWC_ERROR, "DRM_CAP_TIMESTAMP_MONOTONIC unsupported");
        return false;
    }

    char *env = getenv("KYWC_DRM_NO_ATOMIC");
    if (env && strcmp(env, "1") == 0) {
        drm->mode = DRM_KMS_MODE_LEGACY;
        kywc_log(KYWC_DEBUG, "KYWC_DRM_NO_ATOMIC set, forcing legacy DRM interface");
    } else if (drmSetClientCap(drm->fd, DRM_CLIENT_CAP_ATOMIC, 1)) {
        drm->mode = DRM_KMS_MODE_LEGACY;
        kywc_log(KYWC_DEBUG, "Atomic modesetting unsupported, using legacy DRM interface");
    } else {
        drm->mode = DRM_KMS_MODE_ATOMIC;
        kywc_log(KYWC_DEBUG, "Using atomic DRM interface");
    }
#ifdef DRM_CLIENT_CAP_CURSOR_PLANE_HOTSPOT
    if (drm->mode == DRM_KMS_MODE_ATOMIC &&
        drmSetClientCap(drm->fd, DRM_CLIENT_CAP_CURSOR_PLANE_HOTSPOT, 1) == 0) {
        kywc_log(KYWC_INFO, "DRM_CLIENT_CAP_CURSOR_PLANE_HOTSPOT supported");
    }
#endif

    if (drm->mode == DRM_KMS_MODE_LEGACY) {
        drm->supports_tearing_page_flips =
            drmGetCap(drm->fd, DRM_CAP_ASYNC_PAGE_FLIP, &cap) == 0 && cap == 1;
    } else {
        drm->supports_tearing_page_flips =
            drmGetCap(drm->fd, DRM_CAP_ATOMIC_ASYNC_PAGE_FLIP, &cap) == 0 && cap == 1;
    }

    env = getenv("KYWC_DRM_NO_MODIFIERS");
    if (env && strcmp(env, "1") == 0) {
        kywc_log(KYWC_DEBUG, "KYWC_DRM_NO_MODIFIERS set, disabling modifiers");
    } else {
        int ret = drmGetCap(drm->fd, DRM_CAP_ADDFB2_MODIFIERS, &cap);
        drm->addfb2_modifiers = ret == 0 && cap == 1;
        kywc_log(KYWC_DEBUG, "ADDFB2 modifiers %s",
                 drm->addfb2_modifiers ? "supported" : "unsupported");
    }

    return true;
}

static bool init_plane(struct drm_device *drm, struct drm_plane *plane,
                       const drmModePlane *drm_plane)
{
    plane->id = drm_plane->plane_id;

    if (!drm_get_plane_props(drm->fd, plane->id, plane->props)) {
        return false;
    }

    uint64_t type;
    if (!drm_get_property_value(drm->fd, DRM_MODE_OBJECT_PLANE, plane->id,
                                &plane->props[DRM_PLANE_PROP_TYPE], &type)) {
        return false;
    }

    plane->type = type;
    plane->crtc_id = drm_plane->crtc_id;

    for (size_t i = 0; i < drm_plane->count_formats; ++i) {
        // Force a LINEAR layout for the cursor if the driver doesn't support modifiers
        wlr_drm_format_set_add(&plane->formats, drm_plane->formats[i], DRM_FORMAT_MOD_LINEAR);
        if (plane->type != DRM_PLANE_TYPE_CURSOR) {
            wlr_drm_format_set_add(&plane->formats, drm_plane->formats[i], DRM_FORMAT_MOD_INVALID);
        }
    }

    if (plane->props[DRM_PLANE_PROP_IN_FORMATS].id && drm->addfb2_modifiers) {
        uint64_t blob_id;
        if (!drm_get_property_value(drm->fd, DRM_MODE_OBJECT_PLANE, plane->id,
                                    &plane->props[DRM_PLANE_PROP_IN_FORMATS], &blob_id)) {
            kywc_log(KYWC_ERROR, "Failed to read IN_FORMATS property");
            return false;
        }
        drmModePropertyBlobRes *blob = drmModeGetPropertyBlob(drm->fd, blob_id);
        if (!blob) {
            kywc_log(KYWC_ERROR, "Failed to read IN_FORMATS blob");
            return false;
        }

        drmModeFormatModifierIterator iter = { 0 };
        while (drmModeFormatModifierBlobIterNext(blob, &iter)) {
            wlr_drm_format_set_add(&plane->formats, iter.fmt, iter.mod);
        }
        drmModeFreePropertyBlob(blob);
    }

    assert(drm->num_crtcs <= 32);
    for (size_t j = 0; j < drm->num_crtcs; j++) {
        uint32_t crtc_bit = 1 << j;
        if ((drm_plane->possible_crtcs & crtc_bit) == 0) {
            continue;
        }

        struct drm_crtc *crtc = &drm->crtcs[j];
        if (plane->type == DRM_PLANE_TYPE_PRIMARY && !crtc->primary) {
            crtc->primary = plane;
            break;
        }
        if (type == DRM_PLANE_TYPE_CURSOR && !crtc->cursor) {
            crtc->cursor = plane;
            break;
        }
    }

    return true;
}

static void add_cursor_plane(struct drm_device *drm, drmModePlaneRes *plane_res, uint32_t *crtcs)
{
    if (drm->mode != DRM_KMS_MODE_LEGACY) {
        return;
    }

    for (uint32_t i = 0; i < drm->num_planes; ++i) {
        uint32_t id = plane_res->planes[i];

        struct drm_prop_info props[DRM_PLANE_PROP_COUNT];
        if (!drm_get_plane_props(drm->fd, id, props)) {
            return;
        }

        uint64_t type;
        if (!drm_get_property_value(drm->fd, DRM_MODE_OBJECT_PLANE, id, &props[DRM_PLANE_PROP_TYPE],
                                    &type)) {
            return;
        }

        if (type == DRM_PLANE_TYPE_CURSOR) {
            // if cursor plane exits, do not add any
            return;
        }
    }

    // check each crtc whether hardware cursor is supported
    for (size_t i = 0; i < drm->num_crtcs; ++i) {
        if (!drmModeSetCursor(drm->fd, drm->crtcs[i].id, 0, 0, 0)) {
            kywc_log(KYWC_DEBUG, "Going to add cursor plane for crtc %d", drm->crtcs[i].id);
            ++drm->num_planes;
            *crtcs |= 1 << i;
        }
    }
}

static bool init_planes(struct drm_device *drm)
{
    drmModePlaneRes *plane_res = drmModeGetPlaneResources(drm->fd);
    if (!plane_res) {
        kywc_log_errno(KYWC_ERROR, "Failed to get DRM plane resources");
        return false;
    }

    kywc_log(KYWC_INFO, "Found %" PRIu32 " DRM planes", plane_res->count_planes);

    drm->num_planes = plane_res->count_planes;
    /**
     * when drm works on legacy instead of atomic,
     * hardware cursor do not work without cursor plane,
     * add cursor plane for crtcs support hardware cursor.
     *
     * drm->num_planes may increase here.
     */
    uint32_t crtcs = 0;
    add_cursor_plane(drm, plane_res, &crtcs);

    drm->planes = calloc(drm->num_planes, sizeof(*drm->planes));
    if (drm->planes == NULL) {
        kywc_log_errno(KYWC_ERROR, "Allocation failed");
        goto error;
    }

    for (uint32_t i = 0; i < plane_res->count_planes; ++i) {
        uint32_t id = plane_res->planes[i];

        drmModePlane *drm_plane = drmModeGetPlane(drm->fd, id);
        if (!drm_plane) {
            kywc_log_errno(KYWC_ERROR, "Failed to get DRM plane");
            goto error;
        }

        struct drm_plane *plane = &drm->planes[i];
        if (!init_plane(drm, plane, drm_plane)) {
            goto error;
        }

        drmModeFreePlane(drm_plane);
    }

    for (uint32_t i = 0, j = plane_res->count_planes; i < drm->num_crtcs; ++i) {
        if (~crtcs & 1 << i) {
            continue;
        }

        struct drm_plane *cursor_plane = &drm->planes[j++];
        // add format ARGB8888 for cursor plane
        wlr_drm_format_set_add(&cursor_plane->formats, DRM_FORMAT_ARGB8888, DRM_FORMAT_MOD_LINEAR);
        cursor_plane->type = DRM_PLANE_TYPE_CURSOR;

        drm->crtcs[i].cursor = cursor_plane;
    }

    drmModeFreePlaneResources(plane_res);
    return true;

error:
    free(drm->planes);
    drmModeFreePlaneResources(plane_res);
    return false;
}

static bool drm_init_resources(struct drm_device *drm)
{
    drmModeRes *res = drmModeGetResources(drm->fd);
    if (!res) {
        kywc_log_errno(KYWC_ERROR, "Failed to get DRM resources");
        return false;
    }

    kywc_log(KYWC_INFO, "Found %d DRM CRTCs", res->count_crtcs);

    drm->num_crtcs = res->count_crtcs;
    if (drm->num_crtcs == 0) {
        drmModeFreeResources(res);
        return true;
    }

    drm->crtcs = calloc(drm->num_crtcs, sizeof(drm->crtcs[0]));
    if (!drm->crtcs) {
        kywc_log_errno(KYWC_ERROR, "Allocation failed");
        goto error_res;
    }

    for (size_t i = 0; i < drm->num_crtcs; ++i) {
        struct drm_crtc *crtc = &drm->crtcs[i];
        crtc->drm = drm;
        crtc->id = res->crtcs[i];

        drmModeCrtc *drm_crtc = drmModeGetCrtc(drm->fd, crtc->id);
        if (drm_crtc == NULL) {
            kywc_log_errno(KYWC_ERROR, "Failed with drmModeGetCrtc");
            goto error_crtcs;
        }
        crtc->legacy_gamma_size = drm_crtc->gamma_size;
        drmModeFreeCrtc(drm_crtc);

        if (!drm_get_crtc_props(drm->fd, crtc->id, crtc->props)) {
            goto error_crtcs;
        }
    }

    if (!init_planes(drm)) {
        goto error_crtcs;
    }

    drmModeFreeResources(res);

    return true;

error_crtcs:
    free(drm->crtcs);
error_res:
    drmModeFreeResources(res);
    return false;
}

static void drm_destroy_page_flip(struct drm_page_flip *page_flip)
{
    if (!page_flip) {
        return;
    }

    wl_list_remove(&page_flip->link);
    free(page_flip);
}

static int mhz_to_nsec(int mhz)
{
    return 1000000000000LL / mhz;
}

static void drm_plane_fb_finish(struct drm_plane *plane)
{
    if (!plane) {
        return;
    }

    drm_fb_clear_fb(&plane->queued_fb);
    if (plane->queued_release_timeline != NULL) {
        drm_syncobj_timeline_signal(plane->queued_release_timeline, plane->queued_release_point);
        drm_syncobj_timeline_unref(plane->queued_release_timeline);
        plane->queued_release_timeline = NULL;
    }
    drm_fb_clear_fb(&plane->current_fb);
    if (plane->current_release_timeline != NULL) {
        drm_syncobj_timeline_signal(plane->current_release_timeline, plane->current_release_point);
        drm_syncobj_timeline_unref(plane->current_release_timeline);
        plane->current_release_timeline = NULL;
    }

    drm_surface_finish(&plane->multi_surf);
}

/* drm connector */
static void deallocate_crtc(struct drm_connector *conn);

static struct drm_crtc *get_drm_connector_current_crtc(struct drm_connector *conn,
                                                       const drmModeConnector *drm_conn)
{
    uint32_t crtc_id = 0;
    struct drm_device *drm = conn->drm;
    uint32_t crtc_prop = conn->props[DRM_CONNECTOR_PROP_CRTC_ID].id;
    if (crtc_prop) {
        uint64_t value;
        if (!drm_get_property_value(conn->drm->fd, DRM_MODE_OBJECT_CONNECTOR, conn->id,
                                    &conn->props[DRM_CONNECTOR_PROP_CRTC_ID], &value)) {
            kywc_log(KYWC_ERROR, "Failed to get CRTC_ID connector property");
            return NULL;
        }
        crtc_id = (uint32_t)value;
    } else if (drm_conn->encoder_id != 0) {
        // Fallback to the legacy API
        drmModeEncoder *enc = drmModeGetEncoder(drm->fd, drm_conn->encoder_id);
        if (enc == NULL) {
            kywc_log_errno(KYWC_ERROR, "Failed with drmModeGetEncoder");
            return NULL;
        }
        crtc_id = enc->crtc_id;
        drmModeFreeEncoder(enc);
    }

    if (!crtc_id) {
        return NULL;
    }

    for (size_t i = 0; i < drm->num_crtcs; ++i) {
        if (drm->crtcs[i].id == crtc_id) {
            return &drm->crtcs[i];
        }
    }

    kywc_log(KYWC_ERROR, "Failed to find current CRTC ID %" PRIu32, crtc_id);
    return NULL;
}

static drmModeModeInfo *get_connector_current_mode(struct drm_connector *conn)
{
    if (!conn->crtc) {
        return NULL;
    }

    struct drm_device *drm = conn->drm;
    uint32_t mode_id = conn->crtc->props[DRM_CRTC_PROP_MODE_ID].id;
    if (mode_id) {
        size_t size = 0;
        drmModeModeInfo *mode =
            drm_get_property_blob(drm->fd, DRM_MODE_OBJECT_CRTC, conn->crtc->id,
                                  &conn->crtc->props[DRM_CRTC_PROP_MODE_ID], &size);
        return mode;
    } else {
        // Fallback to the legacy API
        drmModeCrtc *drm_crtc = drmModeGetCrtc(drm->fd, conn->crtc->id);
        if (drm_crtc == NULL) {
            kywc_log_errno(KYWC_ERROR, "Failed with drmModeGetCrtc");
            return NULL;
        }

        if (!drm_crtc->mode_valid) {
            drmModeFreeCrtc(drm_crtc);
            return NULL;
        }

        drmModeModeInfo *mode = malloc(sizeof(*mode));
        if (mode == NULL) {
            kywc_log_errno(KYWC_ERROR, "Allocation drmModeModeInfo failed");
            drmModeFreeCrtc(drm_crtc);
            return NULL;
        }

        *mode = drm_crtc->mode;
        drmModeFreeCrtc(drm_crtc);
        return mode;
    }
}

static struct drm_mode *drm_mode_create(const drmModeModeInfo *modeinfo)
{
    struct drm_mode *mode = calloc(1, sizeof(*mode));
    if (!mode) {
        return NULL;
    }

    mode->drm_mode = *modeinfo;
    mode->wlr_mode.width = mode->drm_mode.hdisplay;
    mode->wlr_mode.height = mode->drm_mode.vdisplay;
    mode->wlr_mode.refresh = drm_util_calculate_refresh_rate(modeinfo);
    mode->wlr_mode.picture_aspect_ratio = drm_util_get_picture_aspect_ratio(modeinfo);
    mode->wlr_mode.preferred = modeinfo->type & DRM_MODE_TYPE_PREFERRED;

    return mode;
}

struct drm_connector *drm_connector_from_output(struct wlr_output *wlr_output)
{
    struct drm_connector *conn = wl_container_of(wlr_output, conn, output);
    return conn;
}

static bool buffer_check_plane_formats(struct wlr_buffer *buf,
                                       const struct wlr_drm_format_set *formats)
{
    struct wlr_dmabuf_attributes attribs;
    if (!wlr_buffer_get_dmabuf(buf, &attribs)) {
        return false;
    }

    if (formats && !wlr_drm_format_set_has(formats, attribs.format, attribs.modifier)) {
        const struct pixel_format *info = pixel_format_from_drm(attribs.format);
        if (!info || info->drm_format != DRM_FORMAT_INVALID ||
            wlr_drm_format_set_has(formats, info->drm_format, attribs.modifier)) {
            // Buffer format can't be scanned out
            return false;
        }
    }

    return true;
}

static bool buffer_is_output_swapchain_buffer(struct drm_connector *conn, struct wlr_buffer *buffer)
{
    struct wlr_swapchain *swapchain = conn->output.swapchain;
    if (!swapchain) {
        return false;
    }

    for (size_t i = 0; i < WLR_SWAPCHAIN_CAP; ++i) {
        if (swapchain->slots[i].buffer == buffer) {
            return true;
        }
    }

    return false;
}

static bool drm_connector_update_cursor(struct drm_connector *conn, struct wlr_buffer *buffer)
{
    if (!buffer) {
        return false;
    }

    struct wlr_buffer *local_buf = NULL;
    struct drm_device *drm = conn->drm;
    struct output *output = output_from_wlr_output(&conn->output);
    struct kywc_output *kywc_output = &output->base;

    if (conn->cursor_buffer != buffer) {
        wlr_buffer_unlock(conn->cursor_buffer);
        conn->cursor_buffer = wlr_buffer_lock(buffer);
    }

    bool default_brightness = kywc_output->state.brightness == 100;
    bool default_color_temp =
        (kywc_output->state.color_temp == 0 || kywc_output->state.color_temp == 6500);
    bool default_color_filter = !kywc_output->state.color_filter;
    bool default_color = kywc_output->state.color_feature == KYWC_OUTPUT_COLOR_FEATURE_DISABLE ||
                         (default_color_filter && default_color_temp && default_brightness);
    bool main_gpu = !drm_device_get_parent(drm);
    bool allocator_uses_shm = conn->output.allocator->buffer_caps & WLR_BUFFER_CAP_SHM;
    bool buffer_matches_plane = buffer_check_plane_formats(buffer, &conn->crtc->cursor->formats);
    bool need_cursor_blit =
        !main_gpu || !default_color || allocator_uses_shm || !buffer_matches_plane;

    if (need_cursor_blit) {
        uint32_t format = DRM_FORMAT_ARGB8888;
        struct drm_surface *surface = &conn->crtc->cursor->multi_surf;
        struct drm_surface_blit_config blit_config = {
            .mgpu_renderer = main_gpu ? NULL : &drm->mgpu_renderer,
            .test_state = NULL,
            .width = buffer->width,
            .height = buffer->height,
            .format = format,
            .use_yuyv = false,
            .buffer_matches_plane = buffer_matches_plane,
        };
        if (!drm_surface_init(conn->crtc->cursor, conn, &blit_config)) {
            return false;
        }

        struct drm_render_target target = {
            .source = buffer,
            .brightness = 50 + MIN(kywc_output->state.brightness, 100) / 2,
            .color_temp = default_color_temp ? 6500 : kywc_output->state.color_temp,
            .color_mat = default_color_filter
                             ? drm_output_get_color_filter_matrix(KYWC_OUTPUT_COLOR_FILTER_NONE)
                             : drm_output_get_color_filter_matrix(kywc_output->state.color_filter),
            .damage = NULL,
            .output = output,
            .rgb_clear = true,
            .use_yuyv = false,
        };

        local_buf = drm_surface_blit(surface, &target);
        if (!local_buf) {
            return false;
        }
    } else {
        local_buf = wlr_buffer_lock(buffer);
    }

    bool ok = drm_fb_import_fb(&conn->cursor_pending_fb, conn->drm, local_buf,
                               &conn->crtc->cursor->formats, false);
    wlr_buffer_unlock(local_buf);
    if (!ok) {
        return false;
    }

    return true;
}

static bool drm_connector_set_cursor(struct wlr_output *output, struct wlr_buffer *buffer,
                                     int hotspot_x, int hotspot_y)
{
    struct drm_connector *conn = drm_connector_from_output(output);
    if (!conn->crtc) {
        return false;
    }

    if (!conn->crtc->cursor) {
        return false;
    }

    if (conn->cursor_hotspot_x != hotspot_x || conn->cursor_hotspot_y != hotspot_y) {
        conn->cursor_x -= hotspot_x - conn->cursor_hotspot_x;
        conn->cursor_y -= hotspot_y - conn->cursor_hotspot_y;
        conn->cursor_hotspot_x = hotspot_x;
        conn->cursor_hotspot_y = hotspot_y;
    }

    conn->cursor_enabled = false;
    drm_fb_clear_fb(&conn->cursor_pending_fb);
    if (!buffer) {
        wlr_buffer_unlock(conn->cursor_buffer);
        conn->cursor_buffer = NULL;
        wlr_output_update_needs_frame(output);
        return true;
    }

    struct drm_device *drm = conn->drm;
    if ((uint64_t)buffer->width != drm->cursor_width ||
        (uint64_t)buffer->height != drm->cursor_height) {
        kywc_log(KYWC_DEBUG, "Cursor buffer size mismatch");
        return false;
    }

    if (!drm_connector_update_cursor(conn, buffer)) {
        return false;
    }

    conn->cursor_enabled = true;
    conn->cursor_width = buffer->width;
    conn->cursor_height = buffer->height;

    wlr_output_update_needs_frame(output);

    return true;
}

static bool drm_connector_move_cursor(struct wlr_output *output, int x, int y)
{
    struct drm_connector *conn = drm_connector_from_output(output);
    if (!conn->crtc) {
        return false;
    }

    if (!conn->crtc->cursor) {
        return false;
    }

    int width, height;
    wlr_output_transformed_resolution(output, &width, &height);

    struct wlr_box box = { .x = x, .y = y };
    enum wl_output_transform transform = wlr_output_transform_invert(output->transform);
    wlr_box_transform(&box, &box, transform, width, height);

    box.x -= conn->cursor_hotspot_x;
    box.y -= conn->cursor_hotspot_y;

    conn->cursor_x = box.x;
    conn->cursor_y = box.y;

    wlr_output_update_needs_frame(output);

    return true;
}

static void drm_connector_set_pending_page_flip(struct drm_connector *conn,
                                                struct drm_page_flip *page_flip)
{
    if (conn->pending_page_flip) {
        conn->pending_page_flip->connector = NULL;
    }

    conn->pending_page_flip = page_flip;
}

static void drm_connector_destroy_output(struct wlr_output *output)
{
    struct drm_connector *conn = drm_connector_from_output(output);

    deallocate_crtc(conn);

    conn->status = DRM_MODE_DISCONNECTED;
    drm_connector_set_pending_page_flip(conn, NULL);

    struct drm_mode *mode, *mode_tmp;
    wl_list_for_each_safe(mode, mode_tmp, &conn->output.modes, wlr_mode.link) {
        wl_list_remove(&mode->wlr_mode.link);
        free(mode);
    }

    free(conn->edid);
    conn->edid = NULL;
    conn->output = (struct wlr_output){ 0 };

    wlr_buffer_unlock(conn->cursor_buffer);
    conn->state.committed = 0;
    pixman_region32_fini(&conn->state.damage);
}

static bool output_pending_enable(struct wlr_output *output, const struct wlr_output_state *state)
{
    if (state->committed & WLR_OUTPUT_STATE_ENABLED) {
        return state->enabled;
    }

    return output->enabled;
}

static void reallocate_crtcs(struct drm_connector *want_conn)
{
    struct drm_device *drm = want_conn->drm;
    assert(drm->num_crtcs > 0);

    size_t num_connectors = wl_list_length(&drm->connectors);
    if (num_connectors == 0) {
        return;
    }

    kywc_log(KYWC_DEBUG, "Reallocating CRTCs");

    struct drm_connector *connectors[num_connectors];
    uint32_t connector_constraints[num_connectors];
    uint32_t previous_match[drm->num_crtcs];
    uint32_t new_match[drm->num_crtcs];

    for (size_t i = 0; i < drm->num_crtcs; ++i) {
        previous_match[i] = UNMATCHED;
    }

    kywc_log(KYWC_DEBUG, "State before reallocation:");
    size_t i = 0;
    struct drm_connector *conn;
    wl_list_for_each(conn, &drm->connectors, link) {
        connectors[i] = conn;
        int index = conn->crtc - drm->crtcs;
        if (conn->crtc) {
            previous_match[index] = i;
        }

        // Only request a CRTC if the connected is currently enabled or it's the
        // connector the user wants to enable
        bool want_crtc = conn == want_conn || conn->output.enabled;

        kywc_log(KYWC_DEBUG, "  '%s': crtc=%d status=%s want_crtc=%d", conn->name,
                 conn->crtc ? index : -1, drm_util_get_connector_status_str(conn->status),
                 want_crtc);

        if (conn->status == DRM_MODE_CONNECTED && want_crtc) {
            connector_constraints[i] = conn->possible_crtcs;
        } else {
            // Will always fail to match anything
            connector_constraints[i] = 0;
        }

        ++i;
    }

    drm_util_match_obj(num_connectors, connector_constraints, drm->num_crtcs, previous_match,
                       new_match);

    // Converts our crtc=>connector result into a connector=>crtc one.
    ssize_t connector_match[num_connectors];
    for (size_t i = 0; i < num_connectors; ++i) {
        connector_match[i] = -1;
    }
    for (size_t i = 0; i < drm->num_crtcs; ++i) {
        if (new_match[i] != UNMATCHED) {
            connector_match[new_match[i]] = i;
        }
    }

    // Refuse to remove a CRTC from an enabled connector, and refuse to
    // change the CRTC of an enabled connector.
    for (size_t i = 0; i < num_connectors; ++i) {
        struct drm_connector *conn = connectors[i];
        if (conn->status != DRM_MODE_CONNECTED || !conn->output.enabled) {
            continue;
        }
        if (connector_match[i] == -1) {
            kywc_log(KYWC_DEBUG, "Could not match a CRTC for previously connected output; "
                                 "keeping old configuration");
            return;
        }
        assert(conn->crtc != NULL);
        if (connector_match[i] != conn->crtc - drm->crtcs) {
            kywc_log(KYWC_DEBUG, "Cannot switch CRTC for enabled output; "
                                 "keeping old configuration");
            return;
        }
    }

    // Apply new configuration
    kywc_log(KYWC_DEBUG, "State after reallocation:");
    for (size_t i = 0; i < num_connectors; ++i) {
        struct drm_connector *conn = connectors[i];

        kywc_log(KYWC_DEBUG, "  '%s': crtc=%zd", conn->name, connector_match[i]);

        if (conn->crtc != NULL && connector_match[i] == conn->crtc - drm->crtcs) {
            // We don't need to change anything
            continue;
        }

        deallocate_crtc(conn);
        if (connector_match[i] >= 0) {
            conn->crtc = &drm->crtcs[connector_match[i]];
        }
    }
}

static bool drm_connector_allocate_crtc(struct drm_connector *conn)
{
    if (!conn->crtc) {
        reallocate_crtcs(conn);
    }

    bool ok = conn->crtc != NULL;
    if (!ok) {
        kywc_log(KYWC_WARN, "Failed to find free CRTC");
    }
    return ok;
}

void drm_output_pending_resolution(struct wlr_output *output, const struct wlr_output_state *state,
                                   int *width, int *height)
{
    if (state->committed & WLR_OUTPUT_STATE_MODE) {
        switch (state->mode_type) {
        case WLR_OUTPUT_STATE_MODE_FIXED:
            *width = state->mode->width;
            *height = state->mode->height;
            return;
        case WLR_OUTPUT_STATE_MODE_CUSTOM:
            *width = state->custom_mode.width;
            *height = state->custom_mode.height;
            return;
        }
        abort();
    } else {
        *width = output->width;
        *height = output->height;
    }
}

static void drm_connector_state_init(struct drm_connector_state *state, struct drm_connector *conn,
                                     const struct wlr_output_state *base)
{
    *state = (struct drm_connector_state){
        .connector = conn,
        .base = base,
        .active = output_pending_enable(&conn->output, base),
        .output_state = &conn->state,
        .primary_in_fence_fd = -1,
    };

    struct wlr_output_mode *mode = conn->output.current_mode;
    int32_t width = conn->output.width;
    int32_t height = conn->output.height;
    int32_t refresh = conn->output.refresh;

    if (base->committed & WLR_OUTPUT_STATE_MODE) {
        switch (base->mode_type) {
        case WLR_OUTPUT_STATE_MODE_FIXED:;
            mode = base->mode;
            break;
        case WLR_OUTPUT_STATE_MODE_CUSTOM:
            mode = NULL;
            width = base->custom_mode.width;
            height = base->custom_mode.height;
            refresh = base->custom_mode.refresh;
            break;
        }
    }

    if (mode) {
        struct drm_mode *drm_mode = wl_container_of(mode, drm_mode, wlr_mode);
        state->mode = drm_mode->drm_mode;
    } else {
        drm_util_generate_cvt_mode(&state->mode, width, height, (float)refresh / 1000);
    }
    if (output_pending_enable(&conn->output, base)) {
        // The crtc must be set up before this function is called
        assert(conn->crtc != NULL);

        struct drm_plane *primary = conn->crtc->primary;
        if (primary->queued_fb != NULL) {
            state->primary_fb = drm_fb_lock_fb(primary->queued_fb);
        } else if (primary->current_fb != NULL) {
            state->primary_fb = drm_fb_lock_fb(primary->current_fb);
        }

        if (conn->cursor_enabled) {
            struct drm_plane *cursor = conn->crtc->cursor;
            assert(cursor != NULL);
            if (conn->cursor_pending_fb) {
                state->cursor_fb = drm_fb_lock_fb(conn->cursor_pending_fb);
            } else if (cursor->queued_fb) {
                state->cursor_fb = drm_fb_lock_fb(cursor->queued_fb);
            } else if (cursor->current_fb) {
                state->cursor_fb = drm_fb_lock_fb(cursor->current_fb);
            }
        }
    }
}

static bool drm_connector_state_update_primary_fb(struct drm_connector *conn,
                                                  struct drm_connector_state *state, bool test_only)
{
    assert(state->base->committed & WLR_OUTPUT_STATE_BUFFER);

    struct drm_crtc *crtc = conn->crtc;
    assert(crtc != NULL);

    struct wlr_buffer *local_buf = NULL;
    struct wlr_buffer *source = state->base->buffer;
    struct drm_device *drm = conn->drm;
    struct output *output = output_from_wlr_output(&conn->output);

    struct drm_syncobj_timeline *wait_timeline = NULL;
    uint64_t wait_point = 0;
    if (state->output_state->committed & DRM_OUTPUT_STATE_WAIT_TIMELINE) {
        wait_timeline = state->output_state->wait_timeline;
        wait_point = state->output_state->wait_point;
    }
    assert(state->wait_timeline == NULL);

    bool hw_brightness = drm_output_use_hw_brightness(&conn->output, &output->base.state);
    bool hw_color_temp = drm_output_use_hw_color_temp(&conn->output, &output->base.state);
    bool hw_color_filter = drm_output_use_hw_color_filter(&conn->output, &output->base.state);
    bool hw_support = hw_brightness && hw_color_temp && hw_color_filter;
    bool main_gpu = !drm_device_get_parent(drm);
    bool allocator_uses_shm = conn->output.allocator->buffer_caps & WLR_BUFFER_CAP_SHM;
    bool use_yuyv = drm_output_yuyv_enabled(&conn->output, &output->base.state) &&
                    !allocator_uses_shm && buffer_is_output_swapchain_buffer(conn, source);
    bool need_primary_blit =
        !test_only && (!main_gpu || !hw_support || use_yuyv || allocator_uses_shm);

    if (need_primary_blit) {
        uint32_t format = main_gpu && !use_yuyv ? conn->output.render_format : DRM_FORMAT_ARGB8888;
        if (state->base->committed & WLR_OUTPUT_STATE_RENDER_FORMAT) {
            format = state->base->render_format;
        }

        int width = 0, height = 0;
        struct drm_plane *primary = crtc->primary;
        struct drm_surface *surface = &primary->multi_surf;
        drm_output_pending_resolution(&conn->output, state->base, &width, &height);
        struct drm_surface_blit_config blit_config = {
            .mgpu_renderer = main_gpu ? NULL : &drm->mgpu_renderer,
            .test_state = state->base,
            .width = use_yuyv ? width / 2 : width,
            .height = height,
            .format = format,
            .use_yuyv = use_yuyv,
            .buffer_matches_plane = true,
        };
        if (!drm_surface_init(primary, conn, &blit_config)) {
            return false;
        }

        use_yuyv = use_yuyv && !wlr_renderer_is_pixman_renderer(surface->wlr_rend);

        struct kywc_output *kywc_output = &output->base;
        struct drm_render_target target = {
            .source = source,
            .brightness = hw_brightness ? 100 : kywc_output->state.brightness,
            .color_temp = hw_color_temp ? 6500 : kywc_output->state.color_temp,
            .color_mat = hw_color_filter
                             ? drm_output_get_color_filter_matrix(KYWC_OUTPUT_COLOR_FILTER_NONE)
                             : drm_output_get_color_filter_matrix(kywc_output->state.color_filter),
            .damage = conn->state.committed & DRM_OUTPUT_STATE_DAMAGE ? &conn->state.damage : NULL,
            .output = output,
            .rgb_clear = !use_yuyv ? !hw_support : false,
            .use_yuyv = use_yuyv,
            .wait_timeline = wait_timeline,
            .wait_point = wait_point,
        };

        local_buf = drm_surface_blit(surface, &target);
        if (!local_buf) {
            return false;
        }

        if (local_buf != target.source && primary->multi_surf.timeline != NULL) {
            state->wait_timeline = drm_syncobj_timeline_ref(primary->multi_surf.timeline);
            state->wait_point = primary->multi_surf.point;
        } else if (local_buf == target.source && wait_timeline) {
            state->wait_timeline = drm_syncobj_timeline_ref(wait_timeline);
            state->wait_point = wait_point;
        }
    } else {
        local_buf = wlr_buffer_lock(source);

        if (wait_timeline) {
            state->wait_timeline = drm_syncobj_timeline_ref(wait_timeline);
            state->wait_point = wait_point;
        }
    }

    drm_output_set_state(output);

    use_yuyv = use_yuyv && !test_only;

    crtc->primary->enable_yuyv = use_yuyv;
    bool ok = drm_fb_import_fb(&state->primary_fb, conn->drm, local_buf, &crtc->primary->formats,
                               use_yuyv);
    wlr_buffer_unlock(local_buf);
    if (!ok) {
        kywc_log(KYWC_DEBUG, "Failed to import buffer for scan-out");
        return false;
    }

    return true;
}

static bool drm_connector_prepare(struct drm_connector_state *conn_state, bool test_only)
{
    const struct wlr_output_state *state = conn_state->base;
    struct drm_connector *conn = conn_state->connector;
    struct wlr_output *output = &conn->output;

    uint32_t unsupported = state->committed & ~SUPPORTED_OUTPUT_STATE;
    if (unsupported) {
        kywc_log(KYWC_DEBUG, "Unsupported output state fields = 0x%" PRIx32, unsupported);
        return false;
    }

    if (state->committed & WLR_OUTPUT_STATE_ENABLED && state->enabled) {
        if (output->current_mode == NULL && !(state->committed & WLR_OUTPUT_STATE_MODE)) {
            kywc_log(KYWC_DEBUG, "Can't enable an output without a mode");
            return false;
        }
    }

    if ((state->committed & WLR_OUTPUT_STATE_ADAPTIVE_SYNC_ENABLED) &&
        state->adaptive_sync_enabled && !conn->adaptive_sync_supported) {
        return false;
    }

    if ((state->committed & WLR_OUTPUT_STATE_BUFFER) &&
        (drm_device_get_parent(conn->drm) &&
         !(conn->output.allocator->buffer_caps & WLR_BUFFER_CAP_SHM))) {
        struct wlr_dmabuf_attributes dmabuf;
        if (!wlr_buffer_get_dmabuf(state->buffer, &dmabuf)) {
            kywc_log(KYWC_DEBUG, "Buffer is not a DMA-BUF");
            return false;
        }

        if (!wlr_drm_format_set_has(&conn->drm->mgpu_renderer.formats, dmabuf.format,
                                    dmabuf.modifier)) {
            kywc_log(KYWC_DEBUG,
                     "Buffer format 0x%" PRIX32 " with modifier 0x%" PRIX64 " cannot be "
                     "imported into multi-GPU renderer",
                     dmabuf.format, dmabuf.modifier);
        }
    }

    if (test_only && (drm_device_get_parent(conn->drm) ||
                      (conn->output.allocator->buffer_caps & WLR_BUFFER_CAP_SHM))) {
        // if we are running as a secondary GPU, we can't peform an atomic
        // commit without blitting a buffer
        return true;
    }

    if (state->committed & WLR_OUTPUT_STATE_BUFFER) {
        if (!drm_connector_state_update_primary_fb(conn, conn_state, test_only)) {
            return false;
        }
    }

    if (conn_state->base->tearing_page_flip && !conn->drm->supports_tearing_page_flips) {
        kywc_log(KYWC_ERROR, "Attempted to submit a tearing page flip to an unsupportd drm");
        return false;
    }

    if (conn_state->active && !conn_state->primary_fb) {
        kywc_log(KYWC_ERROR, "No Primary frame buffer available for this connector");
    }
    return true;
}

static struct drm_page_flip *drm_create_page_flip(struct drm_device *drm,
                                                  const struct drm_device_state *state)
{
    struct drm_page_flip *page_flip = calloc(1, sizeof(*page_flip));
    if (!page_flip) {
        return NULL;
    }

    page_flip->connector = state->conn_state->connector;
    wl_list_insert(&drm->page_flips, &page_flip->link);
    return page_flip;
}

static void drm_connector_state_finish(struct drm_connector_state *state, bool test_only)
{
    drm_fb_clear_fb(&state->primary_fb);
    drm_fb_clear_fb(&state->cursor_fb);
    drm_syncobj_timeline_unref(state->wait_timeline);
    if (!test_only) {
        drm_output_state_clear(&state->connector->state);
    }
}

static void drm_connector_apply_commit(const struct drm_connector_state *state,
                                       struct drm_page_flip *page_flip)
{
    struct drm_connector *conn = state->connector;
    struct drm_crtc *crtc = conn->crtc;

    drm_fb_copy_fb(&crtc->primary->queued_fb, state->primary_fb);
    if (crtc->primary->queued_release_timeline != NULL) {
        drm_syncobj_timeline_signal(crtc->primary->queued_release_timeline,
                                    crtc->primary->queued_release_point);
        drm_syncobj_timeline_unref(crtc->primary->queued_release_timeline);
    }
    if (state->output_state->signal_timeline != NULL) {
        crtc->primary->queued_release_timeline =
            drm_syncobj_timeline_ref(state->output_state->signal_timeline);
        crtc->primary->queued_release_point = state->output_state->signal_point;
    } else {
        crtc->primary->queued_release_timeline = NULL;
        crtc->primary->queued_release_point = 0;
    }
    if (crtc->cursor) {
        drm_fb_copy_fb(&crtc->cursor->queued_fb, state->cursor_fb);
    }
    drm_fb_clear_fb(&conn->cursor_pending_fb);

    drm_connector_set_pending_page_flip(conn, page_flip);

    if (state->base->committed & WLR_OUTPUT_STATE_MODE) {
        conn->refresh = drm_util_calculate_refresh_rate(&state->mode);
    }

    if (!state->active) {
        drm_plane_fb_finish(crtc->primary);
        drm_plane_fb_finish(crtc->cursor);
        drm_fb_clear_fb(&conn->cursor_pending_fb);

        conn->cursor_enabled = false;
        conn->crtc = NULL;
    }
}

static bool drm_commit(struct drm_device *drm, const struct drm_device_state *state, uint32_t flags,
                       bool test_only)
{
    // Disallow atomic-only flags
    assert((flags & ~DRM_MODE_PAGE_FLIP_FLAGS) == 0);

    struct drm_page_flip *page_flip = NULL;
    if (flags & DRM_MODE_PAGE_FLIP_EVENT) {
        page_flip = drm_create_page_flip(drm, state);
        if (page_flip == NULL) {
            return false;
        }
        page_flip->async = flags & DRM_MODE_PAGE_FLIP_ASYNC;
    }

    bool ok = drm_kms_commit(drm, state, page_flip, flags, test_only);
    if (ok && !test_only) {
        drm_connector_apply_commit(state->conn_state, page_flip);
    } else {
        drm_destroy_page_flip(page_flip);
        // The set_cursor() hook is a bit special: it's not really synchronized
        // to commit() or test(). Once set_cursor() returns true, the new
        // cursor is effectively committed. So don't roll it back here, or we
        // risk ending up in a state where we don't have a cursor FB but
        // wlr_drm_connector.cursor_enabled is true.
        // TODO: fix our output interface to avoid this issue.
    }
    return ok;
}

static bool drm_commit_connector_state(struct drm_connector *conn,
                                       const struct wlr_output_state *state, bool test_only)
{
    struct drm_device *drm = conn->drm;
    if (!drm->session_active) {
        return false;
    }

    if (test_only && !(state->committed & COMMIT_OUTPUT_STATE)) {
        return true;
    }

    if (output_pending_enable(&conn->output, state) && !drm_connector_allocate_crtc(conn)) {
        kywc_log(KYWC_ERROR, "No CRTC available for this connector");
        return false;
    }

    if (conn->state.color_changed && drm_connector_update_cursor(conn, conn->cursor_buffer)) {
        conn->cursor_enabled = true;
    }

    bool ok = false;
    struct drm_connector_state pending = { 0 };
    drm_connector_state_init(&pending, conn, state);
    struct drm_device_state pending_dev = {
        .modeset = state->allow_reconfiguration,
        // The wlr_output API requires non-modeset commits with a new buffer to
        // wait for the frame event. However compositors often perform
        // non-modesets commits without a new buffer without waiting for the
        // frame event. In that case we need to make the KMS commit blocking,
        // otherwise the kernel will error out with EBUSY.
        .nonblock = !state->allow_reconfiguration && (state->committed & WLR_OUTPUT_STATE_BUFFER),
        .conn_state = &pending,
    };

    if (!drm_connector_prepare(&pending, test_only)) {
        goto out;
    }

    if (test_only && (drm_device_get_parent(conn->drm) ||
                      (conn->output.allocator->buffer_caps & WLR_BUFFER_CAP_SHM))) {
        // if we are running as a secondary GPU, we can not perform an atomic commit without
        // blitting a buffer
        ok = true;
        goto out;
    }

    if (!pending.active && !conn->crtc) {
        // Disabling an already-diabled connector
        ok = true;
        goto out;
    }

    if (!test_only && pending_dev.modeset) {
        if (pending.active) {
            kywc_log(KYWC_INFO, "Modesetting with %dx%d @ %.3f Hz", pending.mode.hdisplay,
                     pending.mode.vdisplay,
                     (float)drm_util_calculate_refresh_rate(&pending.mode) / 1000);
        } else {
            kywc_log(KYWC_INFO, "%s Turning off", conn->name);
        }
    }
    // kms_commit will perform either a non-blocking
    // page-flip, eigher a blocking modeset. when performing a blocking modeset
    // we will wati for all queued page-flips to complete, so we don't need this safeguard
    if (!test_only && pending_dev.nonblock && conn->pending_page_flip) {
        kywc_log_ratelimited(KYWC_ERROR,
                             "Failed to page-flip output: a page-flip is already pending");
        goto out;
    }

    uint32_t flags = 0;
    if (!test_only && pending.active) {
        flags |= DRM_MODE_PAGE_FLIP_EVENT;
    }
    if (pending.base->tearing_page_flip) {
        flags |= DRM_MODE_PAGE_FLIP_ASYNC;
    }

    ok = drm_commit(conn->drm, &pending_dev, flags, test_only);

out:
    drm_connector_state_finish(&pending, test_only);
    return ok;
}

static void deallocate_crtc(struct drm_connector *conn)
{
    if (conn->crtc == NULL) {
        return;
    }

    kywc_log(KYWC_DEBUG, "De-allocating CRTC %" PRIu32, conn->crtc->id);

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, false);
    if (!drm_commit_connector_state(conn, &state, false)) {
        // On GPU unplug, disabling the CRTC can fail with EPERM
        kywc_log(KYWC_ERROR, "Failed to disable [%s]CRTC %" PRIu32, conn->name, conn->crtc->id);

        drm_plane_fb_finish(conn->crtc->primary);
        drm_plane_fb_finish(conn->crtc->cursor);
        drm_fb_clear_fb(&conn->cursor_pending_fb);
        conn->cursor_enabled = false;
        conn->crtc = NULL;
    }
    wlr_output_state_finish(&state);
}

static bool drm_connector_test(struct wlr_output *output, const struct wlr_output_state *state)
{
    struct drm_connector *conn = drm_connector_from_output(output);
    return drm_commit_connector_state(conn, state, true);
}

static bool drm_connector_commit(struct wlr_output *output, const struct wlr_output_state *state)
{
    struct drm_connector *conn = drm_connector_from_output(output);
    return drm_commit_connector_state(conn, state, false);
}

static size_t drm_connector_get_gamma_size(struct wlr_output *output)
{
    struct drm_connector *conn = drm_connector_from_output(output);
    struct drm_crtc *crtc = conn->crtc;

    if (!crtc) {
        return 0;
    }

    return drm_get_crtc_gamma_lut_size(crtc);
}

static const struct wlr_drm_format_set *drm_connector_get_cursor_formats(struct wlr_output *output,
                                                                         uint32_t buffer_caps)
{
    if (!(buffer_caps & (WLR_BUFFER_CAP_DMABUF | WLR_BUFFER_CAP_DATA_PTR))) {
        return NULL;
    }
    struct drm_connector *conn = drm_connector_from_output(output);
    if (!drm_connector_allocate_crtc(conn)) {
        return NULL;
    }
    if (!conn->crtc->cursor) {
        return NULL;
    }

    if (drm_device_get_parent(conn->drm)) {
        return &conn->drm->mgpu_renderer.formats;
    }

    struct wlr_drm_format format = { 0 };
    const struct wlr_drm_format_set *display_formats = &conn->crtc->cursor->formats;
    if (!drm_plane_pick_render_format(output->renderer, display_formats, &format,
                                      DRM_FORMAT_ARGB8888)) {
        // No plane formats could render, put render formats
        return renderer_get_render_formats(output->renderer);
    }
    wlr_drm_format_finish(&format);

    return display_formats;
}

static void drm_connector_get_cursor_size(struct wlr_output *output, int *width, int *height)
{
    struct drm_connector *conn = drm_connector_from_output(output);
    *width = conn->drm->cursor_width;
    *height = conn->drm->cursor_height;
}

static const struct wlr_drm_format_set *drm_connector_get_primary_formats(struct wlr_output *output,
                                                                          uint32_t buffer_caps)
{
    if (!(buffer_caps & (WLR_BUFFER_CAP_DMABUF | WLR_BUFFER_CAP_DATA_PTR))) {
        return NULL;
    }
    struct drm_connector *conn = drm_connector_from_output(output);
    if (!drm_connector_allocate_crtc(conn)) {
        return NULL;
    }
    kywc_log(KYWC_DEBUG, "Output: %s current CRTC_ID: %d get primary_formats", output->name,
             conn->crtc->id);
    if (drm_device_get_parent(conn->drm)) {
        return &conn->drm->mgpu_renderer.formats;
    }

    kywc_log(KYWC_DEBUG, "Primary_plane: %d, formats_len: %ld", conn->crtc->primary->id,
             conn->crtc->primary->formats.len);
    return &conn->crtc->primary->formats;
}

static const struct wlr_output_impl output_impl = {
    .set_cursor = drm_connector_set_cursor,
    .move_cursor = drm_connector_move_cursor,
    .destroy = drm_connector_destroy_output,
    .test = drm_connector_test,
    .commit = drm_connector_commit,
    .get_gamma_size = drm_connector_get_gamma_size,
    .get_cursor_formats = drm_connector_get_cursor_formats,
    .get_cursor_size = drm_connector_get_cursor_size,
    .get_primary_formats = drm_connector_get_primary_formats,
};

bool wlr_output_is_drm(struct wlr_output *wlr_output)
{
    return wlr_output->impl == &output_impl;
}

static bool drm_connector_connect(struct drm_connector *conn, const drmModeConnector *drm_conn)
{
    kywc_log(KYWC_DEBUG, "Current CRTC: %d", conn->crtc ? (int)conn->crtc->id : -1);
    if (!conn->crtc) {
        conn->possible_crtcs = drmModeConnectorGetPossibleCrtcs(conn->drm->fd, drm_conn);
        if (!conn->possible_crtcs) {
            kywc_log(KYWC_WARN, "No CRTC possible");
        }
        conn->crtc = get_drm_connector_current_crtc(conn, drm_conn);
    }

    // keep track of all the modes ourselves first. We must only fill out
    // the modes list after wlr_output_init()
    struct wl_list modes;
    wl_list_init(&modes);

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, false);

    struct drm_device *drm = conn->drm;
    drmModeModeInfo *current_modeinfo = get_connector_current_mode(conn);

    kywc_log(KYWC_INFO, "  Detected modes:");

    for (int i = 0; i < drm_conn->count_modes; ++i) {
        if (drm_conn->modes[i].flags & DRM_MODE_FLAG_INTERLACE) {
            continue;
        }

        struct drm_mode *mode = drm_mode_create(&drm_conn->modes[i]);
        if (!mode) {
            kywc_log_errno(KYWC_ERROR, "Allocation failed");
            wlr_output_state_finish(&state);
            return false;
        }

        // If this is the current mode set on the conn's crtc,
        // then set it as the conn's output current mode.
        if (current_modeinfo != NULL &&
            memcmp(&mode->drm_mode, current_modeinfo, sizeof(*current_modeinfo)) == 0) {
            wlr_output_state_set_mode(&state, &mode->wlr_mode);

            uint64_t mode_id = 0;
            if (!drm_get_property_value(drm->fd, DRM_MODE_OBJECT_CRTC, conn->crtc->id,
                                        &conn->crtc->props[DRM_CRTC_PROP_MODE_ID], &mode_id)) {
                kywc_log(KYWC_DEBUG, "Failed to get MODE ID");
            }

            conn->crtc->mode_id = mode_id;
            conn->refresh = drm_util_calculate_refresh_rate(current_modeinfo);
        }

        kywc_log(KYWC_INFO, "    %" PRId32 "x%" PRId32 " @ %.3f Hz%s", mode->wlr_mode.width,
                 mode->wlr_mode.height, (float)mode->wlr_mode.refresh / 1000,
                 mode->wlr_mode.preferred ? " (preferred)" : "");

        wl_list_insert(modes.prev, &mode->wlr_mode.link);
    }

    free(current_modeinfo);

    conn->state = (struct drm_output_state){ 0 };
    pixman_region32_init(&conn->state.damage);

    struct wlr_output *output = &conn->output;
    wlr_output_init(output, drm->wlr_backend, &output_impl, drm->display, &state);
    wlr_output_state_finish(&state);
    // fill out the modes
    wl_list_insert_list(&output->modes, &modes);

    wlr_output_set_name(output, conn->name);

    free(conn->edid);
    conn->edid_len = 0;

    conn->edid = drm_get_property_blob(drm->fd, DRM_MODE_OBJECT_CONNECTOR, conn->id,
                                       &conn->props[DRM_CONNECTOR_PROP_EDID], &conn->edid_len);
    if (conn->edid_len > 0) {
        drm_util_parse_edid(conn, conn->edid, conn->edid_len);
    } else {
        kywc_log(KYWC_WARN, "Connector has no EDID");
    }

    output->phys_width = drm_conn->mmWidth;
    output->phys_height = drm_conn->mmHeight;

    kywc_log(KYWC_INFO, "  Make: %s, Model: %s, Serial: %s, Physical size: %d x %d mm",
             output->make ? output->make : "(null)", output->model ? output->model : "(null)",
             output->serial ? output->serial : "(null)", output->phys_width, output->phys_height);

    if (drm_conn->subpixel < sizeof(subpixel_map) / sizeof(subpixel_map[0])) {
        output->subpixel = subpixel_map[drm_conn->subpixel];
    } else {
        kywc_log(KYWC_ERROR, "Unknown subpixel value: %d", (int)drm_conn->subpixel);
    }

    uint64_t non_desktop;
    if (drm_get_property_value(drm->fd, DRM_MODE_OBJECT_CONNECTOR, conn->id,
                               &conn->props[DRM_CONNECTOR_PROP_NON_DESKTOP], &non_desktop)) {
        if (non_desktop == 1) {
            kywc_log(KYWC_INFO, "Non-desktop connector");
        }
        output->non_desktop = non_desktop;
    }

    memset(conn->max_bpc_bounds, 0, sizeof(conn->max_bpc_bounds));
    uint32_t maxbpc = conn->props[DRM_CONNECTOR_PROP_MAX_BPC].id;
    if (maxbpc != 0) {
        if (!drm_get_property_range(drm->fd, maxbpc, &conn->max_bpc_bounds[0],
                                    &conn->max_bpc_bounds[1])) {
            kywc_log(KYWC_ERROR, "Failed to introspect 'max bpc' property");
        }
    }

    uint64_t vrr_capable = 0;
    struct drm_prop_info *vrr_capable_prop = &conn->props[DRM_CONNECTOR_PROP_VRR_CAPABLE];
    if (vrr_capable_prop->id) {
        drm_get_property_value(conn->drm->fd, DRM_MODE_OBJECT_CONNECTOR, conn->id, vrr_capable_prop,
                               &vrr_capable);
    }
    conn->adaptive_sync_supported = vrr_capable;

    struct drm_prop_info *rgb_range_prop = &conn->props[DRM_CONNECTOR_PROP_RGB_RANGE];
    if (rgb_range_prop->id) {
        uint64_t rgb_range = 0;
        if (drm_get_property_value(conn->drm->fd, DRM_MODE_OBJECT_CONNECTOR, conn->id,
                                   rgb_range_prop, &rgb_range)) {
            kywc_log(KYWC_DEBUG, "Connector support rgb range");
        }
        conn->rgb_range = rgb_range;
    }

    struct drm_prop_info *overscan_prop = &conn->props[DRM_CONNECTOR_PROP_OVERSCAN];
    if (overscan_prop->id) {
        uint64_t overscan = 0;
        if (drm_get_property_value(conn->drm->fd, DRM_MODE_OBJECT_CONNECTOR, conn->id,
                                   overscan_prop, &overscan)) {
            kywc_log(KYWC_DEBUG, "Connector support overscan");
        }
        conn->overscan = overscan;
    }

    struct drm_prop_info *scaling_mode_prop = &conn->props[DRM_CONNECTOR_PROP_SCALING_MODE];
    if (scaling_mode_prop->id) {
        uint64_t scaling_mode = 0;
        if (drm_get_property_value(conn->drm->fd, DRM_MODE_OBJECT_CONNECTOR, conn->id,
                                   scaling_mode_prop, &scaling_mode)) {
            kywc_log(KYWC_DEBUG, "Connector support scaling mode, current mode: %ld", scaling_mode);
        }
        conn->scaling_mode = scaling_mode;

        char *enum_list = drm_get_property_enum_list(drm->fd, DRM_MODE_OBJECT_CONNECTOR, conn->id,
                                                     scaling_mode_prop);
        if (enum_list) {
            if (strncmp(enum_list, "None", 4) == 0) {
                conn->has_none_mode = true;
            }
            kywc_log(KYWC_INFO, "  Scaling mode: %s", enum_list);
            free(enum_list);
        }
    }

    char *subconnector = NULL;
    uint32_t sub = conn->props[DRM_CONNECTOR_PROP_SUBCONNECTOR].id;
    if (sub) {
        subconnector = drm_get_property_enum(drm->fd, DRM_MODE_OBJECT_CONNECTOR, conn->id,
                                             &conn->props[DRM_CONNECTOR_PROP_SUBCONNECTOR]);
    }
    if (subconnector && strcmp(subconnector, "Native") == 0) {
        free(subconnector);
        subconnector = NULL;
    }

    char description[128];
    snprintf(description, sizeof(description), "%s %s%s%s (%s%s%s)", output->make, output->model,
             output->serial ? " " : "", output->serial ? output->serial : "", output->name,
             subconnector ? " via " : "", subconnector ? subconnector : "");
    wlr_output_set_description(output, description);

    free(subconnector);
    conn->status = DRM_MODE_CONNECTED;
    return true;
}

size_t drm_get_crtc_gamma_lut_size(struct drm_crtc *crtc)
{
    struct drm_device *drm = crtc->drm;
    uint32_t gamma_lut_size_prop = crtc->props[DRM_CRTC_PROP_GAMMA_LUT_SIZE].id;
    if (gamma_lut_size_prop == 0 || drm->mode == DRM_KMS_MODE_LEGACY) {
        return (size_t)crtc->legacy_gamma_size;
    }

    uint64_t gamma_lut_size;
    if (!drm_get_property_value(drm->fd, DRM_MODE_OBJECT_CRTC, crtc->id,
                                &crtc->props[DRM_CRTC_PROP_GAMMA_LUT_SIZE], &gamma_lut_size)) {
        kywc_log(KYWC_ERROR, "Unable to get gamma lut size");
        return 0;
    }

    return gamma_lut_size;
}

static struct drm_connector *drm_connector_create(struct drm_device *drm,
                                                  drmModeConnector *drm_conn)
{
    struct drm_connector *conn = calloc(1, sizeof(*conn));
    if (!conn) {
        kywc_log_errno(KYWC_ERROR, "Allocation drm_connector failed");
        return NULL;
    }

    conn->id = drm_conn->connector_id;
    conn->drm = drm;
    conn->status = DRM_MODE_DISCONNECTED;

    const char *conn_name = drmModeGetConnectorTypeName(drm_conn->connector_type);
    if (conn_name == NULL) {
        conn_name = "Unknown";
    }

    snprintf(conn->name, sizeof(conn->name), "%s-%" PRIu32, conn_name, drm_conn->connector_type_id);
    kywc_log(KYWC_INFO, "Find connector: %s", conn->name);

    if (!drm_get_connector_props(drm->fd, conn->id, conn->props)) {
        free(conn);
        return NULL;
    }

    conn->possible_crtcs = drmModeConnectorGetPossibleCrtcs(drm->fd, drm_conn);
    if (!conn->possible_crtcs) {
        kywc_log(KYWC_WARN, "No CRTC possible");
    }
    conn->crtc = get_drm_connector_current_crtc(conn, drm_conn);

    wl_list_insert(drm->connectors.prev, &conn->link);
    return conn;
}

static void handle_page_flip(int fd, unsigned seq, unsigned tv_sec, unsigned tv_usec,
                             unsigned crtc_id, void *data)
{
    struct drm_page_flip *page_flip = data;
    struct drm_connector *conn = page_flip->connector;
    if (conn) {
        conn->pending_page_flip = NULL;
    }
    drm_destroy_page_flip(page_flip);

    if (!conn) {
        return;
    }

    if (conn->status != DRM_MODE_CONNECTED || conn->crtc == NULL) {
        kywc_log(KYWC_WARN, "Ignoring page-flip event for disabled connector");
        return;
    }

    struct drm_plane *plane = conn->crtc->primary;
    if (plane->queued_fb) {
        drm_fb_move_fb(&plane->current_fb, &plane->queued_fb);
        if (plane->current_release_timeline != NULL) {
            drm_syncobj_timeline_signal(plane->current_release_timeline,
                                        plane->current_release_point);
            drm_syncobj_timeline_unref(plane->current_release_timeline);
        }
        plane->current_release_timeline = plane->queued_release_timeline;
        plane->current_release_point = plane->queued_release_point;
        plane->queued_release_timeline = NULL;
        plane->queued_release_point = 0;
    }

    if (conn->crtc->cursor && conn->crtc->cursor->queued_fb) {
        drm_fb_move_fb(&conn->crtc->cursor->current_fb, &conn->crtc->cursor->queued_fb);
    }

    uint32_t present_flags =
        WLR_OUTPUT_PRESENT_VSYNC | WLR_OUTPUT_PRESENT_HW_CLOCK | WLR_OUTPUT_PRESENT_HW_COMPLETION;
    /* Don't report ZERO_COPY in multi-gpu situations, because we had to copy
     * data between the GPUs, even if we were using the direct scanout
     * interface.
     */
    struct drm_device *drm = conn->drm;
    if (!drm_device_get_parent(drm)) {
        present_flags |= WLR_OUTPUT_PRESENT_ZERO_COPY;
    }

    struct timespec present_time = {
        .tv_sec = tv_sec,
        .tv_nsec = tv_usec * 1000,
    };
    struct wlr_output_event_present present_event = {
        /* The DRM backend guarantees that the presentation event will be for
         * the last submitted frame. */
        .commit_seq = conn->output.commit_seq,
        .presented = drm->session_active,
        .when = &present_time,
        .seq = seq,
        .refresh = mhz_to_nsec(conn->refresh),
        .flags = present_flags,
    };
    wlr_output_send_present(&conn->output, &present_event);

    if (drm->session_active) {
        wlr_output_send_frame(&conn->output);
    }

    return;
}

static int drm_handle_event(int fd, uint32_t mask, void *data)
{
    struct drm_device *drm = data;

    drmEventContext event = {
        .version = 3,
        .page_flip_handler2 = handle_page_flip,
    };

    if (drmHandleEvent(fd, &event) != 0) {
        kywc_log(KYWC_ERROR, "Failed with drmHandleEvent failed");
        wlr_backend_destroy(drm->wlr_backend);
    }

    return 1;
}

static void drm_connector_disconnect(struct drm_connector *conn)
{
    if (conn->status == DRM_MODE_DISCONNECTED) {
        return;
    }
    wlr_output_destroy(&conn->output);
}

static void drm_connector_destroy(struct drm_connector *conn)
{
    drm_connector_disconnect(conn);

    wl_list_remove(&conn->link);
    free(conn);
}

void drm_update_connector(struct drm_device *drm, struct session_device_hotplug_event *event)
{
    if (event == NULL || event->connector_id == 0) {
        return;
    }

    struct drm_connector *conn;
    wl_list_for_each(conn, &drm->connectors, link) {
        if (event->connector_id != conn->id) {
            continue;
        }

        drmModeConnector *drm_conn = drmModeGetConnector(drm->fd, conn->id);
        if (!drm_conn) {
            kywc_log_errno(KYWC_ERROR, "Failed to get DRM connector");
            continue;
        }

        if (conn->status == DRM_MODE_CONNECTED && drm_conn->connection == DRM_MODE_DISCONNECTED) {
            kywc_log(KYWC_INFO, "Monitor DRM connector %" PRIu32 " on %s Changed",
                     event->connector_id, drm->name);

            // the connector disconnection is detected update_drm_connectors
            // disconnect it so that the client will modeset and
            // rerender when the session is activated again.
            wlr_output_destroy(&conn->output);
            if (!conn->crtc) {
                continue;
            }
            // After changing the session, the CRTC ID might have changed,
            // which caused the destroy output commit to fail.
            drm_plane_fb_finish(conn->crtc->primary);
            drm_plane_fb_finish(conn->crtc->cursor);
            drm_fb_clear_fb(&conn->cursor_pending_fb);

            conn->crtc = NULL;
            conn->cursor_enabled = false;
            break;
        }

        drmModeFreeConnector(drm_conn);
    }
}

void drm_restore_connectors(struct drm_device *drm)
{
    drm_scan_connectors(drm, NULL);

    struct drm_connector *conn;
    wl_list_for_each(conn, &drm->connectors, link) {
        struct wlr_output_state state;
        wlr_output_state_init(&state);

        bool enabled = conn->status != DRM_MODE_DISCONNECTED && conn->output.enabled;
        wlr_output_state_set_enabled(&state, enabled);
        if (enabled) {
            if (conn->output.current_mode != NULL) {
                wlr_output_state_set_mode(&state, conn->output.current_mode);
            } else {
                wlr_output_state_set_custom_mode(&state, conn->output.width, conn->output.height,
                                                 conn->output.refresh);
            }
        }

        if (!drm_commit_connector_state(conn, &state, false)) {
            kywc_log(KYWC_ERROR, "Failed to restore state after VT switch");
        }
        wlr_output_state_finish(&state);
    }
}

void drm_scan_connectors(struct drm_device *drm, struct session_device_hotplug_event *event)
{
    if (event != NULL && event->connector_id != 0) {
        kywc_log(KYWC_INFO, "Scanning DRM connector %" PRIu32 " on %s", event->connector_id,
                 drm->name);
    } else {
        kywc_log(KYWC_INFO, "Scanning DRM connectors on %s", drm->name);
    }

    drmModeRes *res = drmModeGetResources(drm->fd);
    if (!res) {
        kywc_log_errno(KYWC_ERROR, "Failed to get DRM resources");
        return;
    }

    size_t seen_len = wl_list_length(&drm->connectors);
    // +1 so length can never be 0, which is undefined behaviour.
    // Last element isn't used.
    bool seen[seen_len + 1];
    memset(seen, false, sizeof(seen));
    struct drm_connector *new_outputs[res->count_connectors + 1];

    size_t new_outputs_len = 0;
    for (int i = 0; i < res->count_connectors; i++) {
        uint32_t conn_id = res->connectors[i];

        ssize_t index = -1;
        struct drm_connector *c, *conn = NULL;
        wl_list_for_each(c, &drm->connectors, link) {
            index++;
            if (c->id == conn_id) {
                conn = c;
                break;
            }
        }

        if (conn && conn->lease) {
            continue;
        }

        // If the hotplug event contains a connector ID, ignore any other
        // connector.
        if (event != NULL && event->connector_id != 0 && event->connector_id != conn_id) {
            if (conn) {
                seen[index] = true;
            }
            continue;
        }

        drmModeConnector *drm_conn = drmModeGetConnector(drm->fd, conn_id);
        if (!drm_conn) {
            kywc_log_errno(KYWC_ERROR, "Failed to get DRM connector");
            continue;
        }

        /* can not find in device::connector_list */
        if (!conn) {
            conn = drm_connector_create(drm, drm_conn);
            if (!conn) {
                drmModeFreeConnector(drm_conn);
                continue;
            }
        } else {
            seen[index] = true;
        }

        // This can only happen *after* hotplug, since we haven't read the
        // connector properties yet
        uint32_t linksta_prop = conn->props[DRM_CONNECTOR_PROP_LINK_STATUS].id;
        if (linksta_prop) {
            uint64_t link_status;
            if (!drm_get_property_value(drm->fd, DRM_MODE_OBJECT_CONNECTOR, conn->id,
                                        &conn->props[DRM_CONNECTOR_PROP_LINK_STATUS],
                                        &link_status)) {
                kywc_log(KYWC_ERROR, "Failed to get link status prop");
                drmModeFreeConnector(drm_conn);
                continue;
            }
            if (link_status == DRM_MODE_LINK_STATUS_BAD) {
                kywc_log(KYWC_WARN, "Bad link detected, disconnect %s", conn->name);
                // We need to reload our list of modes and force a modeset
                drm_connector_disconnect(conn);
            }
        }

        if (conn->status == DRM_MODE_DISCONNECTED && drm_conn->connection == DRM_MODE_CONNECTED) {
            if (!drm_connector_connect(conn, drm_conn)) {
                kywc_log(KYWC_ERROR, "Failed to connect DRM connector");
                drmModeFreeConnector(drm_conn);
                continue;
            }
            new_outputs[new_outputs_len++] = conn;
        } else if (conn->status == DRM_MODE_CONNECTED &&
                   drm_conn->connection != DRM_MODE_CONNECTED) {
            kywc_log(KYWC_INFO, "'%s' disconnected", conn->name);
            drm_connector_disconnect(conn);
        } else if (conn->status == DRM_MODE_CONNECTED &&
                   drm_conn->connection == DRM_MODE_CONNECTED) {
            struct drm_crtc *current_crtc = get_drm_connector_current_crtc(conn, drm_conn);
            if (conn->crtc != current_crtc) {
                kywc_log(KYWC_WARN, "Be carefull crtc changed when session active!");
                if (current_crtc &&
                    drmModeSetCrtc(drm->fd, current_crtc->id, 0, 0, 0, NULL, 0, NULL) != 0) {
                    kywc_log(KYWC_ERROR, "Failed to close Current Crtc Failed!");
                }
            }
        }

        drmModeFreeConnector(drm_conn);
    }

    drmModeFreeResources(res);

    // Iterate in reverse order because we'll remove items from the list and
    // still want indices to remain correct.
    struct drm_connector *conn, *tmp_conn;
    size_t index = wl_list_length(&drm->connectors);
    wl_list_for_each_reverse_safe(conn, tmp_conn, &drm->connectors, link) {
        index--;
        if (index >= seen_len || seen[index]) {
            continue;
        }

        kywc_log(KYWC_INFO, "'%s' disappeared", conn->name);
        drm_connector_destroy(conn);
    }

    for (size_t i = 0; i < new_outputs_len; ++i) {
        struct drm_connector *conn = new_outputs[i];
        kywc_log(KYWC_DEBUG, "Requesting modeset, emit new_output");
        wl_signal_emit_mutable(&drm->wlr_backend->events.new_output, &conn->output);
    }
}

void drm_device_destroy(struct drm_device *drm)
{
    if (!drm) {
        return;
    }

    if (drm_device_get_parent(drm)) {
        drm_mgpu_renderer_finish(&drm->mgpu_renderer);
    }
    // Disconnect any active connectors so that the client will modeset and
    // rerender when the session is activated again.
    struct drm_connector *conn, *next;
    wl_list_for_each_safe(conn, next, &drm->connectors, link) {
        conn->crtc = NULL;
        drm_connector_destroy(conn);
    }

    struct drm_page_flip *pageflip, *pageflip_tmp;
    wl_list_for_each_safe(pageflip, pageflip_tmp, &drm->page_flips, link) {
        drm_destroy_page_flip(pageflip);
    }

    for (size_t i = 0; i < drm->num_crtcs; ++i) {
        struct drm_crtc *crtc = &drm->crtcs[i];
        if (crtc->mode_id && crtc->own_mode_id) {
            drmModeDestroyPropertyBlob(drm->fd, crtc->mode_id);
        }
        if (crtc->gamma_lut) {
            drmModeDestroyPropertyBlob(drm->fd, crtc->gamma_lut);
        }
        if (crtc->ctm) {
            drmModeDestroyPropertyBlob(drm->fd, crtc->ctm);
        }
    }

    free(drm->crtcs);

    for (size_t i = 0; i < drm->num_planes; ++i) {
        struct drm_plane *plane = &drm->planes[i];
        drm_plane_fb_finish(plane);
        wlr_drm_format_set_finish(&plane->formats);
    }
    free(drm->planes);

    struct drm_fb *fb, *fb_tmp;
    wl_list_for_each_safe(fb, fb_tmp, &drm->fbs, link) {
        drm_fb_destroy_fb(fb);
    }

    wl_event_source_remove(drm->drm_event);

    free(drm->name);
    free(drm);
}

struct drm_device *drm_device_create(int fd, struct wlr_backend *backend,
                                     struct wl_display *display)
{
    struct drm_device *drm = calloc(1, sizeof(*drm));
    if (!drm) {
        return NULL;
    }

    char *name = drmGetDeviceNameFromFd2(fd);
    drmVersion *version = drmGetVersion(fd);
    if (!version) {
        kywc_log_errno(KYWC_ERROR, "Failed with drmGetVersion");
        goto error;
    }
    kywc_log(KYWC_INFO, "Initializing DRM backend for %s (%s)", name, version->name);
    drmFreeVersion(version);

    drm->fd = fd;
    drm->name = name;
    drm->wlr_backend = backend;
    drm->display = display;
    drm->session_active = true;

    struct wl_event_loop *event_loop = wl_display_get_event_loop(display);
    drm->drm_event =
        wl_event_loop_add_fd(event_loop, drm->fd, WL_EVENT_READABLE, drm_handle_event, drm);

    if (!drm->drm_event) {
        kywc_log(KYWC_ERROR, "Failed to create DRM event source");
        goto error;
    }

    if (!drm_check_features(drm)) {
        goto error_event;
    }

    if (!drm_init_resources(drm)) {
        goto error_event;
    }

    wl_list_init(&drm->connectors);
    wl_list_init(&drm->fbs);
    wl_list_init(&drm->page_flips);

    return drm;

error_event:
    wl_event_source_remove(drm->drm_event);
error:
    free(name);
    free(drm);
    return NULL;
}

struct wlr_output_mode *wlr_output_add_mode(struct wlr_output *wlr_output, int32_t width,
                                            int32_t height, int32_t refresh)
{
    struct drm_connector *conn = drm_connector_from_output(wlr_output);

    struct wlr_output_mode *wlr_mode;
    wl_list_for_each(wlr_mode, &conn->output.modes, link) {
        if (wlr_mode->width == width && wlr_mode->height == height &&
            (refresh == 0 || wlr_mode->refresh == refresh)) {
            return wlr_mode;
        }
    }

    drmModeModeInfo drm_mode;
    drm_util_generate_cvt_mode(&drm_mode, width, height, refresh * 0.001);

    struct drm_mode *mode = drm_mode_create(&drm_mode);
    if (!mode) {
        return NULL;
    }

    wl_list_insert(&conn->output.modes, &mode->wlr_mode.link);

    kywc_log(KYWC_INFO, "%s: add custom mode %" PRId32 "x%" PRId32 "@%" PRId32, wlr_output->name,
             mode->wlr_mode.width, mode->wlr_mode.height, mode->wlr_mode.refresh);

    return &mode->wlr_mode;
}

struct drm_lease *drm_create_lease(struct wlr_output **outputs, size_t n_outputs, int *lease_fd_ptr)
{
    assert(outputs);

    if (n_outputs == 0) {
        kywc_log(KYWC_ERROR, "Can't lease 0 outputs");
        return NULL;
    }

    struct drm_backend *backend = drm_backend_from_wlr_backend(outputs[0]->backend);

    int n_objects = 0;
    uint32_t objects[4 * n_outputs + 1];
    for (size_t i = 0; i < n_outputs; ++i) {
        struct drm_connector *conn = drm_connector_from_output(outputs[i]);
        assert(conn->lease == NULL);

        if (conn->drm != backend->drm) {
            kywc_log(KYWC_ERROR, "Can't lease output from different backend");
            return NULL;
        }

        objects[n_objects++] = conn->id;
        kywc_log(KYWC_DEBUG, "Connector %d", conn->id);

        if (!drm_connector_allocate_crtc(conn)) {
            kywc_log(KYWC_ERROR, "Failled to allocate connector CRTC");
            return NULL;
        }

        objects[n_objects++] = conn->crtc->id;
        kywc_log(KYWC_DEBUG, "CRTC %d", conn->crtc->id);

        objects[n_objects++] = conn->crtc->primary->id;
        kywc_log(KYWC_DEBUG, "Primary plane %d", conn->crtc->primary->id);

        if (conn->crtc->cursor) {
            kywc_log(KYWC_DEBUG, "Cursor plane %d", conn->crtc->cursor->id);
            objects[n_objects++] = conn->crtc->cursor->id;
        }
    }

    assert(n_objects != 0);

    struct drm_lease *lease = calloc(1, sizeof(*lease));
    if (lease == NULL) {
        return NULL;
    }

    lease->backend = backend;
    wl_signal_init(&lease->events.destroy);

    kywc_log(KYWC_DEBUG, "Issuing DRM lease with %d objects", n_objects);
    int lease_fd =
        drmModeCreateLease(backend->drm->fd, objects, n_objects, O_CLOEXEC, &lease->lessee_id);
    if (lease_fd < 0) {
        free(lease);
        return NULL;
    }
    *lease_fd_ptr = lease_fd;

    kywc_log(KYWC_DEBUG, "Issued DRM lease %" PRIu32, lease->lessee_id);
    for (size_t i = 0; i < n_outputs; ++i) {
        struct drm_connector *conn = drm_connector_from_output(outputs[i]);
        conn->lease = lease;
        conn->crtc->lease = lease;
        drm_connector_disconnect(conn);
    }

    return lease;
}

static void drm_lease_destroy(struct drm_lease *lease)
{
    struct drm_backend *backend = lease->backend;
    struct drm_device *drm = backend->drm;

    wl_signal_emit_mutable(&lease->events.destroy, NULL);
    assert(wl_list_empty(&lease->events.destroy.listener_list));

    struct drm_connector *conn;
    wl_list_for_each(conn, &drm->connectors, link) {
        if (conn->lease == lease) {
            conn->lease = NULL;
        }
    }

    for (size_t i = 0; i < drm->num_crtcs; ++i) {
        if (drm->crtcs[i].lease == lease) {
            drm->crtcs[i].lease = NULL;
        }
    }

    free(lease);
    drm_scan_connectors(drm, NULL);
}

void drm_lease_terminate(struct drm_lease *lease)
{
    kywc_log(KYWC_INFO, "Terminating DRM lease %d", lease->lessee_id);

    struct drm_backend *backend = lease->backend;
    int ret = drmModeRevokeLease(backend->drm->fd, lease->lessee_id);
    if (ret < 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to terminate lease");
    }

    drm_lease_destroy(lease);
}

void drm_scan_leases(struct drm_device *drm)
{
    drmModeLesseeListRes *list = drmModeListLessees(drm->fd);
    if (!list) {
        kywc_log(KYWC_ERROR, "DrmModeListLessees failed");
        return;
    }

    struct drm_connector *conn;
    wl_list_for_each(conn, &drm->connectors, link) {
        if (!conn->lease) {
            continue;
        }

        bool found = false;
        for (size_t i = 0; i < list->count; i++) {
            if (list->lessees[i] == conn->lease->lessee_id) {
                found = true;
                break;
            }
        }
        if (!found) {
            kywc_log(KYWC_DEBUG, "DRM lease %" PRIu32 " has been terminated",
                     conn->lease->lessee_id);
            drm_lease_destroy(conn->lease);
        }
    }

    drmFree(list);
}
