// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/util/region.h>

#include "output.h"
#include "scene/render.h"
#include "scene_p.h"
#include "util/debug.h"
#include "util/macros.h"

/**
 * basic scene node
 */

static struct scene_node *node_accept_input(struct scene_node *node, int lx, int ly, double px,
                                            double py, double *rx, double *ry)
{
    kywc_log(KYWC_ERROR, "Need to implement accept_input interface!");
    assert(false);
    return NULL;
}

static void node_update_outputs(struct scene_node *node, int lx, int ly, struct wl_list *outputs,
                                struct scene_output *ignore, struct scene_output *force)
{
    kywc_log(KYWC_ERROR, "Need to implement update_outputs interface!");
    assert(false);
}

static void node_collect_damage(struct scene_node *node, int lx, int ly, bool parent_enabled,
                                uint32_t update_mask, pixman_region32_t *damage,
                                pixman_region32_t *invisible, pixman_region32_t *affected)
{
    kywc_log(KYWC_ERROR, "Need to implement collect_damage interface!");
    assert(false);
}

static void node_get_opaque_region(struct scene_node *node, pixman_region32_t *opaque)
{
    pixman_region32_clear(opaque);
}

static void node_render(struct scene_node *node, int lx, int ly, struct scene_render_target *target)
{
    kywc_log(KYWC_ERROR, "Need to implement render interface!");
    assert(false);
}

static void node_get_bounding_box(struct scene_node *node, struct wlr_box *box)
{
    kywc_log(KYWC_ERROR, "Need to implement get_bounding_box interface!");
    assert(false);
}

static void node_get_affected_bounding_box(struct scene_node *node, uint32_t type,
                                           struct kywc_box *box)
{
    struct wlr_box bbox;
    node->impl.get_bounding_box(node, &bbox);
    *box = (struct kywc_box){ bbox.x, bbox.y, bbox.width, bbox.height };
}

static void node_push_damage(struct scene_node *node, struct scene_node *damage_node,
                             uint32_t damage_type, pixman_region32_t *damage)
{
    if (!node->enabled && !node->force_damage_event) {
        return;
    }

    assert(damage != NULL);
    /* root node has own push_damage */
    assert(node->parent);

    /* emit damage when node content damaged or children damaged */
    if ((node == damage_node && node->damage_type & SCENE_DAMAGE_HARMLESS) || node != damage_node) {
        wl_signal_emit_mutable(&node->events.damage, damage_node);
    }

    if (!node->enabled) {
        return;
    }

    pixman_region32_translate(damage, node->x, node->y);
    node->parent->node.impl.push_damage(&node->parent->node, damage_node,
                                        damage_type | node->damage_type, damage);
}

static void node_destroy(struct scene_node *node)
{
    wlr_addon_set_finish(&node->addons);
    scene_node_set_enabled(node, false);

    pixman_region32_fini(&node->extend_render_region);
    pixman_region32_fini(&node->visible_region);
    pixman_region32_fini(&node->input_region);
    pixman_region32_fini(&node->clip_region);
    pixman_region32_fini(&node->blur.region);

    if (node->type == SCENE_NODE_BUFFER) {
        struct scene_buffer *scene_buffer = scene_buffer_from_node(node);
        assert(wl_list_empty(&scene_buffer->events.outputs_update.listener_list));
        assert(wl_list_empty(&scene_buffer->events.output_enter.listener_list));
        assert(wl_list_empty(&scene_buffer->events.output_leave.listener_list));
        assert(wl_list_empty(&scene_buffer->events.output_sample.listener_list));
        assert(wl_list_empty(&scene_buffer->events.frame_done.listener_list));
    }

    assert(wl_list_empty(&node->events.add_effect.listener_list));
    assert(wl_list_empty(&node->events.damage.listener_list));
    assert(wl_list_empty(&node->events.destroy.listener_list));

    wl_list_remove(&node->link);
    free(node);
}

void scene_node_init(struct scene_node *node, struct scene_tree *parent)
{
    *node = (struct scene_node){
        .parent = parent,
        .enabled = true,
        .damage_type = SCENE_DAMAGE_HARMFUL,
        .blur.iterations = 4,
        .blur.offset = 5.5f,
        .blur.alpha = 1.0f,
        .impl = {
            .accept_input = node_accept_input,
            .update_outputs = node_update_outputs,
            .collect_damage = node_collect_damage,
            .get_opaque_region = node_get_opaque_region,
            .render = node_render,
            .get_bounding_box = node_get_bounding_box,
            .get_affected_bounding_box = node_get_affected_bounding_box,
            .push_damage = node_push_damage,
            .destroy = node_destroy,
        },
    };

    wl_list_init(&node->link);
    wl_signal_init(&node->events.damage);
    wl_signal_init(&node->events.destroy);
    wl_signal_init(&node->events.add_effect);
    pixman_region32_init(&node->extend_render_region);
    pixman_region32_init(&node->visible_region);
    pixman_region32_init(&node->input_region);
    pixman_region32_init(&node->clip_region);
    pixman_region32_init(&node->blur.region);

    if (parent != NULL) {
        wl_list_insert(parent->children.prev, &node->link);
    }

    wlr_addon_set_init(&node->addons);
}

