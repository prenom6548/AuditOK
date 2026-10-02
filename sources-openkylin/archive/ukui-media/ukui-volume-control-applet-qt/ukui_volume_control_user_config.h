#ifndef UKUIVOLUMECONTROLUSERCONFIG_H
#define UKUIVOLUMECONTROLUSERCONFIG_H

#include <memory>
#include <unordered_map>
#include <QSettings>

#include "../common/user_config.h"
#include "../common/sound_effect_json.h"
#include "../common/user_info_json.h"

class UkuiVolumeControlUserConfig : public UserConfig
{
public:
    UkuiVolumeControlUserConfig();
    ~UkuiVolumeControlUserConfig();

public:
    std::unordered_map<JsonType, std::shared_ptr<IJson>> getJsonMap() const;
    virtual void initJsonMap();

private:
    QString getSoundEffectFullPath();
    QString getUserInfoFullPath();

private:
    std::unordered_map<JsonType, std::function<QString()>> m_keys {
        {JsonType::JSON_TYPE_SOUNDEFFECT, [this](){ return getSoundEffectFullPath();}},
        {JsonType::JSON_TYPE_USERINFO, [this](){ return getUserInfoFullPath(); }},
    };
};

#endif // UKUIVOLUMECONTROLUSERCONFIG_H
