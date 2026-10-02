#ifndef UKCC2MAINWIDGET_H
#define UKCC2MAINWIDGET_H

#include "IAudioMainWidow.h"
#include "Ukui2SwitchButtonItem.h"
#include "Ukui2VolumeSliderItem.h"
#include "Ukui2SelectComboxItem.h"
#include "AppManagerMainWidget.h"
#include "Util.h"

class Ukcc2MainWidget : public QWidget, public IAudioMainWindow
{
public:
    Ukcc2MainWidget(std::unordered_map<TitleLabelType, TitleLabelItem*>&,
                    std::unordered_map<SwitchButtonType, ISwitchButtonItem*>&,
                    std::unordered_map<VolumeSliderType, IVolumeSliderItem*>&,
                    std::unordered_map<SelectComboxType, ISelectComboxItem*>&,
                    std::unordered_map<DetailSettingType, IDetailSettingsItem*>&,
                    std::list<AppManagerItem>&,
                    QWidget* = nullptr);
    ~Ukcc2MainWidget() = default;

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

    virtual void addAppManagerItem(int, int, int, bool, const QString&, const QString&, const QList<ClientDeviceInfo>&, const QList<ClientDeviceInfo>&) override;

    virtual void removeAppManagerItem(const QString&) override;

    virtual void addPortToAppItem(const QString&, const NodeType&, const QString&, const QVariant&) override;

    virtual void removePortFromAppItem(const QString&, const NodeType&, int) override;

    virtual void showAppControlWidget() override;

private:
    void createdItems();

    QFrame* addLine();

private:
    std::list<AppManagerItem> m_appWidgetList {};

    std::unordered_map<TitleLabelType, TitleLabelItem*>& m_titleLabelItemMap;
    std::unordered_map<SwitchButtonType, ISwitchButtonItem*>& m_switchBtnItemMap;
    std::unordered_map<VolumeSliderType, IVolumeSliderItem*>& m_volumeSliderItemMap;
    std::unordered_map<SelectComboxType, ISelectComboxItem*>& m_selectComboxItemMap;
    std::unordered_map<DetailSettingType, IDetailSettingsItem*>& m_detailSettingsItemMap;
};

#endif // UKCC2MAINWIDGET_H
