// SPDX-FileCopyrightText: 2025 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _BACKEND_DRM_P_H_
#define _BACKEND_DRM_P_H_

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <wlr/backend.h>
#include <wlr/backend/session.h>
#include <wlr/render/drm_format_set.h>

#include "backend/drm.h"

struct drm_device;
struct wlr_output;
struct wlr_buffer;
struct output;
struct drm_output_state;
struct wlr_output_state;
struct kywc_output_state;

enum kywc_output_color_filter;

enum drm_kms_mode {
    DRM_KMS_MODE_ATOMIC = 0,
    DRM_KMS_MODE_LEGACY,
};

enum drm_output_state_field {
    DRM_OUTPUT_STATE_BRIGHTNESS = 1 << 1,
    DRM_OUTPUT_STATE_GAMMA_LUT = 1 << 2,
    DRM_OUTPUT_STATE_RGB_CTM = 1 << 3,
    DRM_OUTPUT_STATE_DAMAGE = 1 << 4,
    DRM_OUTPUT_STATE_RGB_RANGE = 1 << 5,
    DRM_OUTPUT_STATE_OVERSCAN = 1 << 6,
    DRM_OUTPUT_STATE_SCALING_MODE = 1 << 7,
};

enum drm_crtc_prop {
    DRM_CRTC_PROP_ACTIVE = 0,
    DRM_CRTC_PROP_MODE_ID,
    DRM_CRTC_PROP_OUT_FENCE_PTR,
    DRM_CRTC_PROP_VRR_ENABLE,
    DRM_CRTC_PROP_CTM,
    DRM_CRTC_PROP_GAMMA_LUT,
    DRM_CRTC_PROP_GAMMA_LUT_SIZE,
    DRM_CRTC_PROP_COUNT,
};

enum drm_plane_prop {
    DRM_PLANE_PROP_TYPE = 0,
    DRM_PLANE_PROP_FB_ID,
    DRM_PLANE_PROP_CRTC_ID,
    DRM_PLANE_PROP_CRTC_X,
    DRM_PLANE_PROP_CRTC_Y,
    DRM_PLANE_PROP_CRTC_W,
    DRM_PLANE_PROP_CRTC_H,
    DRM_PLANE_PROP_SRC_X,
    DRM_PLANE_PROP_SRC_Y,
    DRM_PLANE_PROP_SRC_W,
    DRM_PLANE_PROP_SRC_H,
    DRM_PLANE_PROP_IN_FORMATS,
    DRM_PLANE_PROP_FB_DAMAGE_CLIPS,
    DRM_PLANE_PROP_ROTATION,
    DRM_PLANE_PROP_HOTSPOT_X,
    DRM_PLANE_PROP_HOTSPOT_Y,
    DRM_PLANE_PROP_COLOR_ENCODING,
    DRM_PLANE_PROP_COLOR_RANGE,
    DRM_PLANE_PROP_COUNT,
};

enum drm_connector_prop {
    DRM_CONNECTOR_PROP_EDID = 0,
    DRM_CONNECTOR_PROP_DPMS,
    DRM_CONNECTOR_PROP_LINK_STATUS,
    DRM_CONNECTOR_PROP_NON_DESKTOP,
    DRM_CONNECTOR_PROP_CRTC_ID,
    DRM_CONNECTOR_PROP_CONTENT_TYPE,
    DRM_CONNECTOR_PROP_MAX_BPC,
    DRM_CONNECTOR_PROP_PANEL_ORIENTATION,
    DRM_CONNECTOR_PROP_SUBCONNECTOR,
    DRM_CONNECTOR_PROP_VRR_CAPABLE,
    DRM_CONNECTOR_PROP_BRIGHTNESS,
    DRM_CONNECTOR_PROP_RGB_RANGE,
    DRM_CONNECTOR_PROP_OVERSCAN,
    DRM_CONNECTOR_PROP_SCALING_MODE,
    DRM_CONNECTOR_PROP_COUNT,
};

enum drm_color_encoding {
    DRM_COLOR_ENCODING_BT601 = 0,
    DRM_COLOR_ENCODDING_BT709,
};

