/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "SoundThemePlayerSetting.h"
#include "AudioContext.h"
#include "AudioMethod.h"
#include "SettingWhitebox.h"

namespace UkuiAudioFramwork
{
SoundThemePlayerSetting::SoundThemePlayerSetting()
{
    initSettings();
    initConnect();
}

SoundThemePlayerSetting::~SoundThemePlayerSetting()
{
    if (m_pSchema)
        g_settings_schema_unref(m_pSchema);

    if (m_pSource)
        releaseOwnedGSettingsSchemaSource(m_pSource);

    if (m_pSettings)
        g_object_unref(m_pSettings);
}

void SoundThemePlayerSetting::initSettings()
{
    m_pSource = g_settings_schema_source_get_default();
    m_pSchema = g_settings_schema_source_lookup(m_pSource, SOUND_THEME_PLAYER_SCHEMA, true);
    if (!m_pSchema) {
        AUDIO_ERROR("schema error, failed to create {} settings", SOUND_THEME_PLAYER_SCHEMA);
        return;
    }

    m_pSettings = g_settings_new_with_path(SOUND_THEME_PLAYER_SCHEMA, SOUND_THEME_PLAYER_PATH);
}

void SoundThemePlayerSetting::onSettingsChanged(GSettings* settings, const gchar* key)
{
    AudioMethod::getInstance().settingsChanged(key, g_settings_get_value(settings, key));
}

void SoundThemePlayerSetting::initConnect()
{
    g_signal_connect(G_OBJECT(m_pSettings),
                     "changed",
                     G_CALLBACK(onSettingsChanged),
                     nullptr);
}

GVariant* SoundThemePlayerSetting::getValue(const char* key) const
{
    if (!m_pSettings || !m_pSchema) return g_variant_new("");
    if (!isContainKeys(key)) return g_variant_new("");

    return g_settings_get_value(m_pSettings, key);
}

void SoundThemePlayerSetting::setValue(const char* key, GVariant* gvalue)
{
    if (!m_pSettings || !m_pSchema) return;
    if (!isContainKeys(key)) return;

    GVariantClass variantClass = g_variant_classify(gvalue);
    if (G_VARIANT_CLASS_STRING == variantClass) {
        gsize length;
        g_settings_set_string(m_pSettings, key, g_variant_get_string(gvalue, &length));
    }
    else if (G_VARIANT_CLASS_INT16 == variantClass) {
        g_settings_set_int(m_pSettings, key, g_variant_get_int16(gvalue));
    }
    else if (G_VARIANT_CLASS_UINT16 == variantClass) {
        g_settings_set_int(m_pSettings, key, g_variant_get_uint16(gvalue));
    }
    else if (G_VARIANT_CLASS_INT32 == variantClass) {
        g_settings_set_int(m_pSettings, key, g_variant_get_int32(gvalue));
    }
    else if (G_VARIANT_CLASS_UINT32 == variantClass) {
        g_settings_set_int(m_pSettings, key, g_variant_get_uint32(gvalue));
    }
    else if (G_VARIANT_CLASS_INT64 == variantClass) {
        g_settings_set_int(m_pSettings, key, g_variant_get_int64(gvalue));
    }
    else if (G_VARIANT_CLASS_UINT64 == variantClass) {
        g_settings_set_int(m_pSettings, key, g_variant_get_uint64(gvalue));
    }
    else if (G_VARIANT_CLASS_BOOLEAN == variantClass) {
        g_settings_set_boolean(m_pSettings, key, g_variant_get_boolean(gvalue));
    }
    else {
        AUDIO_DEBUG("Unknown variantClass");
    }
}

bool SoundThemePlayerSetting::isContainKeys(const char* key) const
{
    bool ret = false;
    if (!m_pSettings || !m_pSchema) return ret;

    gchar** keys = g_settings_schema_list_keys(m_pSchema);
    if (g_strv_contains(keys, key))
        ret = true;

    g_strfreev(keys);
    return ret;
}
}
