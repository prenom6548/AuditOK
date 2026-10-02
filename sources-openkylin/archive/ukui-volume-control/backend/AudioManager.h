#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <vector>
#include <nlohmann/json.hpp>

#include "Util.h"
#include "PaDataType.h"
#include "RestoreVolumeJson.h"

namespace UkuiAudioFramwork
{
struct DeviceInfo;
class AudioManager {
public:
    AudioManager();
    virtual ~AudioManager() = default;

public:
    // Method
    std::string getBackend() const;

    void setBackend(BackendType);

    void waitBluezReady(int);

    void setBluezReadyStatus(bool);

    bool getBluezReady() const;

public:
    virtual void init() = 0;

    virtual bool connect() = 0;

    // Interface
    virtual void loadModule(const std::string&, const std::string&) = 0;

    virtual void unloadModule(const std::string&, bool restoreMasterDevice = true) = 0;

    virtual void setDetailVolume(const NodeType&, const std::string&, const std::string&, int volume) = 0;

    virtual int getDetailVolume(const NodeType&, const std::string&, const std::string&) const = 0;

    virtual void setVolume(const NodeType&, int, int) = 0;

    virtual int getVolume(const NodeType&, int) const = 0;

    virtual void setBalance(const NodeType&, double) = 0;

    virtual double getBalance(const NodeType&) const = 0;

    virtual void setDetailMute(const NodeType&, const std::string&, const std::string&, bool) = 0;

    virtual bool getDetailMute(const NodeType&, const std::string&, const std::string&) const = 0;

    virtual void setMute(const NodeType&, int, bool) = 0;

    virtual bool getMute(const NodeType&, int = -1) const = 0;

    virtual DefaultDeviceInfo getDefaultDevice(const NodeType&, int) const = 0;

    virtual void setDefaultDevice(const NodeType&, int, const std::string&, const std::string&) = 0;

    virtual std::vector<std::string> getDeviceList(const NodeType&) const = 0;

    virtual std::list<DefaultDeviceInfo> getSinkList() const = 0;

    virtual std::list<DefaultDeviceInfo> getSourceList() const = 0;

    virtual std::list<std::shared_ptr<DeviceInfo>> getAvailablePortList(const NodeType&) const = 0;

    virtual std::list<StreamMediaInfo> getSinkInputList() const = 0;

    virtual std::list<StreamMediaInfo> getSourceOutputList() const = 0;

    virtual std::list<StreamMediaIntegrationInfo> getStreamMediaList() const = 0;

    virtual std::list<std::string> getModuleList() = 0;

    virtual void setEnabled(std::shared_ptr<RestoreVolumeJson>, const std::string&, const std::string&, bool) = 0;

    virtual bool isEnabled(const std::string&, const std::string&) const = 0;

    virtual bool isLoaded(const ModuleType&) const = 0;

    virtual bool isValidDevice(const NodeType&) const = 0;

    // signal
    virtual void deviceChanged(const NodeType&, const std::string&, const std::string&) = 0;

    virtual void portChanged(const NodeType&, const std::string&) = 0;

    virtual void cardRemoved(const std::string&) = 0;

    virtual void deviceAdjust(const NodeType&) = 0;

    virtual void addStream(int, int, const std::string&, const std::string&) = 0;

    virtual void removeStream(int) = 0;

    virtual void addStream(int, int, int, bool, const std::string&, const std::string&) = 0;

    virtual void removeStream(int, int) = 0;

private:
    bool checkAudioService(const std::string&);

private:
    BackendType m_type = BackendType::PULSEAUDIO;
    bool m_bBlueReady = false;

    mutable std::mutex m_blueMtx;
    mutable std::condition_variable m_blueCond;
};
}
#endif // AUDIOMANAGER_H
