// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SCENE_H_
#define _SCENE_H_

#include <kywc/boxes.h>

#include <pixman.h>

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_damage_ring.h>
#include <wlr/types/wlr_linux_dmabuf_v1.h>
#include <wlr/util/addon.h>
#include <wlr/util/box.h>

#include "render/pass.h"

struct wlr_buffer;
struct wlr_output;

struct scene_node;
struct scene_tree;
struct scene_rect;
struct scene_buffer;

struct scene_output;
struct scene_output_layout;
struct scene_output_state_options;

struct scene_render_target;
struct scene_round_corner {
    /* corner position */
    int x, y;
    int radius;
};

typedef void (*scene_node_destroy_func_t)(struct scene_node *node);

typedef struct scene_node *(*scene_node_accept_input_func_t)(struct scene_node *node, int lx,
                                                             int ly, double px, double py,
                                                             double *rx, double *ry);

typedef void (*scene_node_update_outputs_func_t)(struct scene_node *node, int lx, int ly,
                                                 struct wl_list *outputs,
                                                 struct scene_output *ignore,
                                                 struct scene_output *force);

typedef void (*scene_node_collect_damage_func_t)(struct scene_node *node, int lx, int ly,
                                                 bool parent_enabled, uint32_t damage_type,
                                                 pixman_region32_t *damage,
                                                 pixman_region32_t *invisible,
                                                 pixman_region32_t *affected);

typedef void (*scene_node_get_opaque_region_func_t)(struct scene_node *node,
                                                    pixman_region32_t *opaque);

typedef void (*scene_node_get_round_corner_func_t)(struct scene_node *node,
                                                   struct scene_round_corner corner[static 4]);

typedef void (*scene_node_get_bounding_box_func_t)(struct scene_node *node, struct wlr_box *box);

typedef void (*scene_node_get_affected_bounding_box_func_t)(struct scene_node *node, uint32_t type,
                                                            struct kywc_box *box);

typedef void (*scene_node_push_damage_func_t)(struct scene_node *node,
                                              struct scene_node *damage_node, uint32_t damage_type,
                                              pixman_region32_t *damage);

typedef void (*scene_node_render_func_t)(struct scene_node *node, int lx, int ly,
                                         struct scene_render_target *target);

struct scene_node_interface {
    /**
     * Check the node is accepted input event in the box.
     */
    scene_node_accept_input_func_t accept_input;
    /**
     * Update node output state.
     */
    scene_node_update_outputs_func_t update_outputs;
    /**
     * Collect all nodes damage region.
     */
    scene_node_collect_damage_func_t collect_damage;
    /**
     * Get node opaque region, logic coordinate system.
     */
    scene_node_get_opaque_region_func_t get_opaque_region;
    /**
     * Get node round corner radius with position on the node.
     */
    scene_node_get_round_corner_func_t get_round_corner;
    /**
     * Generate a rendering instance for the node and
     * start the rendering instance generation function for the child nodes.
     */
    scene_node_render_func_t render;
    /**
     * Get node bounding box.
     */
    scene_node_get_bounding_box_func_t get_bounding_box;
    /**
     * Get node bounding box what affected by effects.
     */
    scene_node_get_affected_bounding_box_func_t get_affected_bounding_box;
    /**
     * Push damgae to output.
     */
    scene_node_push_damage_func_t push_damage;
    /**
     * Private method, call it by scene_node_destroy.
     */
    scene_node_destroy_func_t destroy;
};

/* scene node type */
enum scene_node_type {
    SCENE_NODE_TREE,
    SCENE_NODE_RECT,
    SCENE_NODE_BUFFER,
};

/* scene role type */
enum scene_role_type {
    SCENE_ROLE_NONE = 0,

    SCENE_ROLE_ROOT,
    SCENE_ROLE_LAYER,
    SCENE_ROLE_WORKSPACE,

    SCENE_ROLE_VIEW_PROXY,
    SCENE_ROLE_VIEW,
    SCENE_ROLE_XDG_TOPLEVEL,
    SCENE_ROLE_XDG_POPUP,
    SCENE_ROLE_XDG_SURFACE,
    SCENE_ROLE_SUBSURFACE,
    SCENE_ROLE_XWAYLAND_WINDOW,
    SCENE_ROLE_XWAYLAND_UNMANAGED,
    SCENE_ROLE_LAYER_SURFACE,
    SCENE_ROLE_DRAG_ICON,
    SCENE_ROLE_SURFACE,

