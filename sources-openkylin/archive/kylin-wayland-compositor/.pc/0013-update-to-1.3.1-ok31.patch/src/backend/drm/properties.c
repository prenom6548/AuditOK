// SPDX-FileCopyrightText: 2025 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "drm_p.h"

const struct drm_prop_info crtc_infos[DRM_CRTC_PROP_COUNT] = {
    [DRM_CRTC_PROP_ACTIVE] = { .name = "ACTIVE", },
    [DRM_CRTC_PROP_MODE_ID] = {.name = "MODE_ID",},
    [DRM_CRTC_PROP_OUT_FENCE_PTR] = { .name = "OUTPUT_FENCE_PTR", },
    [DRM_CRTC_PROP_VRR_ENABLE] = { .name = "VRR_ENABLED", },
    [DRM_CRTC_PROP_CTM] = {.name= "CTM",},
    [DRM_CRTC_PROP_GAMMA_LUT] = { .name = "GAMMA_LUT", },
    [DRM_CRTC_PROP_GAMMA_LUT_SIZE] = { .name = "GAMMA_LUT_SIZE", },
};

const struct drm_prop_info plane_infos[DRM_PLANE_PROP_COUNT] = {
    [DRM_PLANE_PROP_TYPE] = { .name = "type",},
    [DRM_PLANE_PROP_FB_ID] = { .name = "FB_ID",},
    [DRM_PLANE_PROP_CRTC_ID] = { .name = "CRTC_ID",},
    [DRM_PLANE_PROP_CRTC_X] = { .name = "CRTC_X", },
    [DRM_PLANE_PROP_CRTC_Y] = { .name = "CRTC_Y", },
    [DRM_PLANE_PROP_CRTC_W] = { .name = "CRTC_W", },
    [DRM_PLANE_PROP_CRTC_H] = { .name = "CRTC_H", },
    [DRM_PLANE_PROP_SRC_X] = { .name = "SRC_X", },
    [DRM_PLANE_PROP_SRC_Y] = { .name = "SRC_Y",},
    [DRM_PLANE_PROP_SRC_W] = { .name = "SRC_W", },
    [DRM_PLANE_PROP_SRC_H] = { .name = "SRC_H", },
    [DRM_PLANE_PROP_IN_FORMATS] = { .name = "IN_FORMATS",},
    [DRM_PLANE_PROP_FB_DAMAGE_CLIPS] = { .name = "FB_DAMAGE_CLIPS",},
    [DRM_PLANE_PROP_ROTATION] = { .name = "ROTATION",},
    [DRM_PLANE_PROP_HOTSPOT_X] = { .name = "HOTSPOT_X",},
    [DRM_PLANE_PROP_HOTSPOT_Y] = { .name = "HOTSPOT_Y",},
    [DRM_PLANE_PROP_COLOR_ENCODING] = { .name = "COLOR_ENCODING",},
    [DRM_PLANE_PROP_COLOR_RANGE] = { .name = "COLOR_RANGE",},
};

const struct drm_prop_info connector_infos[DRM_CONNECTOR_PROP_COUNT] = {
    [DRM_CONNECTOR_PROP_EDID] = { .name = "EDID",},
    [DRM_CONNECTOR_PROP_DPMS] = { .name = "DPMS",},
    [DRM_CONNECTOR_PROP_LINK_STATUS] = { .name = "link-status",},
    [DRM_CONNECTOR_PROP_NON_DESKTOP] = { .name = "non-desktop",},
    [DRM_CONNECTOR_PROP_CRTC_ID] = { .name = "CRTC_ID",},
    [DRM_CONNECTOR_PROP_CONTENT_TYPE] = { .name = "content type",},
    [DRM_CONNECTOR_PROP_MAX_BPC] = { .name = "max bpc",},
    [DRM_CONNECTOR_PROP_PANEL_ORIENTATION] = { .name = "panel orientation",},
    [DRM_CONNECTOR_PROP_SUBCONNECTOR] = { .name = "subconnector",},
    [DRM_CONNECTOR_PROP_VRR_CAPABLE] = { .name = "vrr_capable",},
    [DRM_CONNECTOR_PROP_BRIGHTNESS] = { .name = "brightness",},
    [DRM_CONNECTOR_PROP_RGB_RANGE] = { .name = "Broadcast RGB",},
    [DRM_CONNECTOR_PROP_OVERSCAN] = { .name = "overscan",},
    [DRM_CONNECTOR_PROP_SCALING_MODE] = { .name = "scaling mode",},
};

char *drm_get_property_enum_list(int fd, uint32_t type, uint32_t obj, struct drm_prop_info *prop)
{
    uint64_t value;
    if (!drm_get_property_value(fd, type, obj, prop, &value)) {
        return NULL;
    }

    drmModePropertyRes *drm_prop = drmModeGetProperty(fd, prop->id);
    if (!drm_prop) {
        return NULL;
    }

    char *str = NULL;
    size_t total_len = 0;

    for (int i = 0; i < drm_prop->count_enums; i++) {
        total_len += strlen(drm_prop->enums[i].name) + (i > 0 ? 2 : 0);
    }

    if (total_len > 0) {
        str = malloc(total_len + 1); // +1 for null terminator
        if (str) {
            char *ptr = str;
            for (int i = 0; i < drm_prop->count_enums; i++) {
                if (i > 0) {
                    *ptr++ = ',';
                    *ptr++ = ' ';
                }
                const char *name = drm_prop->enums[i].name;
                size_t len = strlen(name);
                memcpy(ptr, name, len);
                ptr += len;
            }
            *ptr = '\0';
        }
    }

    drmModeFreeProperty(drm_prop);

    return str;
}

