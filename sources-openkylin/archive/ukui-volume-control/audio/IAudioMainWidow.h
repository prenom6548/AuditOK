#ifndef IAUDIOMAINWINDOW_H
#define IAUDIOMAINWINDOW_H

#include <QIcon>
#include <QString>

#include "DBusClient.h"
#include "ClientManager.h"
#include "MonitorStream.h"
#include "AppManagerItemWidget.h"
#include "AppManagerMainWidget.h"

typedef struct DevicePortInfo {
    uint32_t index;    // 处于控件的位置
    QString cardName;  // 声卡名称
    QString portName;  // 设备端口名称
} DevicePortInfo;

class IAudioMainWindow
{
public:
    IAudioMainWindow(std::list<AppManagerItem>&);
    virtual ~IAudioMainWindow() = default;

public:
    using VolumeChangedCallback = std::function<void(int, int, int)>;

    using BalanceVolumeChangedCallback = std::function<void(int, double)>;

    using MuteChangedCallback = std::function<void(int, int)>;

    using DeviceChangedCallback = std::function<void(int, int, const QString&, const QString&)>;

    using SettingsChangedCallback = std::function<void(const QString&, const QVariant&)>;

    using DetailSettingsClickedCallback = std::function<void(int)>;

    using EnableDeviceCallback = std::function<void(const QString&, const QString&, bool)>;

    std::shared_ptr<VolumeChangedCallback> addVolumeObserver(const VolumeChangedCallback&);

    void removeVolumeObserver(const std::shared_ptr<VolumeChangedCallback>&);

    void setVolume(int, int, int);

    std::shared_ptr<BalanceVolumeChangedCallback> addBalanceVolumeObserver(const BalanceVolumeChangedCallback&);

    void removeBalanceVolumeObserver(const std::shared_ptr<BalanceVolumeChangedCallback>&);

    void setBalance(int, double);

    std::shared_ptr<MuteChangedCallback> addMuteObserver(const MuteChangedCallback&);

    void removeMuteObserver(const std::shared_ptr<MuteChangedCallback>&);

    void setMute(int, int);

    std::shared_ptr<DeviceChangedCallback> addDeviceObserver(const DeviceChangedCallback&);

    void removeDeviceObserver(const std::shared_ptr<DeviceChangedCallback>&);

    void setDefaultDevice(int, int, const QString&, const QString&);

    std::shared_ptr<SettingsChangedCallback> addSettingsObserver(const SettingsChangedCallback&);

    void removeSettingsObserver(const std::shared_ptr<SettingsChangedCallback>&);

    void setSettingsValue(const QString&, const QVariant&);

    std::shared_ptr<DetailSettingsClickedCallback> addDetailSettingsObserver(const DetailSettingsClickedCallback&);

    void removeDetailSettingsObserver(const std::shared_ptr<DetailSettingsClickedCallback>&);

    void setDetailSettings(int);

    std::shared_ptr<EnableDeviceCallback> addEnableDeviceObserver(const EnableDeviceCallback&);

    void removeEnableDeviceObserver(const std::shared_ptr<EnableDeviceCallback>&);

    void setEnabled(const QString&, const QString&, bool);

    void startMonitor(uint32_t);

    void stopMonitor();

    AppManagerMainWidget* getAppManagerWidget() const;

public:
    virtual void initUi() = 0;

    virtual void initSlots() = 0;

    virtual void setCurrentIndex(int, int) = 0;

    virtual void setAppCurrentIndex(const QString&, const NodeType&, int) = 0;

    virtual void setCurrentAppItem(int) = 0;

    virtual void setAppCbxEnabled(const QString&, const NodeType&, bool) = 0;

    virtual void setAppValue(const QString&, int) = 0;

    virtual void setValue(int, int) = 0;

    virtual void setAppMute(const QString&, bool) = 0;

    virtual void setChecked(int, bool) = 0;

    virtual std::list<DevicePortInfo> getDevicePortList(int) const = 0;

    virtual std::list<DevicePortInfo> getDeviceManagetPortList(int) const = 0;

    virtual std::list<AppManagerItem>& getAppItemList() = 0;

    virtual std::list<DevicePortInfo> getAppManagetPortList(int, int) const = 0;

    virtual QWidget* getWidget() = 0;

    virtual void addDeviceManagerItem(int, const QString&, const QVariant&) = 0;

    virtual void removeDeviceManagerItem(int, int) = 0;

    virtual void setDeviceManagerItemStatus(int, int, bool) = 0;

    virtual void updateDeviceManagerStatus(int, bool) = 0;

    virtual void updateUIByButtonStatus(int, bool) = 0;

    virtual void updateUIByDevice(int type, bool status) = 0;

    virtual void addAppManagerItem(int, int, int, bool, const QString&, const QString&, const QList<ClientDeviceInfo>&, const QList<ClientDeviceInfo>&) = 0;