    SCENE_ROLE_SSD,
    SCENE_ROLE_SSD_TITLEBAR,
    SCENE_ROLE_SSD_BUTTONS,

    SCENE_ROLE_MENU,
    SCENE_ROLE_SUBMEBU,
    SCENE_ROLE_MENU_ITEM,

    SCENE_ROLE_WALLPAPER,
    SCENE_ROLE_DECO,
    SCENE_ROLE_SNAPSHOT,

    SCENE_ROLE_LAST,
};

enum scene_damage_type {
    SCENE_DAMAGE_NONE = 0,
    /* the damage will not affect the visible region of the node */
    SCENE_DAMAGE_HARMLESS = 1 << 0,
    /* the damage will affect the visible region of the node */
    SCENE_DAMAGE_HARMFUL = 1 << 1,
    SCENE_DAMAGE_BOTH = (1 << 2) - 1,
};

enum scene_round_corner_index {
    SCENE_ROUND_CORNER_RB, // right-bottom
    SCENE_ROUND_CORNER_RT, // right-top
    SCENE_ROUND_CORNER_LB, // left-bottom
    SCENE_ROUND_CORNER_LT, // left-top
};

enum scene_bounding_type {
    /* bounding unaffected by effects */
    SCENE_BOUNDING_WITHOUT_EFFECT = 0,
    /* bounding affected by effects */
    SCENE_BOUNDING_WITH_EFFECT = 1,
    /* bounding without the node who has effect */
    SCENE_BOUNDING_WITHOUT_EFFECT_NODE = 2,
    /* bounding without shadow and effects */
    SCENE_BOUNDING_WITHOUT_SSD_SHADOW = 3,
};

enum scene_blur_type {
    SCENE_BLUR_REALTIME = 0,
    SCENE_BLUR_WALLPAPER = 1,
    SCENE_BLUR_SCREENLOCK = 2,
};

struct blur_info {
    enum scene_blur_type type;

    float alpha;
    float offset;
    uint32_t iterations;
    /* region that mark as blur area */
    pixman_region32_t region;
};

struct scene_add_effect_event {
    const struct effect *effect;
    bool is_subeffect;

    bool allow_add;
    /* render data of exist effect entity */
    void *render_data;
};

struct scene_node_role_interface {
    const char *(*desc)(const struct scene_node *node, void *data);
    void (*log)(const struct scene_node *node, uint32_t depth, void *data);
};

struct scene_node {
    struct scene_tree *parent;
    struct wl_list link;

    enum scene_node_type type;
    struct {
        enum scene_role_type type;
        const struct scene_node_role_interface *impl;
        void *data;
    } role;

    bool enabled;
    bool input_bypassed;
    /* emit damage event for children if node is disabled */
    uint32_t force_damage_event;

    int x, y;
    /* round corner radius */
    int radius[4];

    /* node damage type after last collect_damage */
    uint32_t damage_type;
    /* enabled state after last collect_damage */
    bool last_enabled;

    bool has_effect;
    bool has_blur;
    struct blur_info blur;
    /**
     * region that should render for up node blur,
     * must in damage region, maybe don't in visible region.
     */
    pixman_region32_t extend_render_region;

    pixman_region32_t visible_region;
    /* region that can accept input */
    pixman_region32_t input_region;
    /* region that can render */
    pixman_region32_t clip_region;

    /* impl.xxx MUST not be NULL */
    struct scene_node_interface impl;

    struct {
        struct wl_signal damage;
        struct wl_signal destroy;
        struct wl_signal add_effect;
    } events;

    struct wlr_addon_set addons;

    void *data;

    bool sent_dmabuf_feedback;
};

struct scene_tree {
    struct scene_node node;
    struct wl_list children;
};

struct scene {
    struct scene_tree tree;
    scene_node_destroy_func_t tree_destroy;

    struct wl_list outputs;

    /* damage region after collect_damage based in node's visible region */
    pixman_region32_t collected_damage;
    /* invisible region after collect_damage */
    pixman_region32_t collected_invisible;
    /* damage region pushed by nodes */
    pixman_region32_t pushed_damage;

    struct wlr_linux_dmabuf_v1 *linux_dmabuf_v1;
    struct wl_listener linux_dmabuf_v1_destroy;
};

struct scene_rect {
    struct scene_node node;
    scene_node_destroy_func_t node_destroy;

    int width, height;
    float color[4];
};

typedef bool (*scene_buffer_point_accepts_input_func_t)(struct scene_buffer *buffer, double *sx,
                                                        double *sy);

struct scene_outputs_update_event {
    struct scene_output *primary;
    struct scene_output **active;
    size_t size;
};

