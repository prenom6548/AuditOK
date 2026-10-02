#ifndef UKUI2MAINWINDOW_H
#define UKUI2MAINWINDOW_H

#include <QWidget>
#include <QStackedWidget>

#include "IMainWindow.h"
#include "ITrayVolumeSliderItem.h"
#include "ISoundSystemTrayIcon.h"
#include "Ukui2SystemVolumeSliderItem.h"
#include "Ukui2MiniWidget.h"


enum class StackedWidgetType
{
    MINI_STACKED_WIDGET,
    ADVANCE_SYS_WIDGET,
    ADVANCE_APP_WIDGET
};

class Ukui2MainWindow : public QWidget, public IMainWindow
{
    Q_OBJECT
public:
    explicit Ukui2MainWindow(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                             std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                             QWidget* = nullptr);
    ~Ukui2MainWindow() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual void addAppItem(int, const QString&, const QString&) override;

    virtual void removeAppItem(int) override;

    virtual void addAppToWidget(const QString&, const QString&) override;

    virtual void removeAppFromWidget(const QString&) override;

    virtual void addDevToWidget(const QString&, const ClientDeviceInfo&) override;

    virtual void removeDevFromWidget(const QString&, const QString&) override;

    virtual void setCurrentIndex(int) override;

    virtual void setParentWidget(QWidget*) override;

    virtual void setValue(int, int, int) override;

    virtual void updateDescription(const QString&, const QString&) override;

    void setStackWidgetCurrentIndex(int);

Q_SIGNALS:
    void vauleChanged(int);

private Q_SLOTS:
    void siwtchModuleBtnPressedSlots();

    void deviceVolumeBtnPressedSlots();

    void appVolumeBtnPressedSlots();

private:
    QStackedWidget* m_pStackedWidget = nullptr;
    std::shared_ptr<ISoundSystemTrayIcon> m_pTrayIcon = nullptr;
    Ukui2MiniWidget* m_pMiniWidget = nullptr;
    QPushButton* m_pSwitchModuleBtn = nullptr;
    QPushButton* m_pDeviceVolumeBtn = nullptr;
    QPushButton* m_pAppVolumeBtn = nullptr;
    StackedWidgetType m_prevType = StackedWidgetType::ADVANCE_SYS_WIDGET;

    std::unordered_map<VolumeSliderType, QString> m_sysVolumeKeys {
        {VolumeSliderType::VOLUME_SLIDER_MINI_OUTPUT, "mini widget output volume slider"},
        {VolumeSliderType::VOLUME_SLIDER_DEVICE_OUTPUT, "device widget output volume slider"},
        {VolumeSliderType::VOLUME_SLIDER_DEVICE_INPUT, "device widget input volume slider"},
        {VolumeSliderType::VOLUME_SLIDER_APP_OUTPUT, "app widget output volume slider"}
    };
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_sysVolumeSliderItemMap;
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_appSliderItemMap;
};


#endif // UKUI2MAINWINDOW_H