    virtual void removeAppManagerItem(const QString&) = 0;

    virtual void addPortToAppItem(const QString&, const NodeType&, const QString&, const QVariant&) = 0;

    virtual void removePortFromAppItem(const QString&, const NodeType&, int) = 0;

    virtual void showAppControlWidget() = 0;    // 响应任务栏跳转控制面板声音二级弹窗应用管理页面

private:
    void volumeNotify(int, int, int);

    void balanceVolumeNotify(int, double);

    void muteNotify(int, int);

    void defaultDeviceNotify(int, int, const QString&, const QString&);

    void settingsChangedNotify(const QString&, const QVariant&);

    void detailSettingsClickNotify(int);

    void enableDeviceNotify(const QString&, const QString&, bool);

protected:
    std::shared_ptr<ClientManager> m_pClientManager {nullptr};
    std::shared_ptr<MonitorStream> m_pMonitorStream {nullptr};

    AppManagerMainWidget* m_pAppManagerWidget = nullptr;
    std::list<AppManagerItem>& m_appWidgetList;

    std::unordered_map<SwitchButtonType, QString> m_switchMethodKeys {
        {SwitchButtonType::SWITCH_BUTTON_LOOPBACK, "setLoopbackStatus"},
        {SwitchButtonType::SWITCH_BUTTON_MONO, "setMonoStatus"},
        {SwitchButtonType::SWITCH_BUTTON_ECHOCANCEL, "setEchoCancelStatus"},
        {SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST, "setVolumeBoostStatus"},
        {SwitchButtonType::SWITCH_BUTTON_ALERT, "setAlertStatus"},
        {SwitchButtonType::SWITCH_BUTTON_STARTUP, "setStartupStatus"},
        {SwitchButtonType::SWITCH_BUTTON_POWEROFF, "setPoweroffStatus"},
        {SwitchButtonType::SWITCH_BUTTON_LOGOUT, "setLogoutStatus"},
        {SwitchButtonType::SWITCH_BUTTON_WAKEUP, "setWakeupStatus"},
        {SwitchButtonType::SWITCH_BUTTON_AUTO_PAUSE, "setAutoPauseStatus"},
    };

    std::unordered_map<SelectComboxType, QString> m_comboxMethodKeys {
        {SelectComboxType::SELECT_COMBOX_SOUNDEFFECT, "setSoundThemeName"},
        {SelectComboxType::SELECT_COMBOX_VOLUMEADJUST, "setVolumeChangedName"},
        {SelectComboxType::SELECT_COMBOX_NOTIFICTION, "setNotifyGeneralName"},
    };

    std::unordered_map<TitleLabelType, QString> m_titleLabelKeys {
        //~ contents_path /Audio/Output
        {TitleLabelType::TITLE_LABEL_OUTPUT, QObject::tr("Output")},
        //~ contents_path /Audio/Input
        {TitleLabelType::TITLE_LABEL_INPUT, QObject::tr("Input")},
        //~ contents_path /Audio/Sound Effect
        {TitleLabelType::TITLE_LABEL_SOUNDEFFECT, QObject::tr("Sound Effect")},
        //~ contents_path /Audio/Advance Settings
        {TitleLabelType::TITLE_LABEL_ADVANCESETTINGS, QObject::tr("Advance Settings")},
    };

    std::unordered_map<SwitchButtonType, QStringList> m_switchButtonkeys {
        //~ contents_path /Audio/Volume Increase
        {SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST, QStringList(std::initializer_list<QString>{QObject::tr("Volume Increase"), QObject::tr("Volume above 100% can cause sound distortion and damage your speakers"), QString("VolumeIncrease")})},
        //~ contents_path /Audio/Mono Audio
        {SwitchButtonType::SWITCH_BUTTON_MONO, QStringList(std::initializer_list<QString>{QObject::tr("Mono Audio"), QObject::tr("It merges the left and right channels into one channel"), QString("MonoAudio")})},
        //~ contents_path /Audio/Noise Reduction
        {SwitchButtonType::SWITCH_BUTTON_ECHOCANCEL, QStringList(std::initializer_list<QString>{QObject::tr("Noise Reduction"), QString("NoiseReduction")})},
        //~ contents_path /Audio/Voice Monitor
        {SwitchButtonType::SWITCH_BUTTON_LOOPBACK, QStringList(std::initializer_list<QString>{QObject::tr("Voice Monitor"), QObject::tr("You can hear your voice in the output device of your choice"), QString("VoiceMonitor")})},
        //~ contents_path /Audio/Startup Misic
        {SwitchButtonType::SWITCH_BUTTON_STARTUP, QStringList(std::initializer_list<QString>{QObject::tr("Startup Misic"), QString("StartupMisic")})},
        //~ contents_path /Audio/Poweroff Music
        {SwitchButtonType::SWITCH_BUTTON_POWEROFF, QStringList(std::initializer_list<QString>{QObject::tr("Poweroff Music"), QString("PoweroffMusic")})},
        //~ contents_path /Audio/Logout Music
        {SwitchButtonType::SWITCH_BUTTON_LOGOUT, QStringList(std::initializer_list<QString>{QObject::tr("Logout Music"), QString("LogoutMusic")})},
        //~ contents_path /Audio/Wakeup Music
        {SwitchButtonType::SWITCH_BUTTON_WAKEUP, QStringList(std::initializer_list<QString>{QObject::tr("Wakeup Music"), QString("WakeupMusic")})},
        //~ contents_path /Audio/Beep Switch
        {SwitchButtonType::SWITCH_BUTTON_ALERT, QStringList(std::initializer_list<QString>{QObject::tr("Beep Switch"), QString("BeepSwitch")})},
        {SwitchButtonType::SWITCH_BUTTON_AUTO_PAUSE, QStringList(std::initializer_list<QString>{QObject::tr("Pause playback when audio device is disconnected"), QString("PauseDisconnected")})}
    };

