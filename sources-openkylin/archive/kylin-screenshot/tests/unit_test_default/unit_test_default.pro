#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
#QT += dbus xml KService KCoreAddons KXmlGui KGlobalAccel
##greaterThan(QT_MAJOR_VERSION, 4):
#QT += widgets concurrent sql xml
#QT -= gui
LIBS += -lavutil -lavformat -lavcodec -lswscale -lX11 -lXext -lXtst -lXfixes -lXinerama -lpulse 
LIBS += -lkysdk-ukuiwindowhelper 
LIBS += -lkywc
TAG_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
isEmpty(TAG_VERSION){
    TAG_VERSION = v0.6.0
}
DEFINES += APP_VERSION=\\\"$$TAG_VERSION\\\"

#DEFINES += ENABLE_RECORD
DEFINES += SUPPORT_UKUI
DEFINES += SUPPORT_NEWUI
DEFINES += SCREENSHOT_LIBRARY
#DEFINES += QT_BUILD_WIDGETS_LIB
DEFINES += UNUI

QT  += core gui widgets network svg x11extras KWindowSystem
QT += testlib
unix:!macx {
    QT  += dbus
}

CONFIG += c++17 link_pkgconfig
PKGCONFIG += gsettings-qt
#PKGCONFIG += opencv4
QMAKE_CXXFLAGS += -std=c++0x -D_FORTIFY_SOURCE=2 -O2
PKGCONFIG += libavutil libavformat libavcodec libswscale x11 xext xfixes xinerama libpulse kysdk-qtwidgets kysdk-waylandhelper
PKGCONFIG += kysdk-datacollect
TEMPLATE = app

TARGET = unit_test_default
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control
#-fno-exceptions

LIBS          += -lgtest_main -lpthread
#-lpolkit-qt5-core-1
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov -ldconf

#OBJECTS_DIR = ./obj
#MOC_DIR = ./moc

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt \
	     kysdk-sysinfo \
	     gio-2.0 \

INCLUDEPATH += ../libukcc/interface/ \
               ../libukcc/ \
		/usr/include/dconf
#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

DEFINES += QT_DEPRECATED_WARNINGS