static void scene_node_do_log(struct scene_node *node, uint32_t depth, uint32_t max)
{
    if (depth > max) {
        return;
    }

    static const char *const roles[SCENE_ROLE_LAST] = {
        [SCENE_ROLE_NONE] = "unknown",
        [SCENE_ROLE_ROOT] = "root",
        [SCENE_ROLE_LAYER] = "layer",
        [SCENE_ROLE_WORKSPACE] = "workspace",
        [SCENE_ROLE_VIEW_PROXY] = "view_proxy",
        [SCENE_ROLE_VIEW] = "view",
        [SCENE_ROLE_XDG_TOPLEVEL] = "xdg_toplevel",
        [SCENE_ROLE_XDG_POPUP] = "xdg_popup",
        [SCENE_ROLE_XDG_SURFACE] = "xdg_surface",
        [SCENE_ROLE_SUBSURFACE] = "subsurface",
        [SCENE_ROLE_XWAYLAND_WINDOW] = "xwayland_window",
        [SCENE_ROLE_XWAYLAND_UNMANAGED] = "xwayland_unmanaged",
        [SCENE_ROLE_LAYER_SURFACE] = "layer_shell",
        [SCENE_ROLE_DRAG_ICON] = "drag_icon",
        [SCENE_ROLE_SURFACE] = "surface",
        [SCENE_ROLE_SSD] = "ssd",
        [SCENE_ROLE_SSD_TITLEBAR] = "ssd_titlebar",
        [SCENE_ROLE_SSD_BUTTONS] = "ssd_buttons",
        [SCENE_ROLE_MENU] = "menu",
        [SCENE_ROLE_SUBMEBU] = "submenu",
        [SCENE_ROLE_MENU_ITEM] = "menu_item",
        [SCENE_ROLE_WALLPAPER] = "wallpaper",
        [SCENE_ROLE_DECO] = "decoration",
    };

    static char *types[] = { "tree", "rect", "buffer" };

    const char *desc = NULL;
    if (node->role.impl && node->role.impl->desc) {
        desc = node->role.impl->desc(node, node->role.data);
    }

    kywc_log(KYWC_INFO, "%*s L%d %s (%s%s%s)", depth * 2, "", depth, types[node->type],
             roles[node->role.type], desc ? " " : "", desc ? desc : "");

    if (node->type == SCENE_NODE_TREE) {
        struct scene_tree *tree = scene_tree_from_node(node);
        struct scene_node *child;
        wl_list_for_each(child, &tree->children, link) {
            scene_node_do_log(child, depth + 1, max);
        }
    } else if (node->type == SCENE_NODE_RECT) {
        // struct scene_rect *rect = scene_rect_from_node(node);
    } else if (node->type == SCENE_NODE_BUFFER) {
        // struct scene_buffer *buffer = scene_buffer_from_node(node);
    }
}

void scene_node_log(struct scene_node *node, uint32_t max_depth)
{
    scene_node_do_log(node, 0, max_depth);
}

/**
 * scene tree
 */

struct scene_tree *scene_tree_from_node(struct scene_node *node)
{
    struct scene_tree *tree = wl_container_of(node, tree, node);
    return tree;
}

static struct scene_node *tree_accept_input(struct scene_node *node, int lx, int ly, double px,
                                            double py, double *rx, double *ry)
{
    /* skip disabled or input bypassed nodes */
    if (!node->enabled || node->input_bypassed) {
        return NULL;
    }

    struct scene_tree *scene_tree = scene_tree_from_node(node);
    struct scene_node *child, *found;
    double nx, ny;

    wl_list_for_each_reverse(child, &scene_tree->children, link) {
        found = child->impl.accept_input(child, lx + child->x, ly + child->y, px, py, &nx, &ny);
        if (found) {
            *rx = nx;
            *ry = ny;
            return found;
        }
    }

    return NULL;
}

static void tree_update_outputs(struct scene_node *node, int lx, int ly, struct wl_list *outputs,
                                struct scene_output *ignore, struct scene_output *force)
{
    // skip node enabled check
    struct scene_tree *scene_tree = scene_tree_from_node(node);
    struct scene_node *child;

    wl_list_for_each(child, &scene_tree->children, link) {
        child->impl.update_outputs(child, lx + child->x, ly + child->y, outputs, ignore, force);
    }
}

static void tree_collect_damage(struct scene_node *node, int lx, int ly, bool parent_enabled,
                                uint32_t damage_type, pixman_region32_t *damage,
                                pixman_region32_t *invisible, pixman_region32_t *affected)
{
    bool node_enabled = parent_enabled && node->enabled;
    struct scene_tree *scene_tree = scene_tree_from_node(node);

    struct scene_node *child;
    wl_list_for_each_reverse(child, &scene_tree->children, link) {
        child->impl.collect_damage(child, lx + child->x, ly + child->y, node_enabled,
                                   damage_type | child->damage_type, damage, invisible, affected);
    }

    node->last_enabled = node_enabled;
    node->damage_type = SCENE_DAMAGE_NONE;
}

