#ifndef PLUGINENTRY_H
#define PLUGINENTRY_H

#include <QObject>

#include "common.h"

class TouchScreenWidget;
class DeviceManager;
class PluginEntry : public QObject
{
    Q_OBJECT
public:
    explicit PluginEntry(QObject* parent = nullptr);

    QWidget* widget();
    bool widgetEnable() const;

signals:
    void widgetEnableChanged(bool enaled);

private slots:
    void onDeviceChanged(DeviceType deviceType);

private:
    void initConnection();
    void initWidgetConnection();
    void updateWidgetEnable();
    TouchScreenWidget* createWidget();
    void resetWidgetCombobox(DeviceType deviceType);
    void buriedSettings(const QString& settingsName, const QString& action, const QString& value) const;

    TouchScreenWidget* m_widget;
    DeviceManager* m_deviceManager;
    bool m_widgetEnable;
};

#endif // PLUGINENTRY_H
