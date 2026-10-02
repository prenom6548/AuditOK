#ifndef UKUI5MAINWIDGET_H
#define UKUI5MAINWIDGET_H

#include "IAudioMainWidow.h"
#include "ISwitchButtonItem.h"
#include "IVolumeSliderItem.h"
#include "ISelectComboxItem.h"
#include "Util.h"
#include "DeviceManagerMainWidget.h"
#include "klineframe.h"

using namespace UkuiAudioFramwork;
using namespace kdk;

enum class FrameType
{
    BALANCE_MONO_FRAME = 0, // 声道平衡及单声道界面Frame
    LOOP_BACK_FRAME,        // 侦听界面子Frame
    SOUND_THEME_FRAME       // 音效主题设置界面子Frame
};

class Ukui5MainWidget : public QWidget, public IAudioMainWindow
{
    Q_OBJECT

public:
    Ukui5MainWidget(std::unordered_map<TitleLabelType, TitleLabelItem*>&,
                    std::unordered_map<SwitchButtonType, ISwitchButtonItem*>&,
                    std::unordered_map<VolumeSliderType, IVolumeSliderItem*>&,
                    std::unordered_map<SelectComboxType, ISelectComboxItem*>&,
                    std::unordered_map<DetailSettingType, IDetailSettingsItem*>&,
                    std::list<AppManagerItem>&,
                    QWidget* = nullptr);

public:
    virtual void initUi() override ;

    virtual void initSlots() override;

    virtual void setValue(int, int) override;

    virtual void setAppValue(const QString&, int) override;

    virtual void setAppMute(const QString&, bool) override;

    virtual void setCurrentIndex(int, int) override;

    virtual void setAppCurrentIndex(const QString&, const NodeType&, int) override;

    virtual void setAppCbxEnabled(const QString&, const NodeType&, bool) override;

    virtual void setCurrentAppItem(int) override;

    virtual void setChecked(int, bool) override;

    virtual std::list<DevicePortInfo> getDevicePortList(int) const override;

    virtual std::list<DevicePortInfo> getDeviceManagetPortList(int) const override;

    virtual std::list<AppManagerItem>& getAppItemList() override;

    virtual std::list<DevicePortInfo> getAppManagetPortList(int, int) const override;

    virtual QWidget* getWidget() override;

    virtual void addDeviceManagerItem(int, const QString&, const QVariant&) override;

    virtual void removeDeviceManagerItem(int, int) override;

    virtual void setDeviceManagerItemStatus(int, int, bool) override;

    virtual void updateDeviceManagerStatus(int, bool) override;

    virtual void updateUIByButtonStatus(int, bool) override;

    virtual void updateUIByDevice(int type, bool status) override;

    virtual void addAppManagerItem(int, int, int, bool, const QString&, const QString&, const QList<ClientDeviceInfo>&, const QList<ClientDeviceInfo>&) override;

    virtual void removeAppManagerItem(const QString&) override;

    virtual void addPortToAppItem(const QString&, const NodeType&, const QString&, const QVariant&) override;

    virtual void removePortFromAppItem(const QString&, const NodeType&, int) override;

    virtual void showAppControlWidget() override;

private:
    void createdItems();

    QFrame* addLine();

    void setFrameVisible(int index, bool visible);

private:
    DeviceManagerMainWidget* m_pDevManagerWidget = nullptr;
//    AppManagerMainWidget* m_pAppManagerWidget = nullptr;
    DesktopEnvironmentVersion m_type = DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI2;

    std::unordered_map<TitleLabelType, TitleLabelItem*>& m_titleLabelItemMap;
    std::unordered_map<SwitchButtonType, ISwitchButtonItem*>& m_switchBtnItemMap;
    std::unordered_map<VolumeSliderType, IVolumeSliderItem*>& m_volumeSliderItemMap;
    std::unordered_map<SelectComboxType, ISelectComboxItem*>& m_selectComboxItemMap;
    std::unordered_map<DetailSettingType, IDetailSettingsItem*>& m_detailSettingsItemMap;

    std::vector<QFrame*> m_FrameVector;

    std::unordered_map<DetailSettingType, QWidget*> m_detailSettingsWidgetMap;
};

#endif // UKUI5MAINWIDGET_H