static void tree_get_opaque_region(struct scene_node *node, pixman_region32_t *opaque)
{
    /* skip disabled */
    pixman_region32_clear(opaque);
    if (!node->enabled) {
        return;
    }

    struct scene_node *child;
    pixman_region32_t child_opaque;
    pixman_region32_init(&child_opaque);
    struct scene_tree *scene_tree = scene_tree_from_node(node);
    wl_list_for_each_reverse(child, &scene_tree->children, link) {
        child->impl.get_opaque_region(child, &child_opaque);
        pixman_region32_translate(&child_opaque, child->x, child->y);
        pixman_region32_union(opaque, opaque, &child_opaque);
    }
    pixman_region32_fini(&child_opaque);
}

static void tree_render(struct scene_node *node, int lx, int ly, struct scene_render_target *target)
{
    struct scene_tree *scene_tree = scene_tree_from_node(node);
    if (!node->enabled || wl_list_empty(&scene_tree->children)) {
        return;
    }

    /* from bottom to top */
    struct scene_node *child;
    wl_list_for_each(child, &scene_tree->children, link) {
        if (child->has_effect && (target->options & SCENE_RENDER_DISABLE_SUBEFFECT_NODE)) {
            continue;
        }

        child->impl.render(child, lx + child->x, ly + child->y, target);
    }
}

static void tree_get_affected_bounding_box(struct scene_node *node, uint32_t type,
                                           struct kywc_box *box)
{
    struct scene_tree *tree = scene_tree_from_node(node);
    if (wl_list_empty(&tree->children)) {
        *box = (struct kywc_box){ 0 };
        return;
    }

    int min_x1 = INT_MAX, min_y1 = INT_MAX;
    int max_x2 = INT_MIN, max_y2 = INT_MIN;

    struct kywc_box child_box;
    struct scene_node *child;
    wl_list_for_each(child, &tree->children, link) {
        if (!scene_node_is_visible(child)) {
            continue;
        }

        child->impl.get_affected_bounding_box(child, type, &child_box);
        if (kywc_box_empty(&child_box)) {
            continue;
        }

        child_box.x += child->x;
        child_box.y += child->y;

        min_x1 = MIN(min_x1, child_box.x);
        min_y1 = MIN(min_y1, child_box.y);
        max_x2 = MAX(max_x2, child_box.x + child_box.width);
        max_y2 = MAX(max_y2, child_box.y + child_box.height);
    }

    box->x = min_x1 == INT_MAX ? 0 : min_x1;
    box->y = min_y1 == INT_MAX ? 0 : min_y1;
    box->width = max_x2 == INT_MIN ? 0 : max_x2 - min_x1;
    box->height = max_y2 == INT_MIN ? 0 : max_y2 - min_y1;
}

static void tree_get_bounding_box(struct scene_node *node, struct wlr_box *box)
{
    struct kywc_box bbox;
    tree_get_affected_bounding_box(node, SCENE_BOUNDING_WITHOUT_EFFECT, &bbox);
    *box = (struct wlr_box){ bbox.x, bbox.y, bbox.width, bbox.height };
}

static void tree_destroy(struct scene_node *node)
{
    struct scene_tree *tree = scene_tree_from_node(node);

    struct scene_node *child, *child_tmp;
    wl_list_for_each_safe(child, child_tmp, &tree->children, link) {
        scene_node_destroy(child);
    }

    /* just call node_destroy, we not save destroy_func in scene_tree */
    node_destroy(node);
}

static void scene_tree_init(struct scene_tree *tree, struct scene_tree *parent)
{
    *tree = (struct scene_tree){ 0 };
    scene_node_init(&tree->node, parent);

    tree->node.type = SCENE_NODE_TREE;

    /* override node interface */
    tree->node.impl.accept_input = tree_accept_input;
    tree->node.impl.update_outputs = tree_update_outputs;
    tree->node.impl.collect_damage = tree_collect_damage;
    tree->node.impl.get_opaque_region = tree_get_opaque_region;
    tree->node.impl.render = tree_render;
    tree->node.impl.get_bounding_box = tree_get_bounding_box;
    tree->node.impl.get_affected_bounding_box = tree_get_affected_bounding_box;
    tree->node.impl.destroy = tree_destroy;

    wl_list_init(&tree->children);
}

/**
 * scene root based on tree
 */

static void scene_damage_outputs(struct scene *scene, const pixman_region32_t *damage)
{
    struct kywc_box geo;
    struct scene_output *scene_output;
    wl_list_for_each(scene_output, &scene->outputs, link) {
        geo = !kywc_box_empty(&scene_output->viewport.src) ? scene_output->viewport.src
                                                           : scene_output->geometry;
        if (pixman_region32_contains_rectangle(
                damage, &(pixman_box32_t){ geo.x, geo.y, geo.x + geo.width, geo.y + geo.height }) !=
            PIXMAN_REGION_OUT) {
            output_schedule_frame(scene_output->output);
        }
    }
}