enum drm_color_range {
    DRM_COLOR_RANGE_LIMITTED = 0,
    DRM_COLOR_RANGE_FULL,
};

// Part of match_obj
enum {
    UNMATCHED = (uint32_t)-1,
    SKIP = (uint32_t)-2,
};

struct drm_prop_info {
    const char *name; /* name as string (static, not freed) */
    uint32_t id;
};

struct drm_fb {
    struct wlr_buffer *wlr_buf;
    struct wlr_addon addon;
    struct drm_device *drm;
    struct wl_list link;

    uint32_t id; // fb-id
};

struct drm_lease {
    int fd;
    uint32_t lessee_id;
    struct drm_backend *backend;

    struct {
        struct wl_signal destroy;
    } events;

    void *data;
};

struct drm_backend {
    struct wlr_backend wlr_backend;

    struct drm_backend *parent;

    struct wlr_device *dev;
    struct drm_device *drm;

    struct wlr_session *session;

    struct wl_listener display_destroy;
    struct wl_listener session_destroy;
    struct wl_listener session_active;
    struct wl_listener parent_destroy;

    struct wl_listener device_change;
    struct wl_listener device_remove;
};

struct drm_renderer {
    struct wlr_renderer *wlr_rend;
    struct wlr_allocator *allocator;

    struct wlr_drm_format_set formats;
};

struct swapchain_option {
    int width;
    int height;
    struct wlr_output *output;
    struct wlr_drm_format *drm_format;
    const struct wlr_output_state *state;
};

struct surf_swapchain {
    int width, height;
    struct swapchain *swapchain;
    // TODO: check buffer matched
};

struct drm_surface {
    struct wlr_renderer *wlr_rend;

    struct surf_swapchain *swapchain;
    struct surf_swapchain *dumb_swapchain;
};

struct drm_plane {
    struct wl_list link; /* drm_device::plane_list */

    uint32_t type;
    uint32_t id;

    /* initialized required for multi-GPU setups, or single-GPU backend buffers */
    struct drm_surface multi_surf;

    /* Buffer submitted to the kernel, will be presented on next vblank */
    struct drm_fb *queued_fb;
    /* Buffer currently displayed on screen */
    struct drm_fb *current_fb;

    struct wlr_drm_format_set formats;

    struct drm_prop_info props[DRM_PLANE_PROP_COUNT];

    uint32_t crtc_id;
    uint32_t possible_crtcs; // mask

    bool enable_yuyv;
};

struct drm_crtc {
    struct drm_device *drm;
    struct drm_lease *lease;

    uint32_t id;
    uint32_t index; /* index of CRTC in resource arry */

    // Atomic modesetting only
    bool own_mode_id;
    uint32_t mode_id;
    uint32_t gamma_lut;
    uint32_t ctm;

    // Legacy only
    int legacy_gamma_size;

    struct drm_plane *primary;
    struct drm_plane *cursor;

    struct drm_prop_info props[DRM_CRTC_PROP_COUNT];
};

struct drm_page_flip {
    struct wl_list link; // drm_connector.pageflip

    struct drm_connector *connector;

    // True if DRM_MODE_PAGE_FLIP_ASYNC was set
    bool async;
};

struct drm_mode {
    struct wlr_output_mode wlr_mode;
    drmModeModeInfo drm_mode;
};

struct drm_output_state {
    uint32_t committed;

    uint32_t brightness;
    uint32_t overscan;
    uint32_t rgb_range;
    uint32_t scaling_mode;

    bool color_changed;

    /* render damage */
    pixman_region32_t damage;

    struct drm_color_lut *lut;
    size_t lut_size;

    struct drm_color_ctm rgb_ctm;
};

struct drm_render_target {
    struct wlr_renderer *wlr_rend;

    struct wlr_buffer *source;

    uint32_t brightness;
    uint32_t color_temp;
    const float *color_mat;
    const pixman_region32_t *damage;
    struct output *output;

    bool rgb_clear;
    bool use_yuyv;
};

struct drm_connector {
    struct wl_list link;      /* drm_device::connector_list */
    struct wlr_output output; // only valid if status != DISCONNECTED

