#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QString>
#include <list>

//#include "IAudioMainWidow.h"
#include "VersionStrategy.h"
#include "Util.h"
#include "DBusClient.h"
extern "C" {
#include <canberra.h>
}

using namespace UkuiAudioFramwork;

#define MAX_INDEX 4294967295

#ifdef __GNUC__
#define CA_CLAMP(x, low, high)                                          \
        __extension__ ({ typeof(x) _x = (x);                            \
                        typeof(low) _low = (low);                       \
                        typeof(high) _high = (high);                    \
                        ((_x > _high) ? _high : ((_x < _low) ? _low : _x)); \
                })
#else
#define CA_CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget* = nullptr);
    ~MainWidget() {}

    void showAppControlWidget();

private Q_SLOTS:
    void volumeChangedSlots(int, int, const QDBusVariant&);

    void muteChangedSlots(int, int, bool);

    void deviceChangedSlots(int, const QString&, const QString&);

    void deviceAdjustSlots(int);

    void settingsChangedSlots(const QString&, const QDBusVariant&);

    void addStreamSlots(int, int, int, bool, const QString&, const QString&);

    void removeStreamSlots(int, int);

private:
    DesktopEnvironmentVersion getDesktopEnvironmentVersion();

    int getVolume(int, int);

    void setVolume(int, int, int);

    double getBalance(int) const;

    void setBalance(int, double);

    bool getMute(int, int);

    void setMute(int, int, bool);

    void setEnabled(const QString&, const QString&, bool);

    bool isEnabled(const QString&, const QString&) const;

    bool isValidDevice(int) const;

    ClientDefaultDeviceInfo getDefaultDevice(int, int);

    QList<ClientDeviceInfo> getAvailablePortList(const NodeType&);

    QList<StreamInfo> getSinkInputList() const;

    QList<StreamInfo> getSourceOutputList() const;

    QList<ClientStreamMediaInfo> getStreamMediaList() const;

    QList<ClientDefaultDeviceInfo> getSinkList() const;

    QList<ClientDefaultDeviceInfo> getSourceList() const;

    void updateAppDeviceList(QList<ClientDeviceInfo>&, const QList<ClientDefaultDeviceInfo>&);

    void setDefaultDevice(int, int, const QString&, const QString&);

    void setValue(const QString&, const QVariant&);

    QVariant getValue(const QString&) const;

    QList<ClientSoundThemeInfo> getSoundThemeList();

    QList<ClientSoundEffectFileInfo> getSoundEffectFileList(const QString&);

    void initUi();

    void initSystemVolumeSlider();

    void initSystemDeviceList(const NodeType&);

    void initCurrentDefaultDevice(const NodeType&);

    void initDeviceManagerList(const NodeType&);

    void initAppManagerList();

    void initAppCurrentDefaultDevice(int, const NodeType&, int);

    void initSwitchButtonStatus();

    void initSoundThemeList();

    void initData();

    void initSlots();

    void initClientManager();

    void updateSoundTheme();

    bool getAutoPauseStatus() const;

    void setAutoPauseStatus(bool);

    bool getMultiAudioCombineStatus(const NodeType&) const;

    void updateAppWidgetDeviceList(const NodeType&, bool isOnlyUpdateAppList = false);

    bool hasEnabledPort(const NodeType&);

    void updateLoopbackFrameVisible(int comboxIdx, const ClientDefaultDeviceInfo& devInfo);

    void syncAppOutputVolumeControls(bool hasEnabledOutput);

    QString getAppStreamNameByIndex(const NodeType&, int);

    void addAppManagerItem(int, int, int, bool, const QString&, const QString&);

    void setAppManagerSliderRange(int, int);

    void setAppManagerSliderValue(int);

    void playSoundByMethod(QString method);
    void playAlretSoundFromPath(QWidget *w, QString path);
    int caProplistMergeAp(ca_proplist *p, va_list ap);
    int caPlayForWidget(QWidget *w, uint32_t id, ...);
    int caProplistSetForWidget(ca_proplist *p, QWidget *widget);

private:
    static constexpr std::string_view m_kylinSettingsSystemStr = "kylin-settings-system";

    DesktopEnvironmentVersion m_version = DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI2;
    IAudioMainWindow* m_pMainWid = nullptr;

    std::list<AppManagerItem> m_appWidgetList {};
    std::shared_ptr<VersionStrategy> m_pStrategy = nullptr;
    std::shared_ptr<ClientManager> m_pClientManager = nullptr;

    std::unordered_map<SelectComboxType, std::tuple<std::string, std::string>> m_selectComboxKeys {
        {SelectComboxType::SELECT_COMBOX_SOUNDEFFECT, std::tuple(THEME_NAME_KEY, "setSoundThemeName")},
        {SelectComboxType::SELECT_COMBOX_VOLUMEADJUST, std::tuple(VOLUME_CHANGED_KEY, "setVolumeChangedName")},
        {SelectComboxType::SELECT_COMBOX_NOTIFICTION, std::tuple(NOTIFY_GENERAL_KEY, "setNotifyGeneralName")},
    };

    std::unordered_map<uint32_t, QStringList> m_appKeys {
        {MAX_INDEX, QStringList{std::initializer_list<QString>{"kylin-settings-system", QObject::tr("System Volume")}}},
    };

    std::unordered_map<TitleLabelType, TitleLabelItem*> m_titleLabelItemMap {};
    std::unordered_map<SwitchButtonType, ISwitchButtonItem*> m_switchBtnItemMap {};
    std::unordered_map<VolumeSliderType, IVolumeSliderItem*> m_volumeSliderItemMap {};
    std::unordered_map<SelectComboxType, ISelectComboxItem*> m_selectComboxItemMap {};
    std::unordered_map<DetailSettingType, IDetailSettingsItem*> m_detailSettingsItemMap {};

    ca_context *m_caContext = nullptr;
};

#endif // MAINWIDGET_H