static void scene_push_damage(struct scene_node *node, struct scene_node *damage_node,
                              uint32_t damage_type, pixman_region32_t *damage)
{
    if (!node->enabled) {
        return;
    }

    assert(damage != NULL);
    pixman_region32_translate(damage, node->x, node->y);

    struct scene *scene = scene_from_node(node);
    damage_type |= node->damage_type;

    assert(damage_type != SCENE_DAMAGE_NONE);
    if (damage_type == SCENE_DAMAGE_HARMLESS && damage_node->last_enabled) {
        assert(damage_node->type == SCENE_NODE_RECT || damage_node->type == SCENE_NODE_BUFFER);
        pixman_region32_intersect(damage, damage, &damage_node->visible_region);
        scene_add_damage(scene, damage);
        return;
    }

    scene_damage_outputs(scene, damage);
    pixman_region32_union(&scene->pushed_damage, &scene->pushed_damage, damage);
}

static void scene_destroy(struct scene_node *node)
{
    struct scene *scene = scene_from_node(node);
    wl_list_remove(&scene->presentation_destroy.link);

    struct scene_output *output, *tmp;
    wl_list_for_each_safe(output, tmp, &scene->outputs, link) {
        scene_output_destroy(output);
    }

    pixman_region32_fini(&scene->collected_damage);
    pixman_region32_fini(&scene->collected_invisible);
    pixman_region32_fini(&scene->pushed_damage);

    scene->tree_destroy(node);
}

struct scene *scene_create(void)
{
    struct scene *scene = calloc(1, sizeof(*scene));
    if (!scene) {
        return NULL;
    }

    scene_tree_init(&scene->tree, NULL);
    scene_node_set_role(&scene->tree.node, SCENE_ROLE_ROOT, NULL, scene);

    scene->tree_destroy = scene->tree.node.impl.destroy;
    scene->tree.node.impl.destroy = scene_destroy;
    scene->tree.node.impl.push_damage = scene_push_damage;

    wl_list_init(&scene->outputs);
    wl_list_init(&scene->presentation_destroy.link);

    pixman_region32_init(&scene->collected_damage);
    pixman_region32_init(&scene->collected_invisible);
    pixman_region32_init(&scene->pushed_damage);

    return scene;
}

struct scene *scene_from_node(struct scene_node *node)
{
    struct scene_tree *tree = node->parent;
    if (!tree) {
        tree = scene_tree_from_node(node);
    } else {
        while (tree->node.parent != NULL) {
            tree = tree->node.parent;
        }
    }
    struct scene *scene = wl_container_of(tree, scene, tree);
    return scene;
}

void scene_damage_whole(struct scene *scene)
{
    struct scene_output *output;
    wl_list_for_each(output, &scene->outputs, link) {
        scene_output_damage_whole(output);
    }
}

struct scene_tree *scene_tree_create(struct scene_tree *parent)
{
    assert(parent);
    struct scene_tree *tree = calloc(1, sizeof(*tree));
    if (!tree) {
        return NULL;
    }

    scene_tree_init(tree, parent);
    return tree;
}

static void scene_handle_presentation_destroy(struct wl_listener *listener, void *data)
{
    struct scene *scene = wl_container_of(listener, scene, presentation_destroy);
    wl_list_remove(&scene->presentation_destroy.link);
    wl_list_init(&scene->presentation_destroy.link);
    scene->presentation = NULL;
}

void scene_set_presentation(struct scene *scene, struct wlr_presentation *presentation)
{
    assert(scene->presentation == NULL);
    scene->presentation = presentation;
    scene->presentation_destroy.notify = scene_handle_presentation_destroy;
    wl_signal_add(&presentation->events.destroy, &scene->presentation_destroy);
}

static void scene_handle_linux_dmabuf_v1_destroy(struct wl_listener *listener, void *data)
{
    struct scene *scene = wl_container_of(listener, scene, linux_dmabuf_v1_destroy);
    wl_list_remove(&scene->linux_dmabuf_v1_destroy.link);
    wl_list_init(&scene->linux_dmabuf_v1_destroy.link);
    scene->linux_dmabuf_v1 = NULL;
}

void scene_set_linux_dmabuf_v1(struct scene *scene, struct wlr_linux_dmabuf_v1 *linux_dmabuf_v1)
{
    assert(scene->linux_dmabuf_v1 == NULL);
    scene->linux_dmabuf_v1 = linux_dmabuf_v1;
    scene->linux_dmabuf_v1_destroy.notify = scene_handle_linux_dmabuf_v1_destroy;
    wl_signal_add(&linux_dmabuf_v1->events.destroy, &scene->linux_dmabuf_v1_destroy);
}

static void handle_tearing_control_v1_destroy(struct wl_listener *listener, void *data)
{
    struct scene *scene = wl_container_of(listener, scene, tearing_control_v1_destroy);
    wl_list_remove(&scene->tearing_control_v1_destroy.link);
    scene->tearing_control_v1 = NULL;
}

void scene_set_tearing_control_v1(struct scene *scene,
                                  struct wlr_tearing_control_manager_v1 *tearing_control_v1)
{
    assert(scene->tearing_control_v1 == NULL);
    scene->tearing_control_v1 = tearing_control_v1;
    scene->tearing_control_v1_destroy.notify = handle_tearing_control_v1_destroy;
    wl_signal_add(&tearing_control_v1->events.destroy, &scene->tearing_control_v1_destroy);
}