    struct drm_device *drm;
    struct drm_lease *lease;

    char name[24];
    drmModeConnection status;
    uint32_t id;
    uint64_t max_bpc_bounds[2];
    uint8_t *edid;
    size_t edid_len;

    bool adaptive_sync_supported;
    uint32_t brightness;
    uint32_t rgb_range;
    uint32_t overscan;
    uint32_t scaling_mode;
    bool has_none_mode;

    struct drm_crtc *crtc;
    uint32_t possible_crtcs;

    /* Holds the properties for the connector */
    struct drm_prop_info props[DRM_CONNECTOR_PROP_COUNT];

    bool cursor_enabled;
    int cursor_x, cursor_y;
    int cursor_width, cursor_height;
    int cursor_hotspot_x, cursor_hotspot_y;
    /* Buffer to be submitted to the kernel on the next page-flip */
    struct drm_fb *cursor_pending_fb;
    struct wlr_buffer *cursor_buffer;

    // Last committed page-flip
    struct drm_page_flip *pending_page_flip;

    int32_t refresh;

    // output state
    struct drm_output_state state;
};

struct drm_connector_state {
    struct drm_connector *connector;
    const struct wlr_output_state *base;
    const struct drm_output_state *output_state;
    bool active;
    drmModeModeInfo mode;
    struct drm_fb *primary_fb;
    struct drm_fb *cursor_fb;

    // used by atomic
    uint32_t brightness;
    uint32_t overscan;
    uint32_t rgb_range;
    uint32_t scaling_mode;
    uint32_t mode_id;
    uint32_t gamma_lut;
    uint32_t rgb_ctm;
    uint32_t fb_damage_clips;
    bool vrr_enabled;
};

struct drm_device_state {
    bool modeset;
    bool nonblock;

    struct drm_connector_state *conn_state;
};

struct drm_device {
    struct wlr_backend *wlr_backend;
    struct wl_display *display;
    const struct drm_impl *impl;

    int fd;
    char *name;

    bool addfb2_modifiers;
    bool supports_tearing_page_flips;
    uint64_t cursor_width, cursor_height;

    enum drm_kms_mode mode;

    struct wl_event_source *drm_event;

    /* Only initialized on multi-GPU setups */
    struct drm_renderer mgpu_renderer;

    size_t num_crtcs;
    struct drm_crtc *crtcs;

    size_t num_planes;
    struct drm_plane *planes;

    /* drm_connector::link */
    struct wl_list connectors;

    /* drm_pageflip::link */
    struct wl_list page_flips;
    /* drm_fb::link */
    struct wl_list fbs;

    bool session_active; /* wlr_session::active */
};

/**
 * Helper to create new DRM sub-backends on GPU hotplog
 */
struct drm_backend_monitor {
    struct wlr_backend *multi;
    struct wlr_backend *primary_drm;
    struct wlr_session *session;

    struct wl_listener multi_destroy;
    struct wl_listener primary_drm_destroy;
    struct wl_listener session_destroy;
    struct wl_listener session_add_drm_card;
};

const char *pnp_get_manufacturer(const char code[static 3]);

struct drm_device *drm_device_create(int fd, struct wlr_backend *backend,
                                     struct wl_display *display);

void drm_device_destroy(struct drm_device *drm);

void drm_restore_connectors(struct drm_device *drm);

void drm_scan_connectors(struct drm_device *drm, struct wlr_device_hotplug_event *event);

void drm_update_connector(struct drm_device *drm, struct wlr_device_hotplug_event *event);

size_t drm_get_crtc_gamma_lut_size(struct drm_crtc *crtc);

/* property */
bool drm_get_property_value(int fd, uint32_t type, uint32_t obj, struct drm_prop_info *prop,
                            uint64_t *ret);

void *drm_get_property_blob(int fd, uint32_t type, uint32_t obj, struct drm_prop_info *prop,
                            size_t *ret_len);

char *drm_get_property_enum_list(int fd, uint32_t type, uint32_t obj, struct drm_prop_info *prop);

