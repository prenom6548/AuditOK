#ifndef DEVICEMANAGERMAINWIDGET_H
#define DEVICEMANAGERMAINWIDGET_H

#include <QWidget>
#include <kwidget.h>
#include <knavigationbar.h>

#include "DeviceManagerWidget.h"

enum class DeviceManagerType {
    Input = 0,
    Output
};

class DeviceManagerMainWidget : public kdk::KWidget
{
    Q_OBJECT
public:
    DeviceManagerMainWidget(QWidget* = nullptr);
    ~DeviceManagerMainWidget() = default;

public:
    void addItem(const DeviceManagerType&, const QString&, const QString&, const QString&);

    void removeItem(const DeviceManagerType&, int);

    void setChecked(const DeviceManagerType&, int, bool);

    DeviceManagerWidget* getManagerWidgetByType(const DeviceManagerType&);

    void updateDeviceStatus(const DeviceManagerType& type, bool hasDevice);

Q_SIGNALS:
    void enableDeviceSignals(const QString&, const QString&, bool);

    void closeWindowSignals();

private:
    void createItems();

    void initUi();

    void initSlots();

private:
    std::unordered_map<DeviceManagerType, QStringList> m_keys {
        {DeviceManagerType::Output, QStringList(std::initializer_list<QString>{"audio-volume-overamplified-symbolic", QObject::tr("Output")})},
        {DeviceManagerType::Input, QStringList(std::initializer_list<QString>{"ukui-microphone-on-symbolic", QObject::tr("Input")})}
    };

    kdk::KNavigationBar* m_pNavigationBar = nullptr;
    std::unordered_map<DeviceManagerType, QStandardItem*> m_itemMap;
    std::unordered_map<DeviceManagerType, DeviceManagerWidget*> m_deviceWidgetMap;
};

#endif // DEVICEMANAGERMAINWIDGET_H
