#ifndef COMMON_H
#define COMMON_H

#include <QSize>

#define UKUI_THEME_GSETTING_PATH "org.ukui.style"

static const QString KUM_DEST = "com.kylin.systemupgrade";
static const QString KUM_PATH = "/com/kylin/systemupgrade";
static const QString KUM_IFACE = "com.kylin.systemupgrade.interface";


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
#define TREEWIDGETITEMSIZEHINT QSize(0, 40)

/***********        DeviceItem       ************/
#define UPDATEBTNSIZE QSize(96, 36)
#define PROGRESSBARSIZE QSize(66, 16)

/***********        NoDriverUninstallPage       ************/
#define NoDriverIconSize QSize(96, 96)

#endif // COMMON_H
