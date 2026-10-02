#ifndef SESSIONMANAGERSETTING_H
#define SESSIONMANAGERSETTING_H

#include <glib.h>
#include <gio/gio.h>
#include "ISetting.h"

namespace UkuiAudioFramwork
{
//  UKUI Session
//#define SESSION_MANAGER_SCHEMA  "org.ukui.session"
//#define SESSION_MANAGER_PATH    "/org/ukui/desktop/session/"
//#define STARTUP_MUSIC_KEY       "startup-music"
//#define POWEROFF_MUSIC_KEY      "poweroff-music"
//#define LOGOUT_MUSIC_KEY        "logout-music"
//#define WAKEUP_MUSIC_KEY        "weakup-music"

class SessionManagerSetting : public ISetting
{
public:
    SessionManagerSetting();
    ~SessionManagerSetting();

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
#endif // SESSIONMANAGERSETTING_H
