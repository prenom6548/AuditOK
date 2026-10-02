#ifndef SOUNDTHEMEPLAYERSETTING_H
#define SOUNDTHEMEPLAYERSETTING_H

#include <glib.h>
#include <gio/gio.h>
#include "ISetting.h"

namespace UkuiAudioFramwork
{
#define SOUND_THEME_PLAYER_SCHEMA  "org.ukui.media.sound"
#define SOUND_THEME_PLAYER_PATH    "/org/ukui/sound/keybindings/"
#define SOUND_FILENAME_KEY         "filename"
#define SOUND_NAME_KEY             "name"

class SoundThemePlayerSetting : public ISetting
{
public:
    SoundThemePlayerSetting();
    ~SoundThemePlayerSetting();

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
#endif // SOUNDTHEMEPLAYERSETTING_H
