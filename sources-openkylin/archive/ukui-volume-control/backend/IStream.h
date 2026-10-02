#ifndef ISTREAM_H
#define ISTREAM_H

#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/ext-device-restore.h>
#include <pulse/ext-device-manager.h>
#include <string>
#include <vector>

#include "PaDataType.h"
#include "AudioManager.h"

namespace UkuiAudioFramwork
{
struct PaCardPortInfo;
struct PaDataInfo;
struct DeviceInfo;
class AudioDBusServer;

enum class StreamType {
    STREAM_TYPE_SINK,
    STREAM_TYPE_SOURCE,
    STREAM_TYPE_SINKINPUT,
    STREAM_TYPE_SOURCEOUTPUT
};

class IStream {
public:
    IStream(pa_context* ctx, PaDataInfo& info, StreamType type)
        : m_pContext(ctx), m_info(info), m_type(type) {}
    virtual ~IStream() = default;

public:
    virtual std::string toString() const = 0;

    virtual NodeType getNodeType() const = 0;

    virtual uint32_t getIndex() const = 0;

    virtual std::string getName() const = 0;

    virtual pa_proplist* getProplist() const = 0;

    virtual uint32_t getCard() const = 0;

    virtual void setVolume(int) = 0;

    virtual const pa_cvolume getVolume() const = 0;

    virtual void setBalance(double) = 0;

    virtual double getBalance() const = 0;

    virtual void setMute(bool) = 0;

    virtual bool getMute() const = 0;

    virtual DefaultDeviceInfo getDefaultDevice() const = 0;

    virtual void setDefaultDevice(const std::string&, const std::string&) = 0;

    virtual std::vector<std::string> getDeviceList() const = 0;

    virtual std::list<std::shared_ptr<DeviceInfo>> getAvailablePortList() const = 0;

    virtual void volumeChanged(int) = 0;

    virtual void muteChanged(bool) = 0;

    virtual void portChanged(const std::string&) = 0;

    virtual void deviceChanged(bool = false) = 0;

protected:
    pa_context* m_pContext;
    PaDataInfo& m_info;
    StreamType m_type;
};
}
#endif // ISTREAM_H
