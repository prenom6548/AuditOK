#ifndef KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_DEFINES_H
#define KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_DEFINES_H

#include <QString>

namespace tool_box {

static const QString TREASUREBOX_DBUS_NAME = "com.kylin.treasurebox";
static const QString TREASUREBOX_DBUS_PATH = "/com/kylin/treasurebox";
static const QString TREASUREBOX_DBUS_INTERFACE = "com.kylin.treasurebox";

struct DBusAppName {
    QString name;
    QString name_zh_cn;
    QString name_en_us;
};

struct DBusAppDesc {
    QString desc;
    QString desc_zh_cn;
    QString desc_en_us;
};

}

#endif
