// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include "backend/drm.h"
#include "output_p.h"

struct custom_mode {
    int32_t width, height;
    int32_t refresh; // in mHz
};

static const struct custom_mode custom_landscape_modes[] = {
    { 800, 600, 60000 },   { 1024, 768, 60000 },  { 1280, 800, 60000 },  { 1280, 1024, 60000 },
    { 1366, 768, 59093 },  { 1440, 900, 60000 },  { 1440, 960, 60000 },  { 1600, 1200, 60000 },
    { 1680, 1050, 60000 }, { 1680, 1120, 60000 }, { 1920, 1080, 60000 }, { 1920, 1280, 60000 },
    { 2160, 1440, 60000 },
};

static const struct custom_mode custom_portrait_modes[] = {
    { 600, 800, 60000 },   { 768, 1024, 60000 },  { 800, 1280, 60000 },  { 768, 1366, 59093 },
    { 900, 1440, 60000 },  { 960, 1440, 60000 },  { 1200, 1600, 60000 }, { 1050, 1680, 60000 },
    { 1120, 1680, 60000 }, { 1080, 1920, 60000 }, { 1280, 1920, 60000 }, { 1440, 2160, 60000 },
};

void output_add_custom_modes(struct output *output)
{
    struct wlr_output *wlr_output = output->wlr_output;
    if (!wlr_backend_is_drm(wlr_output->backend)) {
        return;
    }

    /* add more modes for eDP LVDS panel */
    if (strncmp(output->base.name, "eDP", 3) && strncmp(output->base.name, "LVDS", 4)) {
        return;
    }

    int width = 0, height = 0;

    struct wlr_output_mode *wlr_mode;
    /* only one same resolution continue to add custom modes */
    wl_list_for_each(wlr_mode, &wlr_output->modes, link) {
        if (width == 0 && height == 0) {
            width = wlr_mode->width;
            height = wlr_mode->height;
            continue;
        }
        if (width != wlr_mode->width && height != wlr_mode->height) {
            return;
        }
    }

    const struct custom_mode *modes;
    size_t mode_lens = 0;

    if (width > height) {
        modes = custom_landscape_modes;
        mode_lens = sizeof(custom_landscape_modes) / sizeof(custom_landscape_modes[0]);
    } else {
        modes = custom_portrait_modes;
        mode_lens = sizeof(custom_portrait_modes) / sizeof(custom_portrait_modes[0]);
    }

    for (size_t i = 0; i < mode_lens; ++i) {
        const struct custom_mode *mode = &modes[i];
        if (mode->width > width || mode->height > height ||
            (mode->width == width && mode->height == height)) {
            continue;
        }

        if (!wlr_output_add_mode(wlr_output, mode->width, mode->height, mode->refresh)) {
            kywc_log(KYWC_ERROR, "Output %s: add custom mode(%dx%d@%f) failed", output->base.name,
                     mode->width, mode->height, mode->refresh * 0.001);
        }
    }
}

void output_add_mode(struct output *output, struct wlr_output_mode *mode, bool check)
{
    struct kywc_output_prop *prop = &output->base.prop;

    if (check) {
        struct kywc_output_mode *_mode;
        wl_list_for_each(_mode, &prop->modes, link) {
            if (_mode->width == mode->width && _mode->height == mode->height &&
                _mode->refresh == mode->refresh) {
                return;
            }
        }
    }

    struct kywc_output_mode *new = calloc(1, sizeof(*new));
    if (!new) {
        return;
    }

    new->width = mode->width;
    new->height = mode->height;
    new->refresh = mode->refresh;
    new->preferred = mode->preferred;
    wl_list_insert(&prop->modes, &new->link);
}
