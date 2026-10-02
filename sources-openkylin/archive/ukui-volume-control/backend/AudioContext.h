#ifndef AUDIOCONTEXT_H
#define AUDIOCONTEXT_H

#include "simple_logger/Logger.h"
#include "BaseType.h"
#include "Util.h"

namespace UkuiAudioFramwork
{
#define AUDIO_DEBUG(fmt, ...) DBG_DEBUG(AudioContext::getInstance().getLogger(), EnumToInt(AudioContext::getInstance().getModule()), fmt, ##__VA_ARGS__)
#define AUDIO_INFO(fmt, ...) DBG_INFO(AudioContext::getInstance().getLogger(), EnumToInt(AudioContext::getInstance().getModule()), fmt, ##__VA_ARGS__)
#define AUDIO_WARN(fmt, ...) DBG_WARN(AudioContext::getInstance().getLogger(), EnumToInt(AudioContext::getInstance().getModule()), fmt, ##__VA_ARGS__)
#define AUDIO_ERROR(fmt, ...) DBG_ERROR(AudioContext::getInstance().getLogger(), EnumToInt(AudioContext::getInstance().getModule()), fmt, ##__VA_ARGS__)
#define AUDIO_FATAL(fmt, ...) DBG_FATAL(AudioContext::getInstance().getLogger(), EnumToInt(AudioContext::getInstance().getModule()), fmt, ##__VA_ARGS__)

class AudioContext
{
public:
    static AudioContext& getInstance();

    /** Resolves log directory; exposed for tests (also used by constructor). */
    static std::string getLogPath();

public:
    Module getModule() const;
    simple_logger::Log& getLogger();

private:
    AudioContext();
    AudioContext(const AudioContext&) = delete;
    AudioContext(AudioContext&&) = delete;
    AudioContext operator=(const AudioContext&) = delete;
    AudioContext operator=(AudioContext&&) = delete;
    virtual ~AudioContext() {
        m_log.Close();
    }

private:
    simple_logger::Log m_log;
    Module m_module = Module::Service;
};
}


#endif // AUDIOCONTEXT_H