struct scene_output_sample_event {
    struct scene_output *output;
    bool direct_scanout;

    struct drm_syncobj_timeline *release_timeline;
    uint64_t release_point;
};

struct scene_buffer {
    struct scene_node node;
    scene_node_destroy_func_t node_destroy;

    /* May be NULL */
    struct wlr_buffer *buffer;
    /* May be NULL */
    struct wlr_texture *texture;

    struct wl_listener renderer_destroy;

    struct wlr_fbox src_box;
    int dst_width, dst_height;
    enum wl_output_transform transform;

    enum render_texture_repeat_mode repeat_mode;
    float opacity;
    pixman_region32_t opaque_region;

    /**
     * The output that the largest area of this buffer is displayed on.
     * This may be NULL if the buffer is not currently displayed on any
     * outputs. This is the output that should be used for frame callbacks,
     * presentation feedback, etc.
     */
    struct scene_output *primary_output;
    uint64_t active_outputs;

    struct drm_syncobj_timeline *wait_timeline;
    uint64_t wait_point;

    scene_buffer_point_accepts_input_func_t point_accepts_input;

    struct {
        struct wl_signal outputs_update; // scene_outputs_update_event
        struct wl_signal output_enter;   // struct scene_output
        struct wl_signal output_leave;   // struct scene_output
        struct wl_signal output_sample;  // scene_output_sample_event
        struct wl_signal frame_done;     // struct timespec
    } events;

    struct wlr_linux_dmabuf_feedback_v1_init_options prev_feedback_options;
};

struct scene_output {
    struct wlr_output *output;
    struct wl_list link;

    struct scene *scene;
    struct wlr_addon addon;
    struct wlr_addon_set addons;

    struct wlr_buffer *buffer;

    struct wlr_damage_ring damage_ring;
    pixman_region32_t collected_damage;
    pixman_region32_t frame_damage; // in layout logical coord

    struct kywc_box geometry;
    struct wl_listener output_configure;

    struct drm_syncobj_timeline *in_timeline;
    uint64_t in_point;
    struct drm_syncobj_timeline *out_timeline;
    uint64_t out_point;

    /**
     * The viewport is selects a portion from the scene and displays it on the output.
     * If src is not empty, the scene content is cropped to the provided rectangle.
     */
    struct {
        struct kywc_box src;
        int dst_width, dst_height;
    } viewport;

    struct {
        /* emit if viewport changed */
        struct wl_signal viewport;
        struct wl_signal frame;
        struct wl_signal destroy;
    } events;

    uint8_t index;
    bool prev_scanout;
    bool direct_scanout;

    struct wl_listener output_damage;
    struct wl_listener output_needs_frame;
};

static inline void scene_node_set_role(struct scene_node *node, enum scene_role_type role,
                                       const struct scene_node_role_interface *impl, void *data)
{
    node->role.type = role;
    node->role.impl = impl;
    node->role.data = data;
}

void scene_node_log(struct scene_node *node, uint32_t max_depth);

void scene_node_destroy(struct scene_node *node);

struct scene *scene_create(void);

void scene_set_linux_dmabuf_v1(struct scene *scene, struct wlr_linux_dmabuf_v1 *linux_dmabuf_v1);

struct scene *scene_from_node(struct scene_node *node);

void scene_damage_whole(struct scene *scene);

void scene_add_damage(struct scene *scene, const pixman_region32_t *damage);

struct scene_tree *scene_tree_create(struct scene_tree *parent);

struct scene_tree *scene_tree_from_node(struct scene_node *node);

void scene_node_set_enabled(struct scene_node *node, bool enabled);

void scene_node_set_input_bypassed(struct scene_node *node, bool bypassed);

void scene_node_force_damage_event(struct scene_node *node, bool force);

void scene_node_set_position(struct scene_node *node, int x, int y);

void scene_node_place_above(struct scene_node *node, struct scene_node *sibling);

void scene_node_place_below(struct scene_node *node, struct scene_node *sibling);

void scene_node_raise_to_top(struct scene_node *node);

void scene_node_lower_to_bottom(struct scene_node *node);

void scene_node_reparent(struct scene_node *node, struct scene_tree *new_parent);

bool scene_node_coords(struct scene_node *node, int *lx_ptr, int *ly_ptr);

bool scene_node_is_enabled(struct scene_node *node);

struct scene_node *scene_node_at(struct scene_node *node, double lx, double ly, double *nx,
                                 double *ny);

void scene_node_set_input_region(struct scene_node *node, const pixman_region32_t *region);

