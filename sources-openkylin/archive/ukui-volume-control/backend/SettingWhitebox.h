#pragma once

#include <gio/gio.h>

namespace UkuiAudioFramwork {

inline void releaseOwnedGSettingsSchemaSource(GSettingsSchemaSource*& source)
{
    if (!source) {
        return;
    }
    if (source != g_settings_schema_source_get_default()) {
        g_settings_schema_source_unref(source);
    }
    source = nullptr;
}

} // namespace UkuiAudioFramwork