    std::unordered_map<VolumeSliderType, QStringList> m_volumeSliderKeys {
        //~ contents_path /Audio/Volume
        {VolumeSliderType::VOLUME_SLIDER_INPUT, QStringList(std::initializer_list<QString>{QObject::tr("Volume"), QString("Volume")})},
        //~ contents_path /Audio/Master Volume
        {VolumeSliderType::VOLUME_SLIDER_OUTPUT, QStringList(std::initializer_list<QString>{QObject::tr("Master Volume"), QString("MasterVolume")})},
        //~ contents_path /Audio/Input Level
        {VolumeSliderType::VOLUME_SLIDER_INPUTLEVEL, QStringList(std::initializer_list<QString>{QObject::tr("Input Level"), QString("InputLevel")})},
        //~ contents_path /Audio/Balance
        {VolumeSliderType::VOLUME_SLIDER_BALANCE, QStringList(std::initializer_list<QString>{QObject::tr("Balance"), QObject::tr("Left"), QObject::tr("Right"), QString("Balance")})}
    };

    std::unordered_map<SelectComboxType, QStringList> m_selectComboxKeys {
        //~ contents_path /Audio/Input Device
        {SelectComboxType::SELECT_COMBOX_INPUT, QStringList(std::initializer_list<QString>{QObject::tr("Input Device"), QString("InputDevice")})},
        //~ contents_path /Audio/Output Device
        {SelectComboxType::SELECT_COMBOX_OUTPUT, QStringList(std::initializer_list<QString>{QObject::tr("Output Device"), QString("OutputDevice")})},
        //~ contents_path /Audio/Sound Theme
        {SelectComboxType::SELECT_COMBOX_SOUNDEFFECT, QStringList(std::initializer_list<QString>{QObject::tr("Sound Theme"), QString("SoundTheme")})},
        //~ contents_path /Audio/Volume Control Sound
        {SelectComboxType::SELECT_COMBOX_VOLUMEADJUST, QStringList(std::initializer_list<QString>{QObject::tr("Volume Control Sound"), QString("VolumeControlSound")})},
        //~ contents_path /Audio/Notification Sound
        {SelectComboxType::SELECT_COMBOX_NOTIFICTION, QStringList(std::initializer_list<QString>{QObject::tr("Notification Sound"), QString("NotificationSound")})}
    };

    std::unordered_map<DetailSettingType, QStringList> m_detailSettingsKeys {
        //~ contents_path /Audio/Sound Equipment Control
        {DetailSettingType::DETAIL_SETTING_DEVMANAGER, QStringList(std::initializer_list<QString>{QObject::tr("Sound Equipment Control"), QString("SoundEquipmentControl")})},
        //~ contents_path /Audio/App Sound Control
        {DetailSettingType::DETAIL_SETTING_APPMANAGER, QStringList(std::initializer_list<QString>{QObject::tr("App Sound Control"), QString("AppSoundControl")})},
    };

private:
    std::vector<std::shared_ptr<VolumeChangedCallback>> m_volumeObservers;
    std::vector<std::shared_ptr<BalanceVolumeChangedCallback>> m_balanceVolumeObservers;
    std::vector<std::shared_ptr<MuteChangedCallback>> m_muteObservers;
    std::vector<std::shared_ptr<DeviceChangedCallback>> m_deviceObservers;
    std::vector<std::shared_ptr<SettingsChangedCallback>> m_settingsObservers;
    std::vector<std::shared_ptr<DetailSettingsClickedCallback>> m_detailSettingsObservers;
    std::vector<std::shared_ptr<EnableDeviceCallback>> m_enableDeviceObservers;
};

#endif // IAUDIOMAINWINDOW_H
