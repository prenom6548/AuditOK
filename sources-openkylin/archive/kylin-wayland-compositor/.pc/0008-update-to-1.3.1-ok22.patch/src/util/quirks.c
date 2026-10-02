// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <xf86drm.h>

#include "util/macros.h"
#include "util/quirks.h"

/* graphics quirks */
struct quirks {
    const char *name;   /* dc name */
    const char *vendor; /* gpu egl-vendor */
    uint32_t mask;
};

static const struct quirks quirks_table[] = {
    { "hisi", "ARM", QUIRKS_MASK_MASTER_FD | QUIRKS_MASK_EGL_WAYLAND },
    { "hisi-dpu", "HUAWEI", QUIRKS_MASK_MASTER_FD | QUIRKS_MASK_EGL_WAYLAND },
    { "ftd330", NULL, QUIRKS_MASK_EXPLICIT_SYNC | QUIRKS_MASK_YUYV_MODE },
    { "ftv310", "Phytium Technology Co., Ltd.", QUIRKS_MASK_EGL_WAYLAND },
    { "ftg340", "Phytium Technology Co., Ltd.",
      QUIRKS_MASK_EGL_WAYLAND | QUIRKS_MASK_PREFER_OPENGL },
    { "nvidia-drm", "NVIDIA",
      QUIRKS_MASK_SOFTWARE_CURSOR | QUIRKS_MASK_EXPLICIT_SYNC | QUIRKS_MASK_PREFER_OPENGL },
    { "nouveau", NULL, QUIRKS_MASK_SOFTWARE_CURSOR | QUIRKS_MASK_DISABLE_CTM },
    { "virtio_gpu", "Mesa Project", QUIRKS_MASK_SOFTWARE_CURSOR | QUIRKS_MASK_EXPLICIT_SYNC },
    { "vmwgfx", NULL, QUIRKS_MASK_SOFTWARE_CURSOR },
    { "mtgpu", "MTT Mesa Client",
      QUIRKS_MASK_NO_MODIFIERS | QUIRKS_MASK_DISABLE_DIRECT_SCANOUT | QUIRKS_MASK_EXPLICIT_SYNC },
    { "mwv207", "Mesa Project",
      QUIRKS_MASK_NO_MODIFIERS | QUIRKS_MASK_DISABLE_GAMMA | QUIRKS_MASK_DISABLE_CTM },
    { "ljm", "Ljmicro Corporation",
      QUIRKS_MASK_EGL_WAYLAND | QUIRKS_MASK_NO_MODIFIERS | QUIRKS_MASK_EXPLICIT_SYNC },
    { "arise", "Mesa Project", QUIRKS_MASK_DISABLE_CTM },
    { "linlondp", "ARM", QUIRKS_MASK_SOFTWARE_CURSOR | QUIRKS_MASK_EXPLICIT_SYNC },
    { "cx4", "Shanghai Zhaoxin Semiconductor Co. Ltd", QUIRKS_MASK_DISABLE_CTM },
    { "zx", "Shanghai Zhaoxin Semiconductor Co., Ltd", QUIRKS_MASK_DISABLE_CTM },
};

static bool quirks_by_env(uint32_t *mask)
{
    char *env = getenv("KYWC_FORCE_QUIRKS");
    if (!env) {
        return false;
    }

    *mask = atoi(env);

    return true;
}

uint32_t quirks_by_backend(int drm_fd)
{
    uint32_t mask;
    if (quirks_by_env(&mask)) {
        return mask;
    }

    drmVersion *version = drmGetVersion(drm_fd);
    if (!version) {
        return 0;
    }

    const struct quirks *quirks = NULL;
    for (size_t i = 0; i < ARRAY_SIZE(quirks_table); ++i) {
        if (!quirks_table[i].name || strcmp(quirks_table[i].name, version->name)) {
            continue;
        }
        quirks = &quirks_table[i];
        break;
    }

    drmFreeVersion(version);

    return quirks ? quirks->mask : 0;
}

uint32_t quirks_by_renderer(int drm_fd, const char *vendor_name)
{
    uint32_t mask;
    if (quirks_by_env(&mask)) {
        return mask;
    }

    drmVersion *version = drmGetVersion(drm_fd);
    if (!version) {
        return 0;
    }

    const struct quirks *quirks = NULL;
    for (size_t i = 0; i < ARRAY_SIZE(quirks_table); ++i) {
        if (!quirks_table[i].vendor || strcmp(vendor_name, quirks_table[i].vendor)) {
            continue;
        }

        if (!quirks_table[i].name || strcmp(quirks_table[i].name, version->name)) {
            continue;
        }

        quirks = &quirks_table[i];
        break;
    }

    drmFreeVersion(version);

    return quirks ? quirks->mask : 0;
}
