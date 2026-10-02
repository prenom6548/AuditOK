QT += widgets network core

TEMPLATE = lib

CONFIG += link_pkgconfig

PKGCONFIG += kylin-ai-subsystem-plugin  gsettings-qt kysdk-qtwidgets kysdk-waylandhelper glib-2.0 kysdk-sysinfo \
            kyai-config kysdk-ukenv libqapt

INCLUDEPATH += /usr/include/KF5/KWindowSystem/
INCLUDEPATH += /usr/include/qapt/
include(libukcc/widgets.pri)

DEFINES += APP_PREFIX=\\\"$$PREFIX\\\"

# 读取 KYLIN_RELEASE_ID 的值
SYSTEM_VERSION = $$system(cat /etc/os-release | grep KYLIN_RELEASE_ID | cut -d= -f2 | tr -d \'\"\')

# 根据版本号定义宏
equals(SYSTEM_VERSION, 2403) {
    DEFINES += SYSTEM_VERSION_2403
}
message(SYSTEM_VERSION: $$SYSTEM_VERSION)

LIBS += -lukui-log4qt -lKF5WindowSystem -lKF5Screen -lkysdk-qtwidgets -lkysdk-kabase   -lpolkit-qt5-core-1 -lcurl

TRANSLATIONS = translations/kylin-ai-subsystem-modelconfig/Internationalization_zh_CN.ts \
               translations/kylin-ai-subsystem-modelconfig/Internationalization_zh_HK.ts \
               translations/kylin-ai-subsystem-modelconfig/Internationalization_mn.ts \
               translations/kylin-ai-subsystem-modelconfig/Internationalization_bo_CN.ts \
               translations/kylin-ai-subsystem-modelconfig/Internationalization_kk.ts \
               translations/kylin-ai-subsystem-modelconfig/Internationalization_ky.ts \
               translations/kylin-ai-subsystem-modelconfig/Internationalization_ug.ts \
               translations/kylin-ai-subsystem-modelconfig/Internationalization_ar.ts \
               translations/kylin-ai-subsystem-modelconfig/Internationalization_ms.ts \
               translations/kylin-ai-subsystem-modelconfig/Internationalization_th.ts \
               translations/kylin-ai-subsystem-modelconfig/Internationalization_vi.ts

# Generate translations in build
TRANSLATIONS_FILES =

qtPrepareTool(LRELEASE, lrelease)
for(tsfile, TRANSLATIONS) {
    qmfile = $$shadowed($$tsfile)
    qmfile ~= s,.ts$,.qm,
    qmdir = $$dirname(qmfile)
    !exists($$qmdir) {
        mkpath($$qmdir)|error("Aborting.")
    }
    command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
    system($$command)|error("Failed to run: $$command")
    TRANSLATIONS_FILES += $$qmfile
}

qmfile.path = $${PREFIX}/share/kylin-ai-subsystem-plugin/kylin-ai-subsystem-modelconfig/translations/
qmfile.files = $${TRANSLATIONS_FILES}

//模型配置UI插件安装目录
target.path = $$[QT_INSTALL_LIBS]/kylin-ai-subsystem-plugin

polkitflie.files = data/org.kylin-ai-subsystem-modelconfig.policy
polkitflie.path = /usr/share/polkit-1/actions

schemas.files += $$PWD/data/org.kylin-ai-subsystem-modelconfig.gschema.xml
schemas.path = /usr/share/glib-2.0/schemas/

INSTALLS += target qmfile polkitflie schemas

SOURCES += \
    aisubsystem.cpp \
    appversionerrorwarningwidget.cpp \
    cloudmodelconfigwidget.cpp \
    cloudmodelitem.cpp \
    computingpowersharing/computepowerinterface.cpp \
    computingpowersharing/computepowerlist.cpp \
    computingpowersharing/computepowersharedialog.cpp \
    computingpowersharing/computepowersharewidget.cpp \
    computingpowersharing/ipitemwidget.cpp \
    computingpowersharing/requestcomputepowerlist.cpp \
    computingpowersharing/sharedcomputepowerlist.cpp \
    computingpowersharing/sharedmodelconfigwidget.cpp \
    configmanager.cpp \
    expandablemodelgroup.cpp \
    kaglobal.cpp \
    localmodelconfigwidget.cpp \
    localmodelitem.cpp \
    modelcallpriority.cpp \
    modelconfigdialog.cpp \
    modeldownload.cpp \
    modelordermanager.cpp \
    modelwidget.cpp \
    privatemodelconfigdialog.cpp \
    privatemodelconfigwidget.cpp \
    privatemodelitemwidget.cpp \
    updatenotificationwidget.cpp

HEADERS += \
    aisubsystem.h \
    appversionerrorwarningwidget.h \
    cloudmodelconfigwidget.h \
    cloudmodelitem.h \
    computingpowersharing/computepowerinterface.h \
    computingpowersharing/computepowerlist.h \
    computingpowersharing/computepowersharedialog.h \
    computingpowersharing/computepowersharewidget.h \
    computingpowersharing/ipitemwidget.h \
    computingpowersharing/requestcomputepowerlist.h \
    computingpowersharing/sharedcomputepowerlist.h \
    computingpowersharing/sharedmodelconfigwidget.h \
    configmanager.h \
    expandablemodelgroup.h \
    kaglobal.h \
    localmodelconfigwidget.h \
    localmodelitem.h \
    modelcallpriority.h \
    modelconfigdialog.h \
    modeldownload.h \
    modelordermanager.h \
    modelwidget.h \
    privatemodelconfigdialog.h \
    privatemodelconfigwidget.h \
    privatemodelitemwidget.h \
    updatenotificationwidget.h \
    util.h

RESOURCES += \
    res.qrc
