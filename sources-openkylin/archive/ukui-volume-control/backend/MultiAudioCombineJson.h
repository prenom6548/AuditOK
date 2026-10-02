#ifndef MULTIAUDIOCOMBINEJSON_H
#define MULTIAUDIOCOMBINEJSON_H

#include <optional>
#include <vector>
#include "IJson.h"

namespace UkuiAudioFramwork
{
#define MULTI_AUDIO_COMBINE_JSON  "/.config/ukui-volume-control/multi_audio_combine.json"

typedef struct MultiAudioCombineInfo {
    int type;       // 0 输入  1 输出
    bool status;
    std::vector<std::string> deviceList;
} MultiAudioCombineInfo;

class MultiAudioCombineJson : public IJson<MultiAudioCombineInfo>
{
public:
    MultiAudioCombineJson();
    ~MultiAudioCombineJson() = default;

public:
    virtual bool isContain(const MultiAudioCombineInfo&) const override;

    virtual void insert(const MultiAudioCombineInfo&) override;

//    template<typename T>
//    std::optional<T> getValue(const std::string&, const std::string&, const std::string&) const;

    std::optional<bool> getValue(const std::string&, const std::string&) const;

    void setValue(const std::string&, const std::string&, bool);

private:
    void initJson();

};
}
#endif // MULTIAUDIOCOMBINEJSON_H
