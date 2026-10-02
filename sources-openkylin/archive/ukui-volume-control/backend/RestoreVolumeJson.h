#ifndef RESTOREVOLUMEJSON_H
#define RESTOREVOLUMEJSON_H

#include <optional>
#include <variant>

#include "IJson.h"

namespace UkuiAudioFramwork
{
#define RESTORE_VOLUME_JSON  "/.config/ukui-volume-control/restore-volume.json"

struct  RestoreVolumeInfo
{
    std::string cardName;
    std::string portName;
    uint32_t volume;
    uint32_t priority;
    double balance;
    bool enable;
    bool muted;
};

using MemberVarPtr = std::variant<
    uint32_t RestoreVolumeInfo::*,
    double RestoreVolumeInfo::*,
    bool RestoreVolumeInfo::*
>;

using FieldMap = std::vector<std::pair<const char*, MemberVarPtr>>;

class RestoreVolumeJson : public IJson<RestoreVolumeInfo>
{
public:
    RestoreVolumeJson();
    ~RestoreVolumeJson() = default;

public:
    virtual bool isContain(const RestoreVolumeInfo&) const override;

    virtual void insert(const RestoreVolumeInfo&) override;

//    template<typename T>
//    std::optional<T> getValue(const std::string&, const std::string&, const std::string&) const;

    std::optional<bool> getValue(const std::string&) const;

    bool getStatus(const std::string&, const std::string&, const std::string&) const;

    void setValue(const std::string&, bool);

private:
    void initJson();

private:
    const FieldMap m_filedMap = {
        {"volume",    &RestoreVolumeInfo::volume},
        {"priority",  &RestoreVolumeInfo::priority},
        {"balance",   &RestoreVolumeInfo::balance},
        {"enable",    &RestoreVolumeInfo::enable},
        {"mute",      &RestoreVolumeInfo::muted}
    };
};
}
#endif // RESTOREVOLUMEJSON_H
