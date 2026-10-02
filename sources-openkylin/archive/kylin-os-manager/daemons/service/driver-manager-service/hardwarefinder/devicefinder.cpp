#include "devicefinder.h"
#include <QProcess>
#include <QDebug>
#include <libudev.h>
#include <QDebug>
#include <glib.h>

deviceFinder::deviceFinder(QObject *parent) : QObject(parent)
{

}

deviceFinder::~deviceFinder()
{

}

void deviceFinder::getDriverModeInfo(QString driverName, DriverModeInfo &modeinfo)
{
    modeinfo.driverName = driverName;

    QProcess proc;
    QString cmd = QString("modinfo " + driverName);
    proc.start(cmd);
    proc.waitForFinished();
    QStringList outPut = QString(proc.readAllStandardOutput()).split("\n");

    for(int i = 0 ; i < outPut.length(); ++i){
        // 获取ko文件路径
        if(QString(outPut.at(i)).contains("filename:")){
            modeinfo.filename = QString(QString(outPut.at(i)).split("filename:")[1]).trimmed();
            continue;
        }

        // 获取模块描述信息
        if(QString(outPut.at(i)).contains("description:")){
            modeinfo.description = QString(QString(outPut.at(i)).split("description:")[1]).trimmed();
            continue;
        }

        // 获取模块的许可信息
        if(QString(outPut.at(i)).contains("license:")){
            modeinfo.license = QString(QString(outPut.at(i)).split("license:")[1]).trimmed();
            continue;
        }

        // 获取模块作者信息
        if(QString(outPut.at(i)).contains("author:")){
            modeinfo.author = QString(QString(outPut.at(i)).split("author:")[1]).trimmed();
            continue;
        }

        // 获取驱动版本
        if(QString(outPut.at(i)).contains("version:") && !QString(outPut.at(i)).contains("srcversion:")){
            modeinfo.version = QString(QString(outPut.at(i)).split("version:")[1]).trimmed();
            continue;
        }

        // 获取模块的源代码版本
        if(QString(outPut.at(i)).contains("srcversion:")){
            modeinfo.srcversion = QString(QString(outPut.at(i)).split("srcversion:")[1]).trimmed();
            continue;
        }

        // 获取设备别名
        if(QString(outPut.at(i)).contains("alias:")){
            modeinfo.alias.append(QString(QString(outPut.at(i)).split("alias:")[1]).trimmed());
            continue;
        }

        // 获取模块的源代码版本
        if(QString(outPut.at(i)).contains("depends:")){
            modeinfo.depends = QString(QString(outPut.at(i)).split("depends:")[1]).trimmed();
            continue;
        }

        // 获取模块的源代码版本
        if(QString(outPut.at(i)).contains("vermagic:")){
            modeinfo.vermagic = QString(QString(outPut.at(i)).split("vermagic:")[1]).trimmed();
            continue;
        }

        // 获取内核模块的依赖关系
        if(QString(outPut.at(i)).contains("softdep:")){
            modeinfo.softdep = QString(QString(outPut.at(i)).split("softdep:")[1]).trimmed();
            continue;
        }
    }
}


void devpath_from_usb_devaddr (const QString &devaddr, HardWareInfo &info)
{
    if (devaddr.isEmpty()) {
        return ;
    }

    struct udev *udev = nullptr;
    udev = udev_new();

    struct udev_enumerate *udev_enum = nullptr;
    udev_enum = udev_enumerate_new (udev);
    if (udev_enum == nullptr) {
        qDebug() << "udev_enumerate_new failed";
        udev_unref (udev);
        return ;
    }

    if (udev_enumerate_add_match_property (udev_enum, "DEVNAME", qstringTochar(devaddr)) < 0) {
        udev_unref (udev);
        udev_enumerate_unref (udev_enum);
        qDebug() << "udev_enumerate_add_match_property failed";
        return ;
    }

    if (udev_enumerate_scan_devices (udev_enum) < 0) {
        udev_unref (udev);
        udev_enumerate_unref (udev_enum);
        qDebug() << "udev_enumerate_scan_devices failed";
        return ;
    }

    struct udev_list_entry *first = nullptr;
    first = udev_enumerate_get_list_entry (udev_enum);
    if (first == nullptr) {
        udev_unref (udev);
        udev_enumerate_unref (udev_enum);
        qDebug ("no device named %s found", qstringTochar(devaddr));
        return ;
    }

    struct udev_device *device;
    device = udev_device_new_from_syspath (udev, udev_list_entry_get_name (first));
    if (device == nullptr) {
        udev_unref (udev);
        udev_enumerate_unref (udev_enum);
        qDebug ("unable to examine device");
        return ;
    }


    info.serial = QString(udev_device_get_sysattr_value (device, "serial"));
    info.vendor = QString(udev_device_get_sysattr_value (device, "manufacturer"));
    info.model  = QString(udev_device_get_sysattr_value (device, "product"));

    device_id_s id;
    parse_device_id(get_ieee1284_id_from_child (udev, device), &id);
    info.deviceId = id.full_device_id;
    if (info.vendor.isEmpty()) {
        info.vendor = id.mfg;
    }
    if (info.model.isEmpty()) {
        info.model = id.mdl;
    }
    if (info.serial.isEmpty()) {
        info.serial = id.sern;
    }

    specialDeviceCheck(info.vendor, info.model);
}