#include(../../kylin_screenshot/src/third-party/singleapplication/singleapplication.pri)
include(../../kylin_screenshot/src/third-party/Qt-Color-Widgets//color_widgets.pri)

DEFINES += QAPPLICATION_CLASS=QApplication

INCLUDEPATH += ../../kylin_screenshot \
               ../../kylin_screenshot/src/common \
               ../../kylin_screenshot/src/utils \
	       ../../kylin_screenshot/src/widgets/ukui-window-switch-wlcom \
	       ../../kylin_screenshot/src/config \
	       ../../kylin_screenshot/src/core
wlcom_sources = $$files(../../kylin_screenshot/src/widgets/ukui-window-switch-wlcom/*.cpp)	      
SOURCES += $$wlcom_sources	       
SOURCES += \
#    unit_test_default.cpp \
    tst_mytest.cpp \
#    main.cpp \
#     ../../kylin_screenshot/src/main.cpp \
#    ../../kylin_screenshot/src/common/CommandLineOptions.cpp \
#    ../../kylin_screenshot/src/common/Logger.cpp \
#    ../../kylin_screenshot/src/core/kabuttonproxystyle.cpp \
#    ../../kylin_screenshot/src/core/kagroupbutton.cpp \
    ../../kylin_screenshot/src/tools/AbstractActionToolWithWidget.cpp \
    ../../kylin_screenshot/src/utils/mysavedialog.cpp \
    ../../kylin_screenshot/src/widgets/aboutdialog.cpp \
    ../../kylin_screenshot/src/widgets/capture/buttonhandler.cpp \
    ../../kylin_screenshot/src/widgets/capture/font_options.cpp \
    ../../kylin_screenshot/src/widgets/capture/font_options2.cpp \
    ../../kylin_screenshot/src/widgets/delaywidget.cpp \
    ../../kylin_screenshot/src/widgets/infowidget.cpp \
    ../../kylin_screenshot/src/config/configwindow.cpp \
    ../../kylin_screenshot/src/widgets/capture/capturewidget.cpp \
    ../../kylin_screenshot/src/widgets/capture/captureshow.cpp \
    ../../kylin_screenshot/src/widgets/capture/colorpicker.cpp \
    ../../kylin_screenshot/src/config/buttonlistview.cpp \
    ../../kylin_screenshot/src/config/uicoloreditor.cpp \
    ../../kylin_screenshot/src/config/geneneralconf.cpp \
    ../../kylin_screenshot/src/core/controller.cpp \
    ../../kylin_screenshot/src/config/clickablelabel.cpp \
    ../../kylin_screenshot/src/config/filenameeditor.cpp \
    ../../kylin_screenshot/src/config/strftimechooserwidget.cpp \
    ../../kylin_screenshot/src/widgets/capture/capturebutton.cpp \
    ../../kylin_screenshot/src/tools/pencil/penciltool.cpp \
    ../../kylin_screenshot/src/tools/undo/undotool.cpp \
#    ../../kylin_screenshot/src/tools/redo/redotool.cpp \
    ../../kylin_screenshot/src/tools/arrow/arrowtool.cpp \
    ../../kylin_screenshot/src/tools/circle/circletool.cpp \
    ../../kylin_screenshot/src/tools/copy/copytool.cpp \
    ../../kylin_screenshot/src/tools/exit/exittool.cpp \
#    ../../kylin_screenshot/src/tools/imgur/imguruploadertool.cpp \
    ../../kylin_screenshot/src/tools/line/linetool.cpp \
    ../../kylin_screenshot/src/tools/marker/markertool.cpp \
#    ../../kylin_screenshot/src/tools/move/movetool.cpp \
#    ../../kylin_screenshot/src/tools/rectangle/rectangletool.cpp \
    ../../kylin_screenshot/src/tools/save/savetool.cpp \
    ../../kylin_screenshot/src/tools/selection/selectiontool.cpp \
#    ../../kylin_screenshot/src/tools/sizeindicator/sizeindicatortool.cpp \
    ../../kylin_screenshot/src/tools/toolfactory.cpp \
    ../../kylin_screenshot/src/utils/filenamehandler.cpp \
    ../../kylin_screenshot/src/utils/screengrabber.cpp \
    ../../kylin_screenshot/src/utils/confighandler.cpp \
    ../../kylin_screenshot/src/utils/systemnotification.cpp \
    ../../kylin_screenshot/src/cli/commandlineparser.cpp \
    ../../kylin_screenshot/src/cli/commandoption.cpp \
    ../../kylin_screenshot/src/cli/commandargument.cpp \
    ../../kylin_screenshot/src/utils/screenshotsaver.cpp \
#    ../../kylin_screenshot/src/tools/imgur/imguruploader.cpp \
    ../../kylin_screenshot/src/widgets/loadspinner.cpp \
    ../../kylin_screenshot/src/widgets/imagelabel.cpp \
    ../../kylin_screenshot/src/widgets/notificationwidget.cpp \
    ../../kylin_screenshot/src/widgets/capture/notifierbox.cpp \
    ../../kylin_screenshot/src/utils/desktopinfo.cpp \
#    ../../kylin_screenshot/src/tools/launcher/applauncherwidget.cpp \
    ../../kylin_screenshot/src/utils/desktopfileparse.cpp \
#    ../../kylin_screenshot/src/tools/launcher/launcheritemdelegate.cpp \
    ../../kylin_screenshot/src/tools/blur/blurtool.cpp \
    ../../kylin_screenshot/src/tools/pin/pintool.cpp \
#    ../../kylin_screenshot/src/tools/launcher/terminallauncher.cpp \
    ../../kylin_screenshot/src/config/visualseditor.cpp \
    ../../kylin_screenshot/src/config/extendedslider.cpp \
#    ../../kylin_screenshot/src/tools/launcher/openwithprogram.cpp \
#    ../../kylin_screenshot/src/tools/launcher/applaunchertool.cpp \
    ../../kylin_screenshot/src/utils/pathinfo.cpp \
    ../../kylin_screenshot/src/utils/colorutils.cpp \
    ../../kylin_screenshot/src/tools/capturecontext.cpp \
    ../../kylin_screenshot/src/widgets/capture/modificationcommand.cpp \
    ../../kylin_screenshot/src/tools/abstractpathtool.cpp \
    ../../kylin_screenshot/src/tools/abstracttwopointtool.cpp \
    ../../kylin_screenshot/src/tools/abstractactiontool.cpp \
    ../../kylin_screenshot/src/utils/globalvalues.cpp \
    ../../kylin_screenshot/src/widgets/panel/utilitypanel.cpp \
    ../../kylin_screenshot/src/widgets/capture/hovereventfilter.cpp \
    ../../kylin_screenshot/src/widgets/capture/selectionwidget.cpp \
    ../../kylin_screenshot/src/tools/pin/pinwidget.cpp \
    ../../kylin_screenshot/src/widgets/capturelauncher.cpp \
    ../../kylin_screenshot/src/tools/text/texttool.cpp \
    ../../kylin_screenshot/src/tools/text/textwidget.cpp \
    ../../kylin_screenshot/src/core/capturerequest.cpp \
    ../../kylin_screenshot/src/tools/text/textconfig.cpp \
    ../../kylin_screenshot/src/widgets/panel/sidepanelwidget.cpp \
    ../../kylin_screenshot/src/widgets/capture/fontsize_color_chose.cpp \
    ../../kylin_screenshot/src/widgets/capture/fontsize_color_chose2.cpp \
    ../../kylin_screenshot/src/widgets/capture/save_location.cpp \
    ../../kylin_screenshot/src/widgets/capture/save_location2.cpp \
    ../../kylin_screenshot/src/tools/options/options.cpp \
    ../../kylin_screenshot/src/tools/save/saveastool.cpp \
#    ../../kylin_screenshot/src/widgets/screentype.cpp \
    ../../kylin_screenshot/src/widgets/shortcutwidget.cpp \
    ../../kylin_screenshot/src/widgets/xatomhelper.cpp \
    ../../kylin_screenshot/src/config/movelabel.cpp \
    ../../kylin_screenshot/src/tools/ocr/ocrtool.cpp \
    ../../kylin_screenshot/src/tools/scroll/scroll_tool.cpp
wlcom_headers = $$files(../../kylin_screenshot/src/widgets/ukui-window-switch-wlcom/*.h) 
HEADERS += $$wlcom_headers
HEADERS  += ../../kylin_screenshot/src/common/Global.h ../../kylin_screenshot/src/common/my_qt.h ../../kylin_screenshot/src/common/my_x.h ../../kylin_screenshot/src/common/my_av.h ../../kylin_screenshot/src/widgets/capture/buttonhandler.h \
#    ../../kylin_screenshot/src/common/CommandLineOptions.h \
    ../../kylin_screenshot/src/common/Enum.h \
#    ../../kylin_screenshot/src/common/Logger.h \
#    ../../kylin_screenshot/src/core/kabuttonproxystyle.h \
#    ../../kylin_screenshot/src/core/kagroupbutton.h \
    ../../kylin_screenshot/src/tools/AbstractActionToolWithWidget.h \
    ../../kylin_screenshot/src/utils/mysavedialog.h \
    ../../kylin_screenshot/src/widgets/aboutdialog.h \
    ../../kylin_screenshot/src/widgets/capture/font_options.h \
    ../../kylin_screenshot/src/widgets/capture/font_options2.h \
    ../../kylin_screenshot/src/widgets/delaywidget.h \
    ../../kylin_screenshot/src/widgets/infowidget.h \
    ../../kylin_screenshot/src/config/configwindow.h \
    ../../kylin_screenshot/src/widgets/capture/capturewidget.h \
    ../../kylin_screenshot/src/widgets/capture/captureshow.h \
    ../../kylin_screenshot/src/widgets/capture/colorpicker.h \
    ../../kylin_screenshot/src/config/buttonlistview.h \
    ../../kylin_screenshot/src/config/uicoloreditor.h \
    ../../kylin_screenshot/src/config/geneneralconf.h \
    ../../kylin_screenshot/src/config/clickablelabel.h \
    ../../kylin_screenshot/src/config/filenameeditor.h \
    ../../kylin_screenshot/src/utils/filenamehandler.h \
    ../../kylin_screenshot/src/config/strftimechooserwidget.h \
    ../../kylin_screenshot/src/utils/screengrabber.h \
    ../../kylin_screenshot/src/tools/capturetool.h \
    ../../kylin_screenshot/src/widgets/capture/capturebutton.h \
    ../../kylin_screenshot/src/tools/pencil/penciltool.h \
    ../../kylin_screenshot/src/tools/undo/undotool.h \
#    ../../kylin_screenshot/src/tools/redo/redotool.h \
    ../../kylin_screenshot/src/tools/arrow/arrowtool.h \
    ../../kylin_screenshot/src/tools/circle/circletool.h \
    ../../kylin_screenshot/src/tools/copy/copytool.h \
    ../../kylin_screenshot/src/tools/exit/exittool.h \
#    ../../kylin_screenshot/src/tools/imgur/imguruploadertool.h \
    ../../kylin_screenshot/src/tools/line/linetool.h \
    ../../kylin_screenshot/src/tools/marker/markertool.h \
#    ../../kylin_screenshot/src/tools/move/movetool.h \
#    ../../kylin_screenshot/src/tools/rectangle/rectangletool.h \
    ../../kylin_screenshot/src/tools/save/savetool.h \
    ../../kylin_screenshot/src/tools/selection/selectiontool.h \
#    ../../kylin_screenshot/src/tools/sizeindicator/sizeindicatortool.h \
    ../../kylin_screenshot/src/tools/toolfactory.h \
    ../../kylin_screenshot/src/utils/confighandler.h \
    ../../kylin_screenshot/src/core/controller.h \
    ../../kylin_screenshot/src/utils/systemnotification.h \
    ../../kylin_screenshot/src/cli/commandlineparser.h \
    ../../kylin_screenshot/src/cli/commandoption.h \
    ../../kylin_screenshot/src/cli/commandargument.h \
    ../../kylin_screenshot/src/utils/screenshotsaver.h \
#    ../../kylin_screenshot/src/tools/imgur/imguruploader.h \
    ../../kylin_screenshot/src/widgets/loadspinner.h \
    ../../kylin_screenshot/src/widgets/imagelabel.h \
    ../../kylin_screenshot/src/widgets/notificationwidget.h \
    ../../kylin_screenshot/src/widgets/capture/notifierbox.h \
    ../../kylin_screenshot/src/utils/desktopinfo.h \
#    ../../kylin_screenshot/src/tools/launcher/applauncherwidget.h \
    ../../kylin_screenshot/src/utils/desktopfileparse.h \
#    ../../kylin_screenshot/src/tools/launcher/launcheritemdelegate.h \
    ../../kylin_screenshot/src/tools/blur/blurtool.h \
    ../../kylin_screenshot/src/tools/pin/pintool.h \
    ../../kylin_screenshot/src/tools/launcher/terminallauncher.h \
    ../../kylin_screenshot/src/config/visualseditor.h \
    ../../kylin_screenshot/src/config/extendedslider.h \
#    ../../kylin_screenshot/src/tools/launcher/openwithprogram.h \
    ../../kylin_screenshot/src/utils/pathinfo.h \
    ../../kylin_screenshot/src/tools/capturecontext.h \
#    ../../kylin_screenshot/src/tools/launcher/applaunchertool.h \
    ../../kylin_screenshot/src/utils/globalvalues.h \
    ../../kylin_screenshot/src/utils/colorutils.h \
    ../../kylin_screenshot/src/widgets/capture/modificationcommand.h \
    ../../kylin_screenshot/src/tools/abstractpathtool.h \
    ../../kylin_screenshot/src/tools/abstracttwopointtool.h \
    ../../kylin_screenshot/src/tools/abstractactiontool.h \
    ../../kylin_screenshot/src/widgets/panel/utilitypanel.h \
    ../../kylin_screenshot/src/widgets/capture/hovereventfilter.h \
    ../../kylin_screenshot/src/widgets/capture/selectionwidget.h \
    ../../kylin_screenshot/src/tools/pin/pinwidget.h \
    ../../kylin_screenshot/src/widgets/capturelauncher.h \
    ../../kylin_screenshot/src/tools/text/texttool.h \
    ../../kylin_screenshot/src/tools/text/textwidget.h \
    ../../kylin_screenshot/src/core/capturerequest.h \
    ../../kylin_screenshot/src/tools/text/textconfig.h \
    ../../kylin_screenshot/src/widgets/panel/sidepanelwidget.h \
    ../../kylin_screenshot/src/widgets/capture/fontsize_color_chose.h \
    ../../kylin_screenshot/src/widgets/capture/fontsize_color_chose2.h \
    ../../kylin_screenshot/src/widgets/capture/save_location.h \
    ../../kylin_screenshot/src/widgets/capture/save_location2.h \
    ../../kylin_screenshot/src/tools/options/options.h \
    ../../kylin_screenshot/src/tools/save/saveastool.h \
#    ../../kylin_screenshot/src/widgets/screentype.h \
    ../../kylin_screenshot/src/widgets/shortcutwidget.h \
    ../../kylin_screenshot/src/widgets/xatomhelper.h \ 
    ../../kylin_screenshot/src/config/movelabel.h \
     ../../kylin_screenshot/src/tools/ocr/ocrtool.h \
     ../../kylin_screenshot/src/tools/scroll/scroll_tool.h

unix:!macx {
    SOURCES += ../../kylin_screenshot/src/core/flameshotdbusadapter.cpp \
        ../../kylin_screenshot/src/utils/dbusutils.cpp

    HEADERS  += ../../kylin_screenshot/src/core/flameshotdbusadapter.h \
        ../../kylin_screenshot/src/utils/dbusutils.h
}

# installs
unix:!macx {
    isEmpty(PREFIX) {
      packaging {
        PREFIX = /usr
      } else {
        PREFIX = /usr/local
      }
    }

    DEFINES += APP_PREFIX=\\\"$$PREFIX\\\"

    target.path = $${PREFIX}/bin/

    qmfile.path = $${PREFIX}/share/kylin-screenshot/translations/
    qmfile.files = $${TRANSLATIONS_FILES}

    dbus.path = $${PREFIX}/share/dbus-1/interfaces/
    dbus.files = dbus/org.dharkael.kylinscreenshot.xml

    gsettings.path = $${PREFIX}/share/glib-2.0/schemas/
    gsettings.files =  gsettings/org.ukui.screenshot.gschema.xml

    icon.path = $${PREFIX}/share/icons/hicolor/
    icon.files = img/hicolor/*

    completion.path = $${PREFIX}/share/bash-completion/completions/
    completion.files = docs/bash-completion/kylin-screenshot

    appdata.path = $${PREFIX}/share/metainfo/
    appdata.files = docs/appdata/kylinscreenshot.appdata.xml

    desktopentry.path = $${PREFIX}/share/applications
    desktopentry.files = docs/desktopEntry/package/kylin-screenshot.desktop

    helpfiles.path = $${PREFIX}/share/kylin-user-guide/data/guide/
    helpfiles.files = data/kylin-screenshot/

    servicedbus.path = $${PREFIX}/share/dbus-1/services/

    packaging {
        servicedbus.files = dbus/package/org.dharkael.kylinscreenshot.service
    } else {
        servicedbus.files = dbus/make/org.dharkael.kylinscreenshot.service
    }

    INSTALLS += target \
        icon \
        desktopentry \
        qmfile \
        gsettings \
        servicedbus \
        dbus \
        completion \
        helpfiles \
        appdata
}

RESOURCES += \
    ../../kylin_screenshot/graphics.qrc

# Imgur API data
include(../../kylin_screenshot/src/imgur.pri)