char *drm_get_property_enum(int fd, uint32_t type, uint32_t obj, struct drm_prop_info *prop)

{
    uint64_t value;
    if (!drm_get_property_value(fd, type, obj, prop, &value)) {
        return NULL;
    }

    drmModePropertyRes *drm_prop = drmModeGetProperty(fd, prop->id);
    if (!drm_prop) {
        return NULL;
    }

    char *str = NULL;
    for (int i = 0; i < drm_prop->count_enums; i++) {
        if (drm_prop->enums[i].value == value) {
            str = strdup(drm_prop->enums[i].name);
            break;
        }
    }

    drmModeFreeProperty(drm_prop);

    return str;
}

void *drm_get_property_blob(int fd, uint32_t type, uint32_t obj, struct drm_prop_info *prop,
                            size_t *ret_len)
{
    uint64_t blob_id;
    if (!drm_get_property_value(fd, type, obj, prop, &blob_id)) {
        kywc_log(KYWC_WARN, "Failed to get Blob Id");
        return NULL;
    }

    drmModePropertyBlobRes *blob = drmModeGetPropertyBlob(fd, blob_id);
    if (!blob) {
        return NULL;
    }

    void *ptr = malloc(blob->length);
    if (!ptr) {
        drmModeFreePropertyBlob(blob);
        return NULL;
    }

    memcpy(ptr, blob->data, blob->length);
    *ret_len = blob->length;

    drmModeFreePropertyBlob(blob);

    return ptr;
}

bool drm_get_property_value(int fd, uint32_t type, uint32_t obj, struct drm_prop_info *prop,
                            uint64_t *ret)
{
    drmModeObjectProperties *drm_props = drmModeObjectGetProperties(fd, obj, type);
    if (!drm_props) {
        kywc_log_errno(KYWC_ERROR, "Failed to get DRM[%x] object[%d] properties", type, obj);
        return false;
    }

    bool found = false;
    for (uint32_t i = 0; i < drm_props->count_props; ++i) {
        if (drm_props->props[i] == prop->id) {
            *ret = drm_props->prop_values[i];
            found = true;
            break;
        }
    }

    drmModeFreeObjectProperties(drm_props);

    return found;
}

bool drm_get_property_range(int fd, uint32_t prop_id, uint64_t *min, uint64_t *max)
{
    drmModePropertyRes *prop = drmModeGetProperty(fd, prop_id);
    if (!prop) {
        return false;
    }

    if (drmModeGetPropertyType(prop) != DRM_MODE_PROP_RANGE) {
        drmModeFreeProperty(prop);
        return false;
    }

    assert(prop->count_values == 2);

    if (min != NULL) {
        *min = prop->values[0];
    }
    if (max != NULL) {
        *max = prop->values[1];
    }

    drmModeFreeProperty(prop);

    return true;
}

static bool scan_drm_properties(int fd, uint32_t obj, uint32_t type, struct drm_prop_info *out,
                                const struct drm_prop_info *info, uint32_t info_len)
{
    drmModeObjectProperties *drm_props = drmModeObjectGetProperties(fd, obj, type);
    if (!drm_props) {
        kywc_log(KYWC_ERROR, "Failed to get DRM [%s] properties",
                 type == DRM_MODE_OBJECT_CRTC    ? "CRTC"
                 : type == DRM_MODE_OBJECT_PLANE ? "plane"
                                                 : "connecotr");
        return false;
    }

    for (uint32_t i = 0; i < drm_props->count_props; ++i) {
        drmModePropertyRes *prop = drmModeGetProperty(fd, drm_props->props[i]);
        if (!prop) {
            kywc_log_errno(KYWC_ERROR, "Failed to get DRM property");
            continue;
        }
        for (uint32_t j = 0; j < info_len; ++j) {
            if (strcmp(prop->name, info[j].name)) {
                continue;
            }

            out[j].name = info[j].name;
            out[j].id = prop->prop_id;
            break;
        }

        drmModeFreeProperty(prop);
    }

    drmModeFreeObjectProperties(drm_props);

    return true;
}

bool drm_get_connector_props(int fd, uint32_t obj, struct drm_prop_info *out)
{
    return scan_drm_properties(fd, obj, DRM_MODE_OBJECT_CONNECTOR, out, connector_infos,
                               DRM_CONNECTOR_PROP_COUNT);
}

bool drm_get_plane_props(int fd, uint32_t obj, struct drm_prop_info *out)
{
    return scan_drm_properties(fd, obj, DRM_MODE_OBJECT_PLANE, out, plane_infos,
                               DRM_PLANE_PROP_COUNT);
}

bool drm_get_crtc_props(int fd, uint32_t obj, struct drm_prop_info *out)
{
    return scan_drm_properties(fd, obj, DRM_MODE_OBJECT_CRTC, out, crtc_infos, DRM_CRTC_PROP_COUNT);
}