bool scene_surface_is_tearing_allowed(struct scene *scene, struct wlr_surface *surface)
{
    if (!scene->tearing_control_v1 || !surface) {
        return false;
    }

    enum wp_tearing_control_v1_presentation_hint hint =
        wlr_tearing_control_manager_v1_surface_hint_from_surface(scene->tearing_control_v1,
                                                                 surface);
    kywc_log(KYWC_DEBUG, "Get surface tearing hint: %d", hint);
    return hint == WP_TEARING_CONTROL_V1_PRESENTATION_HINT_ASYNC;
}

bool scene_is_tearing_needed(struct scene *scene)
{
    if (!scene->tearing_control_v1) {
        return false;
    }

    struct wlr_tearing_control_v1 *hint;
    wl_list_for_each(hint, &scene->tearing_control_v1->surface_hints, link) {
        if (hint->hint == WP_TEARING_CONTROL_V1_PRESENTATION_HINT_ASYNC) {
            return true;
        }
    }

    return false;
}

void scene_collect_damage(struct scene *scene)
{
    PROFILE_ZONE(zone, __func__);

    /* skip collect damage if no pushed_damage */
    if (pixman_region32_not_empty(&scene->pushed_damage)) {
        // invisible region will be recalculated
        pixman_region32_clear(&scene->collected_invisible);
        // get current damage in all scene nodes
        struct scene_node *root = &scene->tree.node;
        root->impl.collect_damage(root, root->x, root->y, true, false, &scene->collected_damage,
                                  &scene->collected_invisible, &scene->pushed_damage);

        pixman_region32_clear(&scene->pushed_damage);
    }

    if (!pixman_region32_not_empty(&scene->collected_damage)) {
        PROFILE_ZONE_END(zone);
        return;
    }

    /* distribute damage to outputs */
    struct kywc_box box;
    pixman_region32_t region;
    struct scene_output *output;
    wl_list_for_each(output, &scene->outputs, link) {
        box = !kywc_box_empty(&output->viewport.src) ? output->viewport.src : output->geometry;
        pixman_region32_init_rect(&region, box.x, box.y, box.width, box.height);
        pixman_region32_intersect(&region, &region, &scene->collected_damage);
        pixman_region32_union(&output->collected_damage, &output->collected_damage, &region);
        pixman_region32_fini(&region);
    }

    pixman_region32_clear(&scene->collected_damage);

    PROFILE_ZONE_END(zone);
}

void scene_add_damage(struct scene *scene, const pixman_region32_t *damage)
{
    if (!pixman_region32_not_empty(damage)) {
        return;
    }

    pixman_region32_union(&scene->collected_damage, &scene->collected_damage, damage);

    scene_damage_outputs(scene, damage);
}

/**
 * scene node operations
 */

void scene_node_destroy(struct scene_node *node)
{
    if (!node) {
        return;
    }
    /**
     * Destroy node must emit signal first, the destroy_node doesn't emit destroy signal.
     */
    wl_signal_emit_mutable(&node->events.destroy, NULL);
    node->impl.destroy(node);
}

void scene_node_set_enabled(struct scene_node *node, bool enabled)
{
    if (node->enabled == enabled) {
        return;
    }

    scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, NULL);
    node->enabled = enabled;
    scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, NULL);
}

void scene_node_set_input_bypassed(struct scene_node *node, bool bypassed)
{
    node->input_bypassed = bypassed;
}

void scene_node_force_damage_event(struct scene_node *node, bool force)
{
    /**
     * damage event for this node is may still not emitted,
     * early returned in scene_node_push_damage by node->enabled
     */
    node->force_damage_event = force;
}

void scene_node_set_position(struct scene_node *node, int x, int y)
{
    if (node->x == x && node->y == y) {
        return;
    }

    scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, NULL);
    node->x = x;
    node->y = y;
    scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, NULL);

    scene_node_update_outputs(node, NULL, NULL, NULL);
}

void scene_node_place_above(struct scene_node *node, struct scene_node *sibling)
{
    assert(node != sibling);
    assert(node->parent == sibling->parent);
    if (node->link.prev == &sibling->link) {
        return;
    }

    scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, NULL);
    wl_list_remove(&node->link);
    wl_list_insert(&sibling->link, &node->link);
    scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, NULL);
}

void scene_node_place_below(struct scene_node *node, struct scene_node *sibling)
{
    assert(node != sibling);
    assert(node->parent == sibling->parent);
    if (node->link.next == &sibling->link) {
        return;
    }

    scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, NULL);
    wl_list_remove(&node->link);
    wl_list_insert(sibling->link.prev, &node->link);
    scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, NULL);
}

void scene_node_raise_to_top(struct scene_node *node)
{
    struct scene_node *current_top =
        wl_container_of(node->parent->children.prev, current_top, link);
    if (node == current_top) {
        return;
    }

    scene_node_place_above(node, current_top);
}

void scene_node_lower_to_bottom(struct scene_node *node)
{
    struct scene_node *current_bottom =
        wl_container_of(node->parent->children.next, current_bottom, link);
    if (node == current_bottom) {
        return;
    }

    scene_node_place_below(node, current_bottom);
}

