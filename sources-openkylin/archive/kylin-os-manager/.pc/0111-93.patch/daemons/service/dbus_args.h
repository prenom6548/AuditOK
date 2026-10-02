#ifndef DBUS_ARGS_H
#define DBUS_ARGS_H

#include <QString>
namespace dbus
{
const QString DBUS_SERVICENAME = QString("com.kylin-os-manager");
//工具
const QString DBUS_PARH_TOOL = QString("/tool");
const QString DBUS_INTERFACE_TOOL = QString("tool.tool");
//驱动信息获取接口
const QString DBUS_DRIVER_INFO_TOOL = QString("/driverinfo");
const QString DBUS_DRIVER_INFO_INTERFACE_TOOL = QString("driverinfo.tool");
//热插拔
const QString DBUS_USB_HOT_INFO_TOOL = QString("/usbhot");
const QString DBUS_USB_HOT_INFO_INTERFACE_TOOL = QString("usbhot.tool");
//系统监控
const QString DBUS_PARH_SYSTEM_MONITOR = QString("/systemmonitor");
const QString DBUS_INTERFACE_SYSTEM_MONITOR = QString("systemmonitor.systemmonitor");
} // namespace dbus
#endif // DBUS_ARGS_H
