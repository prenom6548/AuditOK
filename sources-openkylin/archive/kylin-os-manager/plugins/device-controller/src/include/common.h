#ifndef COMMON_H
#define COMMON_H

#include <QSize>
#define KYLIN_OS_MANAGER "org.kylin-os-manager-data.settings"
#define UKUI_THEME_GSETTING_PATH "org.ukui.style"

static const QString KUM_DEST = "com.kylin.systemupgrade";
static const QString KUM_PATH = "/com/kylin/systemupgrade";
static const QString KUM_IFACE = "com.kylin.systemupgrade.interface";

const QString DBUS_SERVICENAME = QString("com.kylin-os-manager");

//驱动信息获取接口
const QString DBUS_DRIVER_INFO_TOOL = QString("/driverinfo");
const QString DBUS_DRIVER_INFO_INTERFACE_TOOL = QString("driverinfo.tool");

/***********        MainWidget       ************/
#define MAINWIDGETSIZE QSize(824, 600)

/***********        LoadingWidget       ************/
#define PIXMAPSIZE QSize(96, 96)

/***********        SelectionWidget       ************/
#define FUNCTIONBUTTONSIZE QSize(100, 36)

/***********        DriverInstallWidet       ************/
#define ITEMHEIGET 76
#define DEVICEICONBUTTONSIZE QSize(36,36)
#define DEVICEICONSIZE QSize(16,16)

/***********        hardWareInfoWidget       ************/
#define HARDWAREINFOWIDGETBUTTONSIZE QSize(16, 16)
#define TREEITEMLABELWIDTH 148
#define TREEITEMMODELLABELWIDTH 450
#define TREEWIDGETITEMSIZEHINT QSize(0, 40)

/***********        DeviceItem       ************/
#define UPDATEBTNSIZE QSize(96, 36)
#define PROGRESSBARSIZE QSize(66, 16)

/***********        NoDriverUninstallPage       ************/
#define NoDriverIconSize QSize(96, 96)

#endif // COMMON_H
