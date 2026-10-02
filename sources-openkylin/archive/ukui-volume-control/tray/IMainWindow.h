#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QString>
#include <functional>

#include "ISystemVolumeWidget.h"
#include "IAppVolumeWidget.h"
#include "DBusClient.h"

class IMainWindow
{
public:
    IMainWindow() = default;
    virtual ~IMainWindow() {}

public:
    using VolumeChangedCallback = std::function<void(int, int)>;

    using MuteChangedCallback = std::function<void(int, const QString&)>;

    using DeviceChangedCallback = std::function<void(int, const QString&, const QString&)>;

    using TrayIconTriggerCallback = std::function<void()>;

    using SizeChangedCallback = std::function<void(const QSize&)>;

    std::shared_ptr<VolumeChangedCallback> addVolumeObserver(const VolumeChangedCallback&);

    void removeVolumeObserver(const std::shared_ptr<VolumeChangedCallback>&);

    void setVolume(int, int);

    std::shared_ptr<MuteChangedCallback> addMuteObserver(const MuteChangedCallback&);

    void removeMuteObserver(const std::shared_ptr<MuteChangedCallback>&);

    void setMute(int, const QString&);

    std::shared_ptr<DeviceChangedCallback> addDeviceObserver(const DeviceChangedCallback&);

    void removeDeviceObserver(const std::shared_ptr<DeviceChangedCallback>&);

    void setDefaultDevice(int, const QString&, const QString&);

    std::shared_ptr<TrayIconTriggerCallback> addTrayIconTriggerObserver(const TrayIconTriggerCallback&);

    void removeTrayIconTriggerObserver(const std::shared_ptr<TrayIconTriggerCallback>&);

    void triggerWidget();

    std::shared_ptr<SizeChangedCallback> addSizeChangedObserver(const SizeChangedCallback&);

    void removeSizeChangedObserver(const std::shared_ptr<SizeChangedCallback>&);

    void setSize(const QSize&);

public:
    virtual void initUi() = 0;

    virtual void initSlots() = 0;

    virtual void addAppItem(int, const QString&, const QString&) = 0;

    virtual void removeAppItem(int) = 0;

    virtual void addAppToWidget(const QString&, const QString&) = 0;

    virtual void removeAppFromWidget(const QString&) = 0;

    virtual void addDevToWidget(const QString&, const ClientDeviceInfo&) = 0;

    virtual void removeDevFromWidget(const QString&, const QString&) = 0;

    virtual void setCurrentIndex(int) = 0;

    virtual void setParentWidget(QWidget*) = 0;

    virtual void setValue(int, int, int) = 0;

    virtual void updateDescription(const QString&, const QString&) = 0;

private:
    void volumeNotify(int, int);

    void muteNotify(int, const QString&);

    void defaultDeviceNotify(int, const QString&, const QString&);

    void trayIconTriggerNotify();

    void sizeChangedNotify(const QSize&);

protected:
    std::shared_ptr<ISystemVolumeWidget> m_pSysVolumeWidget = nullptr;
    std::shared_ptr<IAppVolumeWidget> m_pAppVolumeWidget = nullptr;

private:
    std::vector<std::shared_ptr<VolumeChangedCallback>> m_volumeObservers;
    std::vector<std::shared_ptr<MuteChangedCallback>> m_muteObservers;
    std::vector<std::shared_ptr<DeviceChangedCallback>> m_deviceObservers;
    std::vector<std::shared_ptr<TrayIconTriggerCallback>> m_trayIconObservers;
    std::vector<std::shared_ptr<SizeChangedCallback>> m_sizeChangedObservers;
};

#endif // MAINWINDOW_H
