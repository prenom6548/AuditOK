#ifndef MULTIAUDIOCOMBINEMODULE_H
#define MULTIAUDIOCOMBINEMODULE_H

#include "Util.h"
#include "MultiAudioCombineJson.h"

#include <mutex>

namespace UkuiAudioFramwork
{
class MultiAudioCombineModule
{
public:
    static MultiAudioCombineModule& getInstance();

    void setJson(std::shared_ptr<MultiAudioCombineJson>);

    bool getMultiAudioCombineStatus(const NodeType&) const;

    void setMultiAudioCombine(const NodeType&, bool, const std::vector<std::string>&);

private:
    MultiAudioCombineModule() = default;
    MultiAudioCombineModule(const MultiAudioCombineModule&) = delete;
    MultiAudioCombineModule(MultiAudioCombineModule&&) = delete;
    MultiAudioCombineModule operator=(const MultiAudioCombineModule&) = delete;
    MultiAudioCombineModule operator=(MultiAudioCombineModule&&) = delete;
    virtual ~MultiAudioCombineModule() = default;

private:
    uint32_t m_retryTimes = 0;
    std::mutex m_mtx;
    std::shared_ptr<MultiAudioCombineJson> m_pJson;
};
}
#endif // MULTIAUDIOCOMBINEMODULE_H
