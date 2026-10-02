#ifndef AUDIOSETTING_H
#define AUDIOSETTING_H

#include <glib.h>
#include <gio/gio.h>
#include "ISetting.h"

namespace UkuiAudioFramwork
{
class AudioSetting : public ISetting
{
public:
    AudioSetting();
    ~AudioSetting();

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
#endif // AUDIOSETTING_H
