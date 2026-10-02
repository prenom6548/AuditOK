#ifndef UKMEDIAMAINWIDGET_H
#define UKMEDIAMAINWIDGET_H

#include <memory>
#include <QWidget>
#include <QObject>
#include <QDBusVariant>

#include "IMainWindow.h"
#include "Ukui2TrayIcon.h"
#include "Ukui4TrayIcon.h"
#include "ukui4.0/Ukui4MainWindow.h"
#include "ukui4.0/Ukui4SystemVolumeWidget.h"
#include "ukui4.0/Ukui4AppVolumeWidget.h"
#include "ITrayVolumeSliderItem.h"
#include "IDevicePortSelectItem.h"
#include "TrayClientManager.h"
#include "Ukui2MainWindow.h"

enum class PanelPosition
{
    PANEL_POSITION_BOTTOM = 0, //!< The bottom side of the screen.
    PANEL_POSITION_TOP,        //!< The top side of the screen.
    PANEL_POSITION_LEFT,       //!< The left side of the screen.
    PANEL_POSITION_RIGHT       //!< The right side of the screen.
};

using namespace UkuiAudioFramwork;

class UkmediaMainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UkmediaMainWidget(QWidget* = nullptr);
    ~UkmediaMainWidget() {}

public:
    DesktopEnvironmentVersion getDesktopEnvironmentVersion();

    int getVolume(int, const QString&);

    void setVolume(int, const QString&, int);

    bool getMute(int, const QString&);

    void setMute(int, const QString&, bool);

    ClientDefaultDeviceInfo getDefaultDevice(int, const QString&) const;

    void setDefaultDevice(int, const QString&, const QString&);

private Q_SLOTS:
    void volumeChangedSlots(int, int, const QDBusVariant&);

    void deviceChangedSlots(int, const QString&, const QString&);

    void deviceAdjustSlots(int);

    void settingsChangedSlots(const QString&, const QDBusVariant&);

    void addStreamSlots(int, int, const QString&, const QString&);

    void removeStreamSlots(int);

private:
    void initUi();

    void initSystemVolumeSlider();

    void initSystemDeviceList();

    void initCurrentDefaultDevice();

    void initAppVolumeList();

    void initAppVolumeSlider();

    void initData();

    void initSlots();

    void initClientManager();

    QRect caculatePosition();

    void removeHeader(QWidget*);

private:

    int m_panelSize = 40;
    PanelPosition m_panelPosition = PanelPosition::PANEL_POSITION_BOTTOM;
    DesktopEnvironmentVersion m_version = DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI2;
    std::shared_ptr<ISoundSystemTrayIcon> m_pTrayIcon = nullptr;
    IMainWindow* m_pMainWid = nullptr;
    std::shared_ptr<TrayClientManager> m_pClientManager = nullptr;

    std::unordered_map<int, std::shared_ptr<IDevicePortItem>> m_devicePortItemMap {};
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>> m_sysVolumeSliderItemMap {};
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>> m_appVolumeSliderItemMap {};
};

#endif // UKMEDIAMAINWIDGET_H
