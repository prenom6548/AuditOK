# QT modules
QT       += \
        core \
        gui \
        KWindowSystem \
        dbus \
        x11extras \
        network \
        printsupport \
        concurrent \
        svg \
        widgets \
        testlib
# 配置kysdk
PKGCONFIG += kysdk-alm kysdk-qtwidgets kysdk-ukenv kysdk-diagnostics kysdk-datacollect kysdk-powermanagement kysdk-waylandhelper kysdk-kabase
INCLUDEPATH += /usr/include/kysdk/applications/kabase/
INCLUDEPATH += ./src/kabase/
INCLUDEPATH += ./src/kabase/Qt/

INCLUDEPATH += \
    $$PWD/src/device/avahi_qt/ \
    $$PWD/src/device/qt_zeroconf/ \
    $$PWD/src/device

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage
LIBS += -lgcov

TARGET = kylin-scanner
TEMPLATE = app

PROJECT_ROOTDIR = $$PWD

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

CONFIG += c++11 \
          link_pkgconfig

PKGCONFIG   += \
        gio-2.0 \
        gio-unix-2.0 \
        gsettings-qt \
        libudev \

LIBS += -lpthread \
        -lX11 \
        -ltiff \
        -ljpeg \
        -lpng \
        -ltesseract \
        -lleptonica \
        -lfreeimage \
        -lusb-1.0 \
        -ludev \
        -lbsd \
        -lpthread \
        -lavahi-common \
        -lavahi-client \
        -lnetsnmp \
        -lnetsnmpagent \
        -lnetsnmpmibs \
        -lsane \
        -lzip \
        -lboundscheck \

INCLUDEPATH += /usr/include/CImg.h
unix:!macx: LIBS += -L$$PWD/../../../usr/lib/x86_64-linux-gnu/
INCLUDEPATH += $$PWD/../../../usr/lib/x86_64-linux-gnu
DEPENDPATH += $$PWD/../../../usr/lib/x86_64-linux-gnu

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/crop.cpp \
    src/customtablewidget.cpp \
    src/detectpagewidget.cpp \
    src/displaywidget.cpp \
    src/failedpagewidget.cpp \
    src/global.cpp \
    src/globalsignal.cpp \
    src/imageBaseOP/loadimage.cpp \
    src/imageBaseOP/savefilebase.cpp \
    src/imageBaseOP/createofd.cpp \
    src/imageOp/imageoperationbase.cpp \
    src/imageOp/imageoperationbeauty.cpp \
    src/imageOp/imageoperationmirror.cpp \
    src/imageOp/imageoperationocr.cpp \
    src/imageOp/imageoperationrectify.cpp \
    src/leftimagehandlesuccesspagewidget.cpp \
    src/leftsuccesspagewidget.cpp \
    src/main.cpp \
    src/mainwidget.cpp \
    src/newdevicelistpage.cpp \
    src/nodevicewidget.cpp \
    src/rectify.cpp \
    src/runningdialog.cpp \
    src/saneobject.cpp \
    src/scandialog.cpp \
    src/scansettingswidget.cpp \
    src/sendmail.cpp \
    src/showimagewidget.cpp \
    src/showocrwidget.cpp \
    src/successpagewidget.cpp \
    src/svghandler.cpp \
    src/thumbnaildelegate.cpp \
    src/thumbnailwidget.cpp \
    src/about/custom_about_dialog.cpp \
    src/titlebar/titlebar.cpp \
    src/toolbarwidget.cpp \
    src/usb.cpp \
    src/usbhotplugthread.cpp \
    src/utils/HorizontalOrVerticalMode.cpp \
    src/utils/rotatechangeinfo.cpp \
    src/utils/scannersettings.cpp \
    src/waittingdialog.cpp \
    src/watermarkdialog.cpp \
    src/custom_push_button.cpp \
    src/navigator.cpp \
    src/include/customlabel.cpp \
    test/mytest.cpp

