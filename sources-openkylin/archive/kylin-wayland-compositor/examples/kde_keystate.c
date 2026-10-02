// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>

#include "kde-keystate-client-protocol.h"

static struct org_kde_kwin_keystate *keystate_manager = NULL;

static char *key_name[] = { "capslock", "numlock", "scrolllock", "alt",
                            "ctrl",     "shift",   "meta",       "right_alt" };
static char *key_state[] = { "unlock", "latched", "locked", "pressed" };

static void handle_state_changed(void *data, struct org_kde_kwin_keystate *keystate, uint32_t key,
                                 uint32_t state)
{
    if (key <= ORG_KDE_KWIN_KEYSTATE_KEY_ALTGR && state <= ORG_KDE_KWIN_KEYSTATE_STATE_PRESSED) {
        printf("Keystate updated - key: %s, %s\n", key_name[key], key_state[state]);
    } else {
        printf("Keystate updated - Unknow state\n");
    }
}

static const struct org_kde_kwin_keystate_listener keystate_listener = {
    .stateChanged = handle_state_changed,
};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                                   const char *interface, uint32_t version)
{
    if (strcmp(interface, org_kde_kwin_keystate_interface.name) == 0) {
        keystate_manager =
            wl_registry_bind(registry, name, &org_kde_kwin_keystate_interface, version);
    }
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = NULL,
};

int main(int argc, char **argv)
{
    struct wl_display *display = wl_display_connect(NULL);
    if (display == NULL) {
        printf("wl_display_connect failed\n");
        return -1;
    }

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(display);

    if (!keystate_manager) {
        printf("wl_registry_bind failed\n");
        wl_display_disconnect(display);
        return -1;
    }

    org_kde_kwin_keystate_fetchStates(keystate_manager);
    org_kde_kwin_keystate_add_listener(keystate_manager, &keystate_listener, NULL);

    while (1) {
        if (wl_display_dispatch(display) == -1) {
            break;
        }
    }

    org_kde_kwin_keystate_destroy(keystate_manager);
    wl_display_disconnect(display);

    return 0;
}