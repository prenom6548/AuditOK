VERSION = 3.2.2
DEFINES += VERSION='\\"$${VERSION}\\"'
DEFINES += MULTI_DISABLE

DEFINES += V10_SP1='\\"V10SP1\\"'
DEFINES += V10_SP1_EDU='\\"V10SP1-edu\\"'

DEB_VERSION = 3.14.5.7
DEFINES += DEB_VERSION='\\"$${DEB_VERSION}\\"'

DEFINES += WIDGET_MARGIN=10
DEFINES += WIDGET_SPACING=4
#define WIDGET_MARGIN 10
#define WIDGET_SPACING 4

exists(/usr/include/ukuisdk/kylin-com4cxx.h) {
    message("kylin common for cxx find.")
    DEFINES += KYLIN_COMMON=true
    LIBS += -lukui-com4cxx
}

exists(/usr/include/kysdk/applications/kdialog.h) {
    message("kyfiledialog find")
    DEFINES += KY_FILE_DIALOG
}

exists(/usr/include/kysdk/kysdk-system/libkysysinfo.h) {
    message("kysysinfo find")
    DEFINES += KY_SDK_SYSINFO
}

exists(/usr/include/kysdk/applications/kaboutdialog.h) {
    message("kyqtwidgets find")
    DEFINES += KY_SDK_QT_WIDGETS
}

exists(/usr/include/kysdk/applications/ukuistylehelper/ukuistylehelper.h) {
    message("kywaylandhelper find")
    DEFINES += KY_SDK_WAYLANDHELPER
}

exists("/usr/include/libkyudfburn/udfburn_global.h") {
    DEFINES += KY_UDF_BURN
}

exists("/usr/include/ukui-search/libsearch_global.h") {
    DEFINES += KY_UKUI_SEARCH
}

exists("/usr/include/kysdk/desktop/kysdk-soundeffects_global.h") {
    DEFINES += KY_SDK_SOUND_EFFECTS
    PKGCONFIG += kysdk-soundeffects
}

exists("/usr/include/kysdk/kysdk-system/libkydate.h") {
    DEFINES += KY_SDK_DATE
    PKGCONFIG += kysdk-systime
}

exists(/usr/include/kysdk/kysdk-system/libkysysinfo.h) {
    message("kysysinfo find")
    DEFINES += KY_SDK_SYSINFO
}

exists(/usr/include/kysdk/diagnosetest/libkydatacollect.h) {
    message("diagnosetest find")
    DEFINES += KY_SDK_DATACOLLECT
}
exists(/usr/include/ukui-log4qt.h) {
    message ("ukui-log4qt found")
    DEFINES += UKUI_LOG4QT
} else:exists(/usr/include/kysdk/kysdk-base/libkylog.h) {
    message ("kysdk log found")
    DEFINES += KY_SDK_KABASE
}

exists(/usr/include/kysdk/kysdk-system/libkypackages.h) {
    message("kypackages find")
    DEFINES += KY_SDK_PACKAGES
}

exists(/usr/include/kysdk/kysdk-system/libkydiskinfo.h) {
    message("kydiskinfo find")
    DEFINES += KY_SDK_DISKINFO
}

exists(/usr/include/kysdk/kysdk-system/libkydiskinfo.h) {
    message("kydiskinfo find")
    DEFINES += KY_SDK_DISKINFO
}

USE_SET_ICON=$$(USE_SET_ICON_GEOMETRY)
equals(USE_SET_ICON,1) {
    message("USE_SET_ICON_GEOMETRY is set")
    DEFINES += KSTARTUPINFO_HAS_SET_ICON_GEOMETRY
} else {
    message("USE_SET_ICON_GEOMETRY is not set")
}

TEST_DESKTOP_WIDGETS=$$(TEST_DESKTOP_WIDGETS)
equals(TEST_DESKTOP_WIDGETS,1) {
    message("TEST_DESKTOP_WIDGETS is set")
    DEFINES += TEST_PEONY_DESKTOP_WIDGETS
} else {
    message("TEST_DESKTOP_WIDGETS is not set")
}

BUILD_WITH_SDK_FEEDBACK_MENU_ACTION=$$(USE_FEEDBACK_MENU_ACTION)
equals(BUILD_WITH_SDK_FEEDBACK_MENU_ACTION, 1) {
    DEFINES += BUILD_WITH_FEEDBACK_ACTION
} else {
    message("do not build with kdk feedback menu action")
}

#DEFINES += VFS_CUSTOM_PLUGIN