char* qstringTochar(const QString &qstr)
{
    if (qstr.isEmpty())
        return nullptr;
    QByteArray ba = qstr.toLocal8Bit();
    return ba.data();
}

void specialDeviceCheck(QString &vendor, QString &model)
{
    QMap<QString, QString>::const_iterator iter = mfgConvertMap.begin();
    while (iter != mfgConvertMap.end())
    {
        if (iter.key().compare(vendor, Qt::CaseInsensitive) == 0) {
            vendor = iter.value();
            break;
        }
        iter++;
    }
    if (model.indexOf("HP") == 0) {
        model.remove(0, 2);
        if (model.at(0) == " ") {
            model.remove(0, 1);
        }
    }

    if (model.contains(" series")) {
        model.remove(" series");
    }
}

char *get_ieee1284_id_from_child (struct udev *udev, struct udev_device *parent)
{
    struct udev_enumerate *udev_enum;
    struct udev_list_entry *item, *first = NULL;
    char *device_id = NULL;

    udev_enum = udev_enumerate_new (udev);
    if (!udev_enum) {
        qDebug ("udev_enumerate_new failed");
        exit (1);
    }

    if (udev_enumerate_add_match_parent (udev_enum, parent) < 0) {
        udev_enumerate_unref (udev_enum);
        qDebug ("uname to add parent match");
        exit (1);
    }

    if (udev_enumerate_scan_devices (udev_enum) < 0) {
        udev_enumerate_unref (udev_enum);
        qDebug ("udev_enumerate_scan_devices failed");
        exit (1);
    }

    first = udev_enumerate_get_list_entry (udev_enum);
    udev_list_entry_foreach (item, first) {
        const char *ieee1284_id = NULL;
        struct udev_device *dev;
        dev = udev_device_new_from_syspath (udev, udev_list_entry_get_name (item));
        if (dev == NULL)
            continue;

        ieee1284_id = udev_device_get_sysattr_value (dev, "ieee1284_id");
        if (ieee1284_id)
            device_id = g_strdup (ieee1284_id);

        udev_device_unref (dev);
        if (device_id)
            break;
    }

    udev_enumerate_unref (udev_enum);
    return device_id;
}

void parse_device_id (const char *device_id, struct device_id_s *id)
{
    QString s = QString(device_id);

    if (s.size() == 0)
        return;

    if (s[s.size() - 1] == '\n')
        s.chop(1);


    id->full_device_id = s;

    QStringList ls = s.split(";");

    for (auto ss : ls) {
        if (id->mfg.isEmpty()
            && (ss.indexOf("MANUFACTURER") == 0 || ss.indexOf("MFG") == 0)
            && (ss.contains(":"))) {

            id->mfg = ss.split(":").at(1);
        }

        else if (id->mdl.isEmpty()
                 && (ss.indexOf("MODEL") == 0 || ss.indexOf("MDL") == 0)
                 && (ss.contains(":"))) {

            id->mdl = ss.split(":").at(1);
        }

        else if (id->sern.isEmpty()
                 && (ss.indexOf("SERIALNUMBER") == 0 || ss.indexOf("SERN") == 0 || ss.indexOf("SN") == 0)
                 && (ss.contains(":"))) {

            id->sern = ss.split(":").at(1);
        }

        else if (id->cls.isEmpty()
                 && (ss.indexOf("CLS") == 0 || ss.indexOf("CLASS") == 0)
                 && (ss.contains(":"))) {

            id->cls = ss.split(":").at(1);
        }
    }
    // 特殊打印机处理
    specialDeviceCheck(id->mfg, id->mdl);
    return ;
}