void scene_node_reparent(struct scene_node *node, struct scene_tree *new_parent)
{
    assert(new_parent != NULL);
    if (node->parent == new_parent) {
        return;
    }

    /* Ensure that a node cannot become its own ancestor. */
    for (struct scene_tree *ancestor = new_parent; ancestor != NULL;
         ancestor = ancestor->node.parent) {
        assert(&ancestor->node != node);
    }

    scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, NULL);
    wl_list_remove(&node->link);
    node->parent = new_parent;
    wl_list_insert(new_parent->children.prev, &node->link);
    scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, NULL);

    scene_node_update_outputs(node, NULL, NULL, NULL);
}

bool scene_node_coords(struct scene_node *node, int *lx_ptr, int *ly_ptr)
{
    assert(node);

    *lx_ptr = 0, *ly_ptr = 0;
    bool enabled = true;

    while (true) {
        enabled = enabled && node->enabled;
        *lx_ptr += node->x;
        *ly_ptr += node->y;
        if (!node->parent) {
            break;
        }
        node = &node->parent->node;
    }

    return enabled;
}

bool scene_node_is_enabled(struct scene_node *node)
{
    assert(node);

    while (node) {
        if (!node->enabled) {
            return false;
        }
        node = node->parent ? &node->parent->node : NULL;
    }

    return true;
}

struct scene_node *scene_node_at(struct scene_node *node, double lx, double ly, double *nx,
                                 double *ny)
{
    struct scene_node *found;
    double sx, sy;
    int x, y;

    scene_node_coords(node, &x, &y);
    found = node->impl.accept_input(node, x, y, lx, ly, &sx, &sy);
    if (!found) {
        return NULL;
    }

    if (nx) {
        *nx = sx;
    }
    if (ny) {
        *ny = sy;
    }
    return found;
}

void scene_node_set_input_region(struct scene_node *node, const pixman_region32_t *region)
{
    /* tree is not support currently */
    assert(node->type == SCENE_NODE_RECT || node->type == SCENE_NODE_BUFFER);
    if (pixman_region32_equal(&node->input_region, region)) {
        return;
    }
    pixman_region32_copy(&node->input_region, region);
}

void scene_node_set_clip_region(struct scene_node *node, const pixman_region32_t *region)
{
    /* tree is not support currently */
    assert(node->type == SCENE_NODE_RECT || node->type == SCENE_NODE_BUFFER);
    if (pixman_region32_equal(&node->clip_region, region)) {
        return;
    }
    pixman_region32_copy(&node->clip_region, region);
    scene_node_push_damage(node, SCENE_DAMAGE_BOTH, NULL);
}

void scene_node_set_blur_region(struct scene_node *node, const pixman_region32_t *region)
{
    /* tree is not support currently */
    assert(node->type == SCENE_NODE_RECT || node->type == SCENE_NODE_BUFFER);
    bool has_blur = region != NULL;
    /* early return if still has no blur */
    if (!has_blur && has_blur == node->has_blur) {
        return;
    }

    /* add blur or remove blur */
    if (node->has_blur != has_blur) {
        node->has_blur = has_blur;
        if (has_blur) {
            pixman_region32_copy(&node->blur.region, region);
        } else {
            pixman_region32_clear(&node->blur.region);
        }
    } else { /* change blur region */
        if (pixman_region32_equal(&node->blur.region, region)) {
            return;
        }
        pixman_region32_copy(&node->blur.region, region);
    }

    scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, NULL);
}

void scene_node_set_blur_level(struct scene_node *node, uint32_t iterations, float offset)
{
    /* tree is not support currently */
    assert(node->type == SCENE_NODE_RECT || node->type == SCENE_NODE_BUFFER);
    if (node->blur.iterations == iterations && node->blur.offset == offset) {
        return;
    }

    node->blur.iterations = iterations;
    node->blur.offset = offset;
    if (node->has_blur) {
        pixman_region32_t *region =
            pixman_region32_not_empty(&node->blur.region) ? &node->blur.region : NULL;
        scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, region);
    }
}

void scene_node_set_blur_opacity(struct scene_node *node, float opacity)
{
    /* tree is not support currently */
    assert(node->type == SCENE_NODE_RECT || node->type == SCENE_NODE_BUFFER);
    if (DOUBLE_EQUAL(node->blur.alpha, opacity)) {
        return;
    }

    node->blur.alpha = pow(opacity, 0.4);
    if (node->has_blur) {
        pixman_region32_t *region =
            pixman_region32_not_empty(&node->blur.region) ? &node->blur.region : NULL;
        scene_node_push_damage(node, SCENE_DAMAGE_HARMFUL, region);
    }
}

static void scene_node_get_blur_region(struct scene_node *node, pixman_region32_t *region)
{
    /* tree is not support currently */
    assert(node->type == SCENE_NODE_RECT || node->type == SCENE_NODE_BUFFER);
    if (!node->has_blur) {
        pixman_region32_clear(region);
        return;
    }

    if (pixman_region32_not_empty(&node->blur.region)) {
        pixman_region32_copy(region, &node->blur.region);
    } else {
        struct wlr_box box;
        node->impl.get_bounding_box(node, &box);
        pixman_region32_fini(region);
        pixman_region32_init_rect(region, box.x, box.y, box.width, box.height);
        if (pixman_region32_not_empty(&node->clip_region)) {
            pixman_region32_intersect(region, region, &node->clip_region);
        }
    }
}