char *drm_get_property_enum(int fd, uint32_t type, uint32_t obj, struct drm_prop_info *prop);

bool drm_get_property_range(int fd, uint32_t prop_id, uint64_t *min, uint64_t *max);

bool drm_get_connector_props(int fd, uint32_t obj, struct drm_prop_info *out);

bool drm_get_plane_props(int fd, uint32_t obj, struct drm_prop_info *out);

bool drm_get_crtc_props(int fd, uint32_t obj, struct drm_prop_info *out);

/* drm-fb */
struct drm_fb *drm_fb_lock_fb(struct drm_fb *fb);

void drm_fb_clear_fb(struct drm_fb **fb_ptr);

void drm_fb_destroy_fb(struct drm_fb *fb);

void drm_fb_copy_fb(struct drm_fb **new, struct drm_fb *old);

void drm_fb_move_fb(struct drm_fb **new, struct drm_fb **old);

bool drm_fb_import_fb(struct drm_fb **fb_ptr, struct drm_device *drm, struct wlr_buffer *buf,
                      const struct wlr_drm_format_set *formats, bool use_yuyv);

/* util */
int32_t drm_util_calculate_refresh_rate(const drmModeModeInfo *mode);

enum wlr_output_mode_aspect_ratio drm_util_get_picture_aspect_ratio(const drmModeModeInfo *mode);

void drm_util_parse_edid(struct drm_connector *conn, const uint8_t *data, size_t len);

const char *drm_util_get_connector_status_str(drmModeConnection status);

size_t drm_util_match_obj(size_t num_objs, const uint32_t objs[static restrict num_objs],
                          size_t num_res, const uint32_t res[static restrict num_res],
                          uint32_t out[static restrict num_res]);

void drm_util_generate_cvt_mode(drmModeModeInfo *mode, int hdisplay, int vdisplay, float vrefresh);

/* renderer */
bool drm_mgpu_renderer_init(struct drm_device *drm, struct drm_renderer *renderer);

void drm_mgpu_renderer_finish(struct drm_renderer *renderer);

bool drm_plane_configure_surface_swapchain(struct drm_plane *plane, struct drm_connector *conn,
                                           struct drm_renderer *mgpu_renderer, int width,
                                           int height, uint32_t format,
                                           const struct wlr_output_state *state, bool use_yuyv);

struct wlr_buffer *drm_surface_blit(struct drm_surface *surface, struct drm_render_target *target);

void drm_surface_finish(struct drm_surface *surf);

/* kms */
bool drm_kms_commit(struct drm_device *drm, const struct drm_device_state *state,
                    struct drm_page_flip *page_flip, uint32_t flags, bool test_only);

/* drm lease */
void drm_scan_leases(struct drm_device *drm);

struct drm_lease *drm_create_lease(struct wlr_output **outputs, size_t n_outputs,
                                   int *lease_fd_ptr);

void drm_lease_terminate(struct drm_lease *lease);

struct drm_backend *drm_backend_from_wlr_backend(struct wlr_backend *wlr_backend);

struct drm_connector *drm_connector_from_output(struct wlr_output *wlr_output);

void drm_output_pending_resolution(struct wlr_output *output, const struct wlr_output_state *state,
                                   int *width, int *height);

bool drm_output_use_hw_brightness(struct wlr_output *output, const struct kywc_output_state *state);

bool drm_output_use_hw_color_temp(struct wlr_output *output, const struct kywc_output_state *state);

bool drm_output_use_hw_color_filter(struct wlr_output *output,
                                    const struct kywc_output_state *state);

void drm_output_set_state(struct output *output);

void drm_output_state_clear(struct drm_output_state *state);

void drm_color_temp_to_rgb(uint32_t color_temp, float *rgb);

const float *drm_output_get_color_filter_matrix(enum kywc_output_color_filter color_filter);

#if HAVE_DRM_LEASE_DEVICE
bool drm_lease_device_v1_create(struct drm_backend *backend);
#else
static __attribute__((unused)) inline bool drm_lease_device_v1_create(struct drm_backend *backend)
{
    return false;
}
#endif

#endif /* _BACKEND_DRM_P_H_ */
