#ifndef DBUS_ARGS_H
#define DBUS_ARGS_H

#include <QString>
namespace dbus
{
const QString DBUS_SERVICENAME = QString("com.kylin-os-manager");
//工具
const QString DBUS_PARH_TOOL = QString("/tool");
const QString DBUS_INTERFACE_TOOL = QString("tool.tool");
//系统监控
const QString DBUS_PARH_SYSTEM_MONITOR = QString("/systemmonitor");
const QString DBUS_INTERFACE_SYSTEM_MONITOR = QString("systemmonitor.systemmonitor");
//数据迁移（rsync）
const QString DBUS_PARH_DATA_MIGRATION = QString("/datamigration");
const QString DBUS_INTERFACE_DATA_MIGRATION = QString("datamigration.datamigration");
} // namespace dbus
#endif // DBUS_ARGS_H
