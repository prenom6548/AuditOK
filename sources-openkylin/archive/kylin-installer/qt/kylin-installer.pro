QT       += core gui xml dbus sql network QApt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kylin-installer

CONFIG += c++17

LIBS          += -L$$[QT_INSTALL_LIBS]

# 适配窗口管理器圆角阴影
QT += KWindowSystem x11extras

# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11
LIBS +=-ldl -lapt-pkg -lQApt

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-package

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS


DEFINES += QT_NO_INFO_OUTPUT #屏蔽信息输出
DEFINES += QT_NO_WARNING_OUTPUT #屏蔽警告输出

#DEFINES += QT_NO_DEBUG_OUTPUT #屏蔽qDebug输出

#配置sdk
PKGCONFIG += kysdk-qtwidgets kysdk-alm kysdk-log kysdk-waylandhelper
LIBS +=  -L/usr/lib/kysdk/kysdk-base -lkyconf -L/usr/lib/x86_64-linux-gnu/libkysdk-qtwidgets.so

INCLUDEPATH += /usr/include/kysdk/applications/
INCLUDEPATH += /usr/include/kysdk/kysdk-system/
INCLUDEPATH += /usr/include/kysdk/applications/

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    adaptscreeninfo.cpp \
    batch-install.cpp \
    dbuscallthread.cpp \
    debinfowidget.cpp \
    kylininstalldbuscallthread.cpp \
    main.cpp \
    kreinstall.cpp \
    package/PackageUtils.cpp \
    package/Package.cpp \
    package/KaimingPackage.cpp \
    package/DebPackage.cpp \
    package/ApkPackage.cpp \
    package/PackageFactory.cpp \
    package/PackageCheckUtils.cpp \
    picturetowhite.cpp \
    titlewidget.cpp \
    widbox.cpp \
    xatom-helper.cpp \
    qtlocalpeer.cpp \
    qtlockedfile.cpp \
    qtlockedfile_unix.cpp \
    qtsingleapplication.cpp \
    ../src/common/utils/CustomBoxUtil.cpp \
    ../src/common/utils/SystemInfoUtil.cpp

HEADERS += \
    ../src/common/GlobalInfo.h \
    adaptscreeninfo.h \
    batch-install.h \
    dbuscallthread.h \
    debinfowidget.h \
    kreinstall.h \
    kylininstalldbuscallthread.h \
    package/PackageEnum.h \
    package/PackageUtils.h \
    package/Package.h \
    package/KaimingPackage.h \
    package/DebPackage.h \
    package/ApkPackage.h \
    package/PackageFactory.h \
    package/PackageCheckUtils.h \
    picturetowhite.h \
    titlewidget.h \
    widbox.h \
    xatom-helper.h \
    qtlocalpeer.h \
    qtlockedfile.h \
    qtsingleapplication.h\
    ../src/common/utils/CustomBoxUtil.h \
    ../src/common/utils/SystemInfoUtil.h


TRANSLATIONS += \
                ../translations/kre-install_zh_CN.ts\
                ../translations/kre-install_bo_CN.ts \
                ../translations/kre-install_mn_MN.ts \
                ../translations/kre-install_zh_HK.ts \
                ../translations/kre-install_kk.ts \
                ../translations/kre-install_ky.ts \
                ../translations/kre-install_mn.ts \
                ../translations/kre-install_ug.ts \
                ../translations/kre-install_ms.ts \
                ../translations/kre-install_th.ts \
                ../translations/kre-install_vi.ts

translation.path = /usr/share/kre/kre-install/translations/
translation.files += ../translations/kre-install_zh_CN.qm
translation.files += ../translations/kre-install_bo_CN.qm
translation.files += ../translations/kre-install_mn_MN.qm
translation.files += ../translations/kre-install_zh_HK.qm
translation.files += ../translations/kre-install_kk.qm
translation.files += ../translations/kre-install_ky.qm
translation.files += ../translations/kre-install_mn.qm
translation.files += ../translations/kre-install_ug.qm
translation.files += ../translations/kre-install_ms.qm
translation.files += ../translations/kre-install_th.qm
translation.files += ../translations/kre-install_vi.qm

desktop.path = /usr/share/applications/
desktop.files += kylin-installer.desktop

guideCN.files += ./data/kylin-installer/zh_CN/
guideCN.path += /usr/share/kylin-user-guide/data/guide/kylin-installer/

guideEN.files += ./data/kylin-installer/en_US/
guideEN.path += /usr/share/kylin-user-guide/data/guide/kylin-installer/

imageformats.files += ./data/kylin-installer/kylin-installer.png
imageformats.path = /usr/share/kylin-user-guide/data/guide/kylin-installer/

# Default rules for deployment.
qnx: target.path = /usr/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target translation desktop guideCN guideEN imageformats

RESOURCES += \
    kre-install.qrc

FORMS +=

DISTFILES += \
    ../translations/kre-install_bo_CN.ts \
    ../translations/kre-install_zh_CN.ts \
    ../translations/kre-install_mn_MN.ts \
    ../translations/kre-install_zh_HK.ts


