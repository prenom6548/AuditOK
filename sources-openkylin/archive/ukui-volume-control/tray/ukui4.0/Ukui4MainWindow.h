#ifndef UKUI4MAINWINDOW_H
#define UKUI4MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>

#include "IMainWindow.h"
#include "ISoundSystemTrayIcon.h"
#include "ukui4.0/Ukui4SystemVolumeWidget.h"
#include "ukui4.0/Ukui4AppVolumeWidget.h"
#include "ITrayVolumeSliderItem.h"
#include "IDevicePortSelectItem.h"
#include "Ukui4SystemVolumeSliderItem.h"


class Ukui4MainWindow : public QWidget, public IMainWindow, public std::enable_shared_from_this<Ukui4MainWindow>
{
    Q_OBJECT
public:
    explicit Ukui4MainWindow(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                             std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                             std::unordered_map<int, std::shared_ptr<IDevicePortItem>>&,
                             QWidget* = nullptr);
    ~Ukui4MainWindow() = default;

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

    virtual void updateDescription(const QString&, const QString&);

Q_SIGNALS:
    void vauleChanged(int);

private:
    QTabWidget* m_pTabWidget = nullptr;
    std::shared_ptr<ISoundSystemTrayIcon> m_pTrayIcon = nullptr;
    std::unordered_map<VolumeSliderType, QString> m_sysVolumeKeys {
        {VolumeSliderType::VOLUME_SLIDER_SYSTEM_OUTPUT, "system widget output volume slider"},
        {VolumeSliderType::VOLUME_SLIDER_APP_OUTPUT, "app widget output volume slider"}
    };

    std::unordered_map<int, std::shared_ptr<IDevicePortItem>>& m_devicePortItemMap;
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_sysVolumeSliderItemMap;
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_appSliderItemMap;
};

#endif // UKUI4MAINWINDOW_H