HEADERS += \
    src/crop.h \
    src/customtablewidget.h \
    src/detectpagewidget.h \
    src/displaywidget.h \
    src/failedpagewidget.h \
    src/global.h \
    src/globalsignal.h \
    src/imageBaseOP/loadimage.h \
    src/imageBaseOP/savefilebase.h \
    src/imageBaseOP/createofd.h \
    src/imageOp/imageoperationbase.h \
    src/imageOp/imageoperationbeauty.h \
    src/imageOp/imageoperationmirror.h \
    src/imageOp/imageoperationocr.h \
    src/imageOp/imageoperationrectify.h \
    src/include/common.h \
    src/include/theme.h \
    src/leftimagehandlesuccesspagewidget.h \
    src/leftsuccesspagewidget.h \
    src/mainwidget.h \
    src/newdevicelistpage.h \
    src/nodevicewidget.h \
    src/rectify.h \
    src/runningdialog.h \
    src/saneobject.h \
    src/scandialog.h \
    src/scansettingswidget.h \
    src/sendmail.h \
    src/showimagewidget.h \
    src/showocrwidget.h \
    src/successpagewidget.h \
    src/svghandler.h \
    src/thumbnaildelegate.h \
    src/thumbnailwidget.h \
    src/about/custom_about_dialog.h \
    src/titlebar/titlebar.h \
    src/toolbarwidget.h \
    src/usb.h \
    src/usbhotplugthread.h \
    src/utils/HorizontalOrVerticalMode.h \
    src/utils/rotatechangeinfo.h \
    src/utils/scannersettings.h \
    src/waittingdialog.h \
    src/watermarkdialog.h   \
    src/custom_push_button.h    \
    src/navigator.h \
    src/kabase/buriedpoint.hpp \
    src/kabase/windowmanage.hpp \
    src/include/customlabel.h \
    test/mytest.h


# Manual
DISTFILES += \
    man/kylin-scanner.1 \
    src/icons/waiting/loading1.svg \
    src/icons/waiting/loading2.svg \
    src/icons/waiting/loading3.svg \
    src/icons/waiting/loading4.svg \
    src/icons/waiting/loading5.svg \
    src/icons/waiting/loading6.svg \
    src/icons/waiting/loading7.svg

HEADERS += \
    $$PWD/src/device/ukui_apt.h \
    $$PWD/src/device/device_information.h \
    $$PWD/src/device/base_info.h \
    $$PWD/src/device/common.h \
    $$PWD/src/device/usbFinder.h \
    $$PWD/src/device/singleton.h \
    $$PWD/src/device/deviceFinder.h \
    $$PWD/src/device/connect.h \
    $$PWD/src/device/snmpFinder.h


SOURCES += \
    $$PWD/src/device/device_information.cpp \
    $$PWD/src/device/common.cpp \
    $$PWD/src/device/ukui_apt.cpp \
    $$PWD/src/device/base_info.cpp \
    $$PWD/src/device/usbFinder.cpp \
    $$PWD/src/device/deviceFinder.cpp \
    $$PWD/src/device/connect.cpp \
    $$PWD/src/device/snmpFinder.cpp


# UI files
FORMS += \
    src/about/about.ui \
    src/titlebar/titlebar.ui \

# Translation
TRANSLATIONS += \
    translations/kylin-scanner_zh_CN.ts \
    translations/kylin-scanner_bo_CN.ts \      #藏
    translations/kylin-scanner_kk.ts \         #哈萨克
    translations/kylin-scanner_ky.ts \         #柯尔克孜
    translations/kylin-scanner_ug.ts \         #维吾尔
    translations/kylin-scanner_mn.ts \         #蒙古
    translations/kylin-scanner_zh_HK.ts        #繁

!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate pm")
system($$shell_path(cp $$PROJECT_ROOTDIR/translations/*.qm  $$OUT_PWD/))

# custom variable: target
target.files += $$TARGET
target.path = /usr/bin/

# custom variable: desktop
desktop.files += data/kylin-scanner.desktop
desktop.path = /usr/share/applications/

# custom variable: icons
icons.files += data/scanner.png
icons.path = /usr/share/pixmaps/

# custom variable: qm_files
qm_files.files = translations/*.qm
qm_files.path = /usr/share/kylin-scanner/translations/

# custom variable: help_files
help_files.files = data/kylin-scanner/
help_files.path = /usr/share/kylin-user-guide/data/guide/

schemes.files += \
     $$PWD/data/org.kylin-scanner-data.gschema.xml \
     $$PWD/data/org.ukui.log4qt.kylin-scanner.gschema.xml

schemes.path = /usr/share/glib-2.0/schemas/

jsonfiles.files += /data/kylin-scanner.json
jsonfiles.path = /etc/ukui/ukui-notification/default-app/


# Install file in system
INSTALLS += target desktop icons qm_files help_files schemes jsonfiles

RESOURCES += \
    src/icons/icons.qrc
DISTFILES += \
    data/org.kylin-scanner-data.gschema.xml \
    data/org.ukui.log4qt.kylin-scanner.gschema.xml \