/* the x、y value is the offset relative to the first node */
static void node_for_each_enabled(struct scene_node *node, int x, int y,
                                  void (*callback)(struct scene_node *, int, int, void *),
                                  void *data)
{
    if (!node->enabled) {
        return;
    }

    x += node->x;
    y += node->y;
    callback(node, x, y, data);

    if (node->type != SCENE_NODE_TREE) {
        return;
    }

    struct scene_tree *tree = scene_tree_from_node(node);
    struct scene_node *pos;
    wl_list_for_each(pos, &tree->children, link) {
        node_for_each_enabled(pos, x, y, callback, data);
    }
}

static void get_node_blur_info(struct scene_node *node, int x, int y, void *data)
{
    /* tree is not support currently */
    if (!node->has_blur || (node->type != SCENE_NODE_RECT && node->type != SCENE_NODE_BUFFER)) {
        return;
    }

    pixman_region32_t blur_region;
    pixman_region32_init(&blur_region);
    scene_node_get_blur_region(node, &blur_region);
    pixman_region32_translate(&blur_region, x, y);

    struct blur_info *info = data;
    pixman_region32_union(&info->region, &info->region, &blur_region);
    pixman_region32_fini(&blur_region);

    info->iterations =
        info->iterations < node->blur.iterations ? node->blur.iterations : info->iterations;
    info->offset = info->offset < node->blur.offset ? node->blur.offset : info->offset;
    /* use the minimum alpha */
    info->alpha = info->alpha < node->blur.alpha ? info->alpha : node->blur.alpha;
}

void scene_node_get_blur_info(struct scene_node *node, struct blur_info *info)
{
    info->alpha = 1;
    info->offset = 0;
    info->iterations = 0;
    pixman_region32_clear(&info->region);
    if (node->type != SCENE_NODE_TREE) {
        get_node_blur_info(node, 0, 0, info);
        return;
    }

    struct scene_node *pos;
    struct scene_tree *tree = scene_tree_from_node(node);
    wl_list_for_each(pos, &tree->children, link) {
        node_for_each_enabled(pos, 0, 0, get_node_blur_info, info);
    }
}

void scene_node_set_radius(struct scene_node *node, const int radius[static 4])
{
    /* tree is not support currently */
    assert(node->type == SCENE_NODE_RECT || node->type == SCENE_NODE_BUFFER);
    if (memcmp(node->radius, radius, sizeof(node->radius)) == 0) {
        return;
    }

    memcpy(node->radius, radius, sizeof(node->radius));
    scene_node_push_damage(node, SCENE_DAMAGE_BOTH, NULL);
}

struct round_corner {
    int x, y;
    int radius;
};

static void get_node_radius(struct scene_node *node, int x, int y, void *data)
{
    if (node->type != SCENE_NODE_RECT && node->type != SCENE_NODE_BUFFER) {
        return;
    }

    int x1 = x, y1 = y;
    int x2 = x1, y2 = y1;

    if (node->type == SCENE_NODE_RECT) {
        struct scene_rect *rect = scene_rect_from_node(node);
        x2 += rect->width;
        y2 += rect->height;
    } else if (node->type == SCENE_NODE_BUFFER) {
        struct scene_buffer *buffer = scene_buffer_from_node(node);
        x2 += buffer->dst_width;
        y2 += buffer->dst_height;
    }

    struct round_corner *corner = data;
    int index = SCENE_ROUND_CORNER_RB;
    if (node->radius[index] > 0 && (corner[index].x < x2 || corner[index].y < y2)) {
        corner[index].x = x2;
        corner[index].y = y2;
        corner[index].radius = node->radius[index];
    }
    index = SCENE_ROUND_CORNER_RT;
    if (node->radius[index] > 0 && (corner[index].x < x2 || corner[index].y > y1)) {
        corner[index].x = x2;
        corner[index].y = y1;
        corner[index].radius = node->radius[index];
    }
    index = SCENE_ROUND_CORNER_LB;
    if (node->radius[index] > 0 && (corner[index].x > x1 || corner[index].y < y2)) {
        corner[index].x = x1;
        corner[index].y = y2;
        corner[index].radius = node->radius[index];
    }
    index = SCENE_ROUND_CORNER_LT;
    if (node->radius[index] > 0 && (corner[index].x > x1 || corner[index].y > y1)) {
        corner[index].x = x1;
        corner[index].y = y1;
        corner[index].radius = node->radius[index];
    }
}

