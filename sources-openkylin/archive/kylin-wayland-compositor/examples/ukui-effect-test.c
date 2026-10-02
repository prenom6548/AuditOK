// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>

#include "ukui-effect-v1-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"

#define MAX_SURFACES 10
static struct wl_shm *wl_shm = NULL;
static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct xdg_wm_base *wm_base = NULL;
static struct ukui_effect_v1 *ukui_effect = NULL;
static struct wl_surface *surfaces[MAX_SURFACES] = { 0 };
static struct xdg_surface *xdg_surfaces[MAX_SURFACES] = { 0 };
static struct xdg_toplevel *toplevels[MAX_SURFACES] = { 0 };
static struct animation_curve_v1 *animations[MAX_SURFACES] = { 0 };
static struct zxdg_decoration_manager_v1 *decoration_manager = NULL;
static int surface_count = 0;
static int animation_count = 0;

static void randname(char *buf)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long r = ts.tv_nsec;
    for (int i = 0; i < 6; ++i) {
        buf[i] = 'A' + (r & 15) + (r & 16) * 2;
        r >>= 5;
    }
}

static int create_shm_file(void)
{
    int retries = 100;
    do {
        char name[] = "/wl_shm-XXXXXX";
        randname(name + sizeof(name) - 7);
        --retries;
        int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
        if (fd >= 0) {
            shm_unlink(name);
            return fd;
        }
    } while (retries > 0 && errno == EEXIST);

    return -1;
}

