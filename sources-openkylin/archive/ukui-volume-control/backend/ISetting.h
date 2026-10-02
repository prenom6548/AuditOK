#ifndef ISETTING_H
#define ISETTING_H

#include <gio/gio.h>

namespace UkuiAudioFramwork
{
enum class SettingType
{
    AUDIO_SETTING,
    GLOBALTHEME_SETTING,
    SESSIONMANAGER_SETTING,
    SOUDNTHEMEPLAYER_SETTING
};

class ISetting
{
public:
    ISetting() = default;
    virtual ~ISetting() = default;

public:
    virtual void initSettings() = 0;

    virtual GVariant* getValue(const char* key) const = 0;

    virtual void setValue(const char* key, GVariant* gvalue) = 0;

    virtual bool isContainKeys(const char* key) const = 0;
};
}
#endif // ISETTING_H