void scene_node_get_radius(struct scene_node *node, enum scene_round_corners_type type,
                           int radius[static 4])
{
    if (node->type == SCENE_NODE_RECT || node->type == SCENE_NODE_BUFFER) {
        memcpy(radius, node->radius, sizeof(node->radius));
        return;
    }

    /* 0 = right-bottom, 1 = right-top, 2 = left-bottom, 3 = left-top */
    struct round_corner corner[4] = {
        { .x = INT_MIN, .y = INT_MIN },
        { .x = INT_MIN, .y = INT_MAX },
        { .x = INT_MAX, .y = INT_MIN },
        { .x = INT_MAX, .y = INT_MAX },
    };
    node_for_each_enabled(node, -node->x, -node->y, get_node_radius, &corner);

    if (type == SCENE_ROUND_CORNERS_FOR_OUTERMOST) {
        for (int i = 0; i < 4; ++i) {
            radius[i] = corner[i].radius;
        }
        return;
    }

    struct wlr_box box = { 0 };
    node->impl.get_bounding_box(node, &box);

    int x1 = box.x, y1 = box.y;
    int x2 = box.x + box.width, y2 = box.y + box.height;

    struct point {
        int x, y;
    } points[4] = { { x2, y2 }, { x2, y1 }, { x1, y2 }, { x1, y1 } };

    for (int i = 0; i < 4; ++i) {
        if (corner[i].radius > 0 && corner[i].x == points[i].x && corner[i].y == points[i].y) {
            radius[i] = corner[i].radius;
        }
    }
}

bool scene_node_is_visible(struct scene_node *node)
{
    if (!node->enabled) {
        return false;
    }

    if (node->type == SCENE_NODE_TREE) {
        struct scene_tree *tree = scene_tree_from_node(node);
        return !wl_list_empty(&tree->children);
    } else if (node->type == SCENE_NODE_RECT) {
        struct scene_rect *rect = scene_rect_from_node(node);
        return rect->color[3] != 0;
    } else if (node->type == SCENE_NODE_BUFFER) {
        struct scene_buffer *buffer = scene_buffer_from_node(node);
        return buffer->buffer && buffer->opacity != 0;
    }

    return false;
}

void scene_node_push_damage(struct scene_node *node, enum scene_damage_type damage_type,
                            const pixman_region32_t *damage)
{
    if (!node->enabled) {
        return;
    }

    pixman_region32_t damage_region;
    pixman_region32_init(&damage_region);

    if (damage == NULL) {
        struct wlr_box box = { 0 };
        node->impl.get_bounding_box(node, &box);
        pixman_region32_init_rect(&damage_region, box.x, box.y, box.width, box.height);
    } else {
        pixman_region32_copy(&damage_region, damage);
    }

    if (!pixman_region32_not_empty(&damage_region)) {
        pixman_region32_fini(&damage_region);
        return;
    }

    node->damage_type |= damage_type;
    node->impl.push_damage(node, node, 0, &damage_region);

    pixman_region32_fini(&damage_region);
}

void scene_corner_region(pixman_region32_t *region, int width, int height,
                         const int radius[static 4])
{
    int rb = radius[SCENE_ROUND_CORNER_RB];
    int rt = radius[SCENE_ROUND_CORNER_RT];
    int lb = radius[SCENE_ROUND_CORNER_LB];
    int lt = radius[SCENE_ROUND_CORNER_LT];

    if (rb > 0) {
        pixman_region32_union_rect(region, region, width - rb, height - rb, rb, rb);
    }
    if (rt > 0) {
        pixman_region32_union_rect(region, region, width - rt, 0, rt, rt);
    }
    if (lb > 0) {
        pixman_region32_union_rect(region, region, 0, height - lb, lb, lb);
    }
    if (lt > 0) {
        pixman_region32_union_rect(region, region, 0, 0, lt, lt);
    }

    /**
     * if enable rounded corners use the rounded corner shader.
     * shader has one-pixel feathered edge for anti-aliasing, this edge is non-opaque.
     */
    if (pixman_region32_not_empty(region)) {
        int anti_aliasing_width = 1;
        pixman_region32_t aa_edge;
        pixman_region32_init_rect(&aa_edge, 0, 0, width, height);
        pixman_region32_t inner;
        pixman_region32_init_rect(&inner, anti_aliasing_width, anti_aliasing_width,
                                  width - 2 * anti_aliasing_width,
                                  height - 2 * anti_aliasing_width);
        pixman_region32_subtract(&aa_edge, &aa_edge, &inner);
        pixman_region32_fini(&inner);
        pixman_region32_union(region, region, &aa_edge);
        pixman_region32_fini(&aa_edge);
    }
}

void scene_log_region(enum kywc_log_level level, const char *desc, const pixman_region32_t *region)
{
    int rects_len;
    const pixman_box32_t *rects = pixman_region32_rectangles(region, &rects_len);

    kywc_log(level, "%s region dump: %d nrects with extend (%d, %d) %d x %d", desc, rects_len,
             region->extents.x1, region->extents.y1, region->extents.x2 - region->extents.x1,
             region->extents.y2 - region->extents.y1);

    for (int i = 0; i < rects_len; i++) {
        const pixman_box32_t *rect = &rects[i];
        kywc_log(level, "\trect[%d]: (%d, %d) %d x %d", i, rect->x1, rect->y1, rect->x2 - rect->x1,
                 rect->y2 - rect->y1);
    }
}

void scene_log_box(enum kywc_log_level level, const char *desc, struct kywc_box *box)
{
    kywc_log(level, "%s box: (%d, %d) %d x %d", desc, box->x, box->y, box->width, box->height);
}

void scene_log_fbox(enum kywc_log_level level, const char *desc, struct kywc_fbox *box)
{
    kywc_log(level, "%s box: (%.3f, %.3f) %.3f x %.3f", desc, box->x, box->y, box->width,
             box->height);
}