static int allocate_shm_file(size_t size)
{
    int fd = create_shm_file();
    if (fd < 0) {
        return -1;
    }

    int ret;
    do {
        ret = ftruncate(fd, size);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0) {
        fprintf(stderr, "Failed to truncate shared memory file: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    /* sent by the compositor when it's no longer using this buffer */
    wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};

static struct wl_buffer *draw_frame(void)
{
    const int width = 640, height = 480;
    int stride = width * 4;
    int size = stride * height;

    int fd = allocate_shm_file(size);
    if (fd == -1) {
        return NULL;
    }

    uint32_t *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return NULL;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(wl_shm, fd, size);
    struct wl_buffer *buffer =
        wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    /* draw checkerboxed background */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            data[y * width + x] = 0xFF666666;
        }
    }

    munmap(data, size);
    wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);
    return buffer;
}

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
    printf("Attch xdg_surface: %p, data: %p\n", xdg_surface, data);
    struct wl_surface *wl_surface = xdg_surface_get_user_data(xdg_surface);
    struct wl_buffer *buffer = draw_frame();
    printf("Attch surface: %p\n", wl_surface);
    wl_surface_attach(wl_surface, buffer, 0, 0);
    wl_surface_commit(wl_surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static void handle_builtin_animation_curves(void *data, struct ukui_effect_v1 *manager,
                                            struct animation_curve_v1 *animation, uint32_t x1,
                                            uint32_t y1, uint32_t x2, uint32_t y2)
{
    printf("Builtin animation event received\n");
    printf("Manager=%p, animation=%p\n", manager, animation);

    if (animation_count < MAX_SURFACES) {
        animations[animation_count] = animation;
        printf("Storing animation at index %d (total %d)\n", animation_count, animation_count + 1);
        animation_count++;
    } else {
        printf("Warning: Animation buffer full (max %d), discarding animation\n", MAX_SURFACES);
    }

    printf("Current animation count: %d\n", animation_count);
}

static void handle_done(void *data, struct ukui_effect_v1 *manager)
{
    printf("All builtin animations received\n");
}

static const struct ukui_effect_v1_listener ukui_effect_listener = {
    .builtin_animation_curves = handle_builtin_animation_curves,
    .done = handle_done,
};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                                   const char *interface, uint32_t version)
{
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(wm_base, &xdg_wm_base_listener, NULL);
    } else if (strcmp(interface, ukui_effect_v1_interface.name) == 0) {
        ukui_effect = wl_registry_bind(registry, name, &ukui_effect_v1_interface, 1);
        printf("Adding manager listener...\n");
        printf("effect_manager=%p, manager_listener=%p\n", ukui_effect, &ukui_effect_listener);
        int ret = ukui_effect_v1_add_listener(ukui_effect, &ukui_effect_listener, NULL);
        printf("listener_add result=%d\n", ret);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        decoration_manager =
            wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    // Ignored
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void decoration_handle_configure(void *data, struct zxdg_toplevel_decoration_v1 *decoration,
                                        enum zxdg_toplevel_decoration_v1_mode mode)
{
    printf("Using %d\n", mode);
}

static const struct zxdg_toplevel_decoration_v1_listener decoration_listener = {
    .configure = decoration_handle_configure,
};

static void request_preferred_mode(struct zxdg_toplevel_decoration_v1 *decoration)
{
    enum zxdg_toplevel_decoration_v1_mode mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
    if (!mode) {
        printf("Requesting compositor preferred mode\n");
        zxdg_toplevel_decoration_v1_unset_mode(decoration);
        return;
    }

    printf("Requesting mode %d\n", mode);
    zxdg_toplevel_decoration_v1_set_mode(decoration, mode);
}

static bool get_int(int *index, int min, int max, const char *erro_info)
{
    bool ret = true;
    if (scanf("%d", index) != 1 || *index < min || *index > max) {
        printf("%s\n", erro_info);
        ret = false;
    }

    while (getchar() != '\n') {
    } // Clear input buffer

    return ret;
}

static struct animation_curve_v1 *slect_animation(void)
{
    int anim_idx;
    printf("Select animation (1-%d, 0 for none): ", animation_count);
    if (!get_int(&anim_idx, 1, animation_count, "Invalid animation selection")) {
        anim_idx = 0;
    }

    return anim_idx == 0 ? NULL : animations[anim_idx - 1];
}

static void test_fade_effect(int parent_idx, struct wl_surface *surface,
                             struct animation_curve_v1 *animation, bool is_center)
{
    struct fade_effect_v1 *fade_effect = ukui_effect_v1_create_fade_effect(ukui_effect, surface,
                                                                           0x1); // MAP stage
    if (!fade_effect) {
        printf("Failed to create fade effect\n");
        return;
    }

    // Set fade effect options
    uint32_t duration = 1000; // 1 second
    uint32_t alpha_start = 0; // fully transparent
    uint32_t alpha_end = 100; // fully opaque
    uint32_t size_start = 50; // 50% of surface size
    uint32_t size_end = 100;  // 100% of surface size

    fade_effect_v1_set_animation(fade_effect, animation, animation);
    fade_effect_v1_set_opacity(fade_effect, alpha_start);
    fade_effect_v1_set_duration(fade_effect, duration);
    if (is_center) {
        fade_effect_v1_set_scale(fade_effect, 0, size_start, size_start);
    } else {
        fade_effect_v1_set_scale(fade_effect, 1, 100, size_start);
    }

    printf("Created fade effect on surface #%d with parameters:\n"
           "  Duration: %dms\n"
           "  Alpha: %d%% -> %d%%\n"
           "  Size: %d%% -> %d%%\n",
           parent_idx, duration, alpha_start, alpha_end, size_start, size_end);
    // Commit to make the surface visible
    wl_surface_commit(surface);
    while (wl_display_dispatch(display)) {
    }
    fade_effect_v1_destroy(fade_effect);
}

static void test_slide_effect(int parent_idx, struct wl_surface *surface)
{
    struct slide_effect_v1 *slide_effect = ukui_effect_v1_create_slide_effect(ukui_effect, surface,
                                                                              0x1); // MAP stage
    if (!slide_effect) {
        printf("Failed to create slide effect\n");
        return;
    }

    // Set slide effect options
    uint32_t duration = 2000; // 2 second
    uint32_t alpha_start = 0; // fully transparent
    uint32_t alpha_end = 100; // fully opaque
    uint32_t location = 1;
    uint32_t offset = 10;
    slide_effect_v1_set_duration(slide_effect, duration);
    slide_effect_v1_set_position(slide_effect, location, offset);
    slide_effect_v1_set_opacity(slide_effect, alpha_start);

    printf("Created slide effect on surface #%d with parameters:\n"
           "  Duration: %dms\n"
           "  Alpha: %d%% -> %d%%\n"
           "  Pos location: %d, offset: %d\n",
           parent_idx, duration, alpha_start, alpha_end, location, offset);
    // Commit to make the surface visible
    wl_surface_commit(surface);
    while (wl_display_dispatch(display)) {
    }
    slide_effect_v1_destroy(slide_effect);
}

static void print_menu(void)
{
    printf("\nUKUI Effect Test Menu\n");
    printf("1. Create new surface\n");
    printf("2. Create fade effect on view\n");
    printf("3. Create slide effect on view\n");
    printf("4. Create animation curve\n");
    printf("5. Create custom animation curve\n");
    printf("6. Create fade effect on popup window\n");
    printf("7. Create slide effect on popup window\n");
    printf("8. Create top left fade effect on popup window\n");
    printf("9. Exit\n");
    printf("Choice: ");
}

int main(int argc, char **argv)
{
    display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        return 1;
    }

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_dispatch(display);
    wl_display_roundtrip(display);
    if (!ukui_effect) {
        fprintf(stderr, "ukui-effect protocol not supported\n");
        return 1;
    }

    printf("Initial events processed. Received %d animations\n", animation_count);
    printf("Entering main loop...\n");
    int running = 1;
    while (running) {
        print_menu();
        char choice[10];
        if (!fgets(choice, sizeof(choice), stdin)) {
            break;
        }

        switch (choice[0]) {
        case '1': {
            if (surface_count >= MAX_SURFACES) {
                printf("Maximum surfaces reached\n");
                break;
            }

            struct wl_surface *surface = wl_compositor_create_surface(compositor);
            if (!surface) {
                printf("Failed to create surface\n");
                break;
            }

            printf("Create surface: %p\n", surface);
            // Create xdg_surface for toplevel window
            struct xdg_surface *xdg_surface = xdg_wm_base_get_xdg_surface(wm_base, surface);
            if (!xdg_surface) {
                printf("Failed to create xdg_surface\n");
                wl_surface_destroy(surface);
                break;
            }
            xdg_surface_set_user_data(xdg_surface, surface);
            xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, surface);

            struct xdg_toplevel *toplevel = xdg_surface_get_toplevel(xdg_surface);
            if (!toplevel) {
                printf("Failed to create toplevel\n");
                xdg_surface_destroy(xdg_surface);
                wl_surface_destroy(surface);
                break;
            }

            xdg_toplevel_set_min_size(toplevel, 800, 600);
            xdg_toplevel_set_max_size(toplevel, 800, 600);

            struct zxdg_toplevel_decoration_v1 *decoration =
                zxdg_decoration_manager_v1_get_toplevel_decoration(decoration_manager, toplevel);
            request_preferred_mode(decoration);
            zxdg_toplevel_decoration_v1_add_listener(decoration, &decoration_listener, NULL);
            xdg_toplevel_set_title(toplevel, "ukui effect test client");

            // Store all created objects
            surfaces[surface_count] = surface;
            xdg_surfaces[surface_count] = xdg_surface;
            toplevels[surface_count] = toplevel;
            surface_count++;

            printf("Created surface #%d with xdg_surface and toplevel\n", surface_count);
            break;
        }
        case '2': {
            if (surface_count == 0) {
                printf("No surfaces available\n");
                break;
            }

            int idx;
            printf("Select surface (1-%d): ", surface_count);
            if (!get_int(&idx, 1, surface_count, "Invalid surface selection")) {
                break;
            }

            test_fade_effect(0, surfaces[idx - 1], slect_animation(), true);
            break;
        }
        case '3': {
            if (surface_count == 0) {
                printf("No surfaces available\n");
                break;
            }

            int idx;
            printf("Select surface (1-%d): ", surface_count);
            if (!get_int(&idx, 1, surface_count, "Invalid surface selection")) {
                break;
            }

            test_slide_effect(0, surfaces[idx - 1]);
            break;
        }
        case '4': {
            int curve_type;
            printf("Enter animation curve type (1=linear, 2=ease-in, 3=ease-out, 4=ease-in-out): ");
            if (!get_int(&curve_type, 1, 4, "Invalid curve type")) {
                break;
            }

            // 根据曲线类型设置贝塞尔曲线控制点
            uint32_t x1, y1, x2, y2;
            switch (curve_type) {
            case 1: // linear
                x1 = 0;
                y1 = 0;
                x2 = 100;
                y2 = 100;
                break;
            case 2: // ease-in
                x1 = 42;
                y1 = 0;
                x2 = 100;
                y2 = 100;
                break;
            case 3: // ease-out
                x1 = 0;
                y1 = 0;
                x2 = 58;
                y2 = 100;
                break;
            case 4: // ease-in-out
                x1 = 42;
                y1 = 0;
                x2 = 58;
                y2 = 100;
                break;
            default:
                printf("Invalid curve type\n");
                continue;
            }

            // 创建动画曲线对象
            struct animation_curve_v1 *animation =
                ukui_effect_v1_create_animation_curve(ukui_effect, x1, y1, x2, y2);
            if (!animation) {
                printf("Failed to create animation curve\n");
                break;
            }

            if (animation_count < MAX_SURFACES) {
                animations[animation_count++] = animation;
                printf("Created animation curve #%d (type %d: P1(%d,%d) P2(%d,%d))\n",
                       animation_count, curve_type, x1, y1, x2, y2);
            } else {
                printf("Animation buffer full, discarding animation\n");
                animation_curve_v1_destroy(animation);
            }
            break;
        }
        case '5': {
            int x1, y1, x2, y2;
            printf("Enter control point x1 (0-100): ");
            if (!get_int(&x1, 0, 100, "Invalid x1 value")) {
                break;
            }

            printf("Enter control point y1 (0-100): ");
            if (!get_int(&y1, 0, 100, "Invalid y1 value")) {
                break;
            }

            printf("Enter control point x2 (0-100): ");
            if (!get_int(&x2, 0, 100, "Invalid x2 value")) {
                break;
            }

            printf("Enter control point y2 (0-100): ");
            if (!get_int(&y2, 0, 100, "Invalid y2 value")) {
                break;
            }

            struct animation_curve_v1 *animation =
                ukui_effect_v1_create_animation_curve(ukui_effect, x1, y1, x2, y2);
            if (!animation) {
                printf("Failed to create custom animation curve\n");
                break;
            }

            if (animation_count < MAX_SURFACES) {
                animations[animation_count++] = animation;
                printf("Created custom animation curve #%d (P1(%d,%d) P2(%d,%d))\n",
                       animation_count, x1, y1, x2, y2);
            } else {
                printf("Animation buffer full, discarding animation\n");
                animation_curve_v1_destroy(animation);
            }
            break;
        }
        case '6':
        case '7':
        case '8': {
            if (surface_count == 0) {
                printf("No surfaces available\n");
                break;
            }

            if (!wm_base) {
                printf("xdg_wm_base not available\n");
                break;
            }

            int idx;
            printf("Select parent surface (1-%d): ", surface_count);
            if (!get_int(&idx, 1, surface_count, "Invalid surface selection")) {
                break;
            }
            // Use the selected surface directly
            wl_surface_commit(surfaces[idx - 1]);
            wl_display_dispatch(display);

            // Create popup surface
            struct wl_surface *popup_surface = wl_compositor_create_surface(compositor);
            if (!popup_surface) {
                printf("Failed to create popup surface\n");
                break;
            }

            // Create xdg surface for popup (attached to the selected surface)
            struct xdg_surface *popup_xdg_surface =
                xdg_wm_base_get_xdg_surface(wm_base, popup_surface);
            if (!popup_xdg_surface) {
                printf("Failed to create xdg surface for popup\n");
                wl_surface_destroy(popup_surface);
                break;
            }

            xdg_surface_set_user_data(popup_xdg_surface, popup_surface);
            xdg_surface_add_listener(popup_xdg_surface, &xdg_surface_listener, popup_surface);

            // Create positioner for popup
            struct xdg_positioner *positioner = xdg_wm_base_create_positioner(wm_base);
            if (!positioner) {
                printf("Failed to create positioner\n");
                xdg_surface_destroy(popup_xdg_surface);
                wl_surface_destroy(popup_surface);
                break;
            }

            // Configure positioner (100x100 popup at position 820,20)
            xdg_positioner_set_size(positioner, 100, 100);
            xdg_positioner_set_anchor_rect(positioner, 820, 20, 1, 1);
            xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
            xdg_positioner_set_constraint_adjustment(
                positioner, XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X |
                                XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y);
            // Create popup
            struct xdg_popup *popup =
                xdg_surface_get_popup(popup_xdg_surface, xdg_surfaces[idx - 1], positioner);
            xdg_positioner_destroy(positioner);
            if (!popup) {
                printf("Failed to create popup\n");
                xdg_surface_destroy(popup_xdg_surface);
                wl_surface_destroy(popup_surface);
                break;
            }

            printf("Created 100x100 popup window at position 20,20 on surface #%d\n", idx);
            if (choice[0] == '6') {
                test_fade_effect(idx, popup_surface, slect_animation(), true);
            } else if (choice[0] == '8') {
                test_fade_effect(idx, popup_surface, slect_animation(), false);
            } else {
                test_slide_effect(idx, popup_surface);
            }
            break;
        }
        case '9':
            running = 0;
            break;
        default:
            printf("Invalid choice\n");
        }
    }

    // Clean up
    for (int i = 0; i < surface_count; i++) {
        if (toplevels[i]) {
            xdg_toplevel_destroy(toplevels[i]);
        }
        if (xdg_surfaces[i]) {
            xdg_surface_destroy(xdg_surfaces[i]);
        }
        wl_surface_destroy(surfaces[i]);
    }
    for (int i = 0; i < animation_count; i++) {
        animation_curve_v1_destroy(animations[i]);
    }
    ukui_effect_v1_destroy(ukui_effect);
    wl_compositor_destroy(compositor);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);

    return 0;
}
