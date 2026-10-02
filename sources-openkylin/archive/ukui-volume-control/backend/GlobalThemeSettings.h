#ifndef GLOBALTHEMESETTINGS_H
#define GLOBALTHEMESETTINGS_H

#include <glib.h>
#include <gio/gio.h>
#include "ISetting.h"

namespace UkuiAudioFramwork
{
#define GLOBAL_THEME_SCHEMA     "org.ukui.globaltheme.settings"
#define GLOBAL_THEME_PATH       "/org/ukui/globaltheme/settings/"
#define GLOBAL_THEME_NAME_KEY   "global-theme-name"

class GlobalThemeSettings : public ISetting
{
public:
    GlobalThemeSettings();
    ~GlobalThemeSettings();

public:
    void initConnect();

    virtual void initSettings() override;

    virtual GVariant* getValue(const char*) const override;

    virtual void setValue(const char*, GVariant*) override;

    virtual bool isContainKeys(const char*) const override;

private:
    static void onSettingsChanged(GSettings*, const gchar*);

private:
    GSettings* m_pSettings = nullptr;
    GSettingsSchema* m_pSchema = nullptr;
    GSettingsSchemaSource* m_pSource = nullptr;
};
}
#endif // GLOBALTHEMESETTINGS_H
