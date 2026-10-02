#include <QGSettings>
#include <QSettings>
#include "config.h"
#include "error.h"
#include "brightness.h"

Config::Config(QObject *parent) : QObject(parent), m_gsettings(new QGSettings((QByteArray)POWER_MANAGER_SETTINGS))
{
    if (isNeedConfig()) {
        initConfig();
    }

}

Config::~Config() {}

void Config::setGsettings(const QString &key, const QVariant &value)
{
    m_gsettings->set(key,value);
}

QVariant Config::getGsettings(const QString &key)
{
    return m_gsettings->get(key);;
}

bool Config::isNeedConfig()
{
    if (m_gsettings->get(INIT_POWER_MANAGER).toBool()) {
        qDebug() << "power manager need init config";
        return true;
    } else {
        qDebug() << "power manager need't init config";
        return false;
    }
}

void Config::initConfig()
{
    int ret(0);
    QStringList str;
    QString proName;
    QSettings Settings("/etc/ukui/power/power-manager.conf", QSettings::NativeFormat);
    if (Settings.status() != QSettings::NoError) {
        ret = CONFIG_QSET_INIT_ERROR;
        qCritical() << "error code: " << ret;
        return;
    }
    proName = getProduct();
    Settings.beginGroup("product");
    str = Settings.childKeys();
    Settings.endGroup();
    if (-1 == str.indexOf(proName)) {
        qDebug() << "vendor special treatment";
        vendorSpecialTreatment();
    } else {
        qDebug() << "produce special treatement";
        Settings.beginGroup(proName);
        str = Settings.childKeys();
        for (int var = 0; var < str.size(); ++var) {
            setGsettings(str.at(var), Settings.value(str.at(var)));
        }
        Settings.endGroup();
    }
    qDebug() << "Config" << SUCCESS;
}

QString Config::getProduct()
{
    int ret(0);
    QString name("null");
    QFile file("/sys/class/dmi/id/product_name");
    if (!file.open(QIODevice::ReadOnly)) {
        ret = CONFIG_GET_CONF_DATA_ERROR;
        qCritical() << "error code: " << ret << ", msg: " << file.errorString();
        return name;
    }
    QByteArray data = file.readAll();
    file.close();
    name.prepend(data);
    name = name.simplified();
    qDebug() << "product name is :" << name;
    return name;
}

int Config::getManufacturer()
{
    int ret(0);
    QString vendor;
    QFile file("/sys/class/dmi/id/board_vendor");
    if (!file.open(QIODevice::ReadOnly)) {
        ret = CONFIG_VENDOR_GET_ERROR;
        qCritical() << "error code: " << ret << ", msg: " << file.errorString();
        return Unknown;
    }
    QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty()) {
        ret = CONFIG_GET_CONF_DATA_ERROR;
        qCritical() << "error code: " << ret;
        return Unknown;
    }
    vendor.prepend(data);
    vendor = vendor.simplified();
    return vendorTypeMatch(vendor);
}

int Config::vendorTypeMatch(const QString &type)
{
    int vendor;
    if (QString("Greatwall") == type) {
        vendor = Greatwall;
    } else {
        vendor = Unknown;
    }
    qDebug() << "Manufacturer is :" << vendor;
    return vendor;
}

void Config::vendorSpecialTreatment()
{
    switch (getManufacturer()) {
    case Greatwall:
        setGsettings(POWER_POLICY_BATTERY, 1);
        break;
    default:
        break;
    }
}
