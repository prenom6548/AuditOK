#ifndef PIPEWIREMANAGER_H
#define PIPEWIREMANAGER_H

#include "AudioManager.h"

namespace UkuiAudioFramwork
{
class PipewireManager : public AudioManager
{
public:
    PipewireManager(int);
    ~PipewireManager() = default;

public:
    virtual void init() override;

    virtual bool connect() override;

    // Interface
    virtual void loadModule(const std::string&, const std::string&) override;

    virtual void unloadModule(const std::string&, bool restoreMasterDevice = true) override;

    // Method
    virtual void setDetailVolume(const NodeType&, const std::string&, const std::string&, int) override;

    virtual int getDetailVolume(const NodeType&, const std::string&, const std::string&) const override;

    virtual void setVolume(const NodeType&, int, int) override;

    virtual int getVolume(const NodeType&, int) const override;

    virtual void setDetailMute(const NodeType&, const std::string&, const std::string&, bool) override;

    virtual bool getDetailMute(const NodeType&, const std::string&, const std::string&) const override;

    virtual void setBalance(const NodeType&, double) override;

    virtual double getBalance(const NodeType&) const override;

    virtual void setMute(const NodeType&, int, bool) override;

    virtual bool getMute(const NodeType&, int) const override;

    virtual DefaultDeviceInfo getDefaultDevice(const NodeType&, int) const override;

    virtual void setDefaultDevice(const NodeType&, int, const std::string&, const std::string&) override;

    virtual std::vector<std::string> getDeviceList(const NodeType&) const override;

    virtual std::list<DefaultDeviceInfo> getSinkList() const override;

    virtual std::list<DefaultDeviceInfo> getSourceList() const override;

    virtual std::list<std::shared_ptr<DeviceInfo>> getAvailablePortList(const NodeType&) const override;

    virtual std::list<StreamMediaInfo> getSinkInputList() const override;

    virtual std::list<StreamMediaInfo> getSourceOutputList() const override;

    virtual std::list<StreamMediaIntegrationInfo> getStreamMediaList() const override;

    virtual std::list<std::string> getModuleList() override;

    virtual void setEnabled(std::shared_ptr<RestoreVolumeJson>, const std::string&, const std::string&, bool) override;

    virtual bool isEnabled(const std::string&, const std::string&) const override;

    virtual bool isLoaded(const ModuleType&) const override;

    virtual bool isValidDevice(const NodeType&) const override;

    // signal
    virtual void deviceChanged(const NodeType&, const std::string&, const std::string&) override;

    virtual void deviceAdjust(const NodeType&) override;

    virtual void portChanged(const NodeType&, const std::string&) override;

    virtual void cardRemoved(const std::string&) override;

    virtual void addStream(int, int, const std::string&, const std::string&) override;

    virtual void removeStream(int) override;

    virtual void addStream(int, int, int, bool, const std::string&, const std::string&) override;

    virtual void removeStream(int, int) override;

private:
    int m_reconnectTime;
};
}
#endif // PIPEWIREMANAGER_H