void scene_node_set_clip_region(struct scene_node *node, const pixman_region32_t *region);

void scene_node_set_blur_region(struct scene_node *node, const pixman_region32_t *region);

void scene_node_set_blur_level(struct scene_node *node, uint32_t iterations, float offset);

void scene_node_set_blur_opacity(struct scene_node *node, float opacity);

void scene_node_set_blur_type(struct scene_node *node, enum scene_blur_type type);

/* if node hasn't blur, bur region is empty. */
void scene_node_get_blur_info(struct scene_node *node, struct blur_info *info);

void scene_node_set_radius(struct scene_node *node, const int radius[static 4]);

/* radius on bounding box */
void scene_node_get_radius(struct scene_node *node, int radius[static 4]);

bool scene_node_is_visible(struct scene_node *node);

void scene_node_push_damage(struct scene_node *node, enum scene_damage_type damage_type,
                            const pixman_region32_t *damage);

void scene_node_get_affected_bounding_box(struct scene_node *node, enum scene_bounding_type type,
                                          struct kywc_box *box);

void scene_node_render(struct scene_node *node, int lx, int ly, struct scene_render_target *target);

/**
 * scene rect
 */
struct scene_rect *scene_rect_create(struct scene_tree *parent, int width, int height,
                                     const float color[static 4]);

void scene_rect_set_size(struct scene_rect *rect, int width, int height);

void scene_rect_set_color(struct scene_rect *rect, const float color[static 4]);

struct scene_rect *scene_rect_from_node(struct scene_node *node);

bool scene_rect_render(struct scene_node *node, struct kywc_box geo, float color[4],
                       bool render_with_visibility, struct scene_render_target *target);

/**
 * scene buffer
 */
struct scene_buffer *scene_buffer_create(struct scene_tree *parent, struct wlr_buffer *buffer);

struct scene_buffer *scene_buffer_from_node(struct scene_node *node);

void scene_buffer_set_buffer(struct scene_buffer *scene_buffer, struct wlr_buffer *buffer);

void scene_buffer_set_buffer_with_damage(struct scene_buffer *scene_buffer,
                                         struct wlr_buffer *buffer,
                                         const pixman_region32_t *region);

void scene_buffer_set_timeline(struct scene_buffer *scene_buffer,
                               struct drm_syncobj_timeline *timeline, uint64_t point);

void scene_buffer_set_opacity(struct scene_buffer *scene_buffer, float opacity);

void scene_buffer_set_opaque_region(struct scene_buffer *scene_buffer,
                                    const pixman_region32_t *region);

void scene_buffer_set_source_box(struct scene_buffer *scene_buffer, const struct wlr_fbox *box);

void scene_buffer_set_dest_size(struct scene_buffer *scene_buffer, int width, int height);

void scene_buffer_set_transform(struct scene_buffer *scene_buffer,
                                enum wl_output_transform transform);

void scene_buffer_set_repeat_mode(struct scene_buffer *scene_buffer,
                                  enum render_texture_repeat_mode repeat_mode);

void scene_buffer_send_dmabuf_feedback(
    const struct scene *scene, struct scene_buffer *scene_buffer,
    const struct wlr_linux_dmabuf_feedback_v1_init_options *options);

bool scene_buffer_is_clipped(struct scene_buffer *scene_buffer);

struct wlr_texture *scene_buffer_get_texture(struct scene_buffer *scene_buffer,
                                             struct wlr_renderer *renderer);

#if HAVE_ALPHA_MODIFIER
bool alpha_modifier_v1_create(struct wl_display *display);
#else
static __attribute__((unused)) inline bool alpha_modifier_v1_create(struct wl_display *display)
{
    return false;
}
#endif

bool tearing_control_manager_v1_create(struct wl_display *display);

bool presentation_create(struct wl_display *display);

/**
 * scene output
 */
struct scene_output *scene_get_scene_output(struct scene *scene, struct wlr_output *output);

struct output *scene_get_first_enabled_output(struct scene *scene);

void scene_output_damage_whole(struct scene_output *scene_output);

void scene_output_set_viewport_source_box(struct scene_output *scene_output,
                                          const struct kywc_box *src_box);

bool scene_output_commit(struct scene_output *scene_output,
                         const struct scene_output_state_options *options);

void scene_output_send_frame_done(struct scene_output *scene_output, struct timespec *now);

struct scene_output *scene_output_create(struct scene *scene, struct wlr_output *output);

void scene_output_destroy(struct scene_output *scene_output);

#endif
