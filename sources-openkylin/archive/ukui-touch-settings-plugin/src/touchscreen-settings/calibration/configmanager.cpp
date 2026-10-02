#include "configmanager.h"

#include <QDebug>
#include <QCryptographicHash>

ConfigManager::ConfigManager(QString filepath)
    : m_config(filepath, QSettings::IniFormat)
{

}

void ConfigManager::saveConfigInfo(QString outputName, const TouchDeviceConfigInfo& touchDevice)
{
    int mapId = findMapId(touchDevice);
    if (mapId == -1) {
        mapId = createNewMapId();
    }

    QString mapName = QString("/MAP%1/").arg(mapId);

    m_config.setValue(QString(mapName + "name"), touchDevice.deviceName);
    m_config.setValue(QString(mapName + "productId"), touchDevice.productId);
    m_config.setValue(QString(mapName + "serial"), touchDevice.serial);
    m_config.setValue(QString(mapName + "scrname"), outputName);

    qDebug() << "Save config:" << m_config.fileName() << mapName
             << touchDevice.deviceName << touchDevice.productId << touchDevice.serial
             << outputName;

    updateMapCount();
}

int ConfigManager::createNewMapId()
{
    int mapCount = m_config.value("COUNT/num").toInt();

    return ++mapCount;
}

int ConfigManager::findMapId(const TouchDeviceConfigInfo& touchDevice)
{
    int mapCount = m_config.value("COUNT/num").toInt();
    for (int i = 0; i < mapCount; ++i) {
        QString mapName = QString("/MAP%1/").arg(i + 1);
        TouchDeviceConfigInfo cfgDevice;
        cfgDevice.deviceName = m_config.value(QString(mapName + "name")).toString();
        cfgDevice.productId = m_config.value(QString(mapName + "productId")).toString();
        cfgDevice.serial = m_config.value(QString(mapName + "serial")).toString();
        if (cfgDevice == touchDevice) {
            qDebug() << "Touch device:" << touchDevice.deviceName << touchDevice.productId
                     << "has been saved in config fill, map id is" << i + 1
                     << "current map to:" << m_config.value(QString(mapName + "scrname")).toString();
            return i + 1;
        }
    }

    return -1;
}

void ConfigManager::updateMapCount()
{
    QStringList groupList = m_config.childGroups();
    int count = 0;
    for (const QString& group : groupList) {
        if (group.contains("MAP"))
            ++count;
    }

    m_config.setValue("COUNT/num", count);
}
