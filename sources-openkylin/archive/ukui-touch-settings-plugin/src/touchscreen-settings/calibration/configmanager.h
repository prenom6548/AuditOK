#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QSettings>

struct TouchDeviceConfigInfo {
    QString deviceName;
    QString productId;
    QString serial;

    bool operator==(const TouchDeviceConfigInfo& rhs) {
        if (rhs.deviceName == deviceName && rhs.productId == productId && rhs.serial == serial)
            return true;

        return false;
    }
};

class ConfigManager
{
public:
    ConfigManager(QString filepath);
    void saveConfigInfo(QString outputName, const TouchDeviceConfigInfo& touchDevice);

private:
    int findMapId(const TouchDeviceConfigInfo& touchDevice);
    int createNewMapId();
    void updateMapCount();

    QSettings m_config;
};

#endif // CONFIGMANAGER_H
