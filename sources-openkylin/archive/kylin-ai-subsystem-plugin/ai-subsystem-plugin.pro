QT += widgets dbus xml

TEMPLATE = lib
DEFINES += LIBAISDKPLUGIN_LIBRARY
TARGET = kylin-ai-subsystem-plugin

CONFIG += plugin link_pkgconfig c++17
LIBS += -lukcc -lQApt -lgsettings-qt

PKGCONFIG += kysdk-qtwidgets kysdk-sysinfo

# 读取 KYLIN_RELEASE_ID 的值
SYSTEM_VERSION = $$system(cat /etc/os-release | grep KYLIN_RELEASE_ID | cut -d= -f2 | tr -d \'\"\')

# 根据版本号定义宏
equals(SYSTEM_VERSION, 2403) {
    DEFINES += SYSTEM_VERSION_2403
}
message(SYSTEM_VERSION: $$SYSTEM_VERSION)

TARGET = $$qtLibraryTarget(kylinaisubsystemplugin)
target.path = $$[QT_INSTALL_LIBS]/ukui-control-center

interfaces.path = /usr/include/kylin-ai-subsystem-plugin/
interfaces.files = aisubsystemplugininterface.h

schemas.files += $$PWD/data/org.kylin-ai-subsystem-plugin.gschema.xml
schemas.path = /usr/share/glib-2.0/schemas/

INSTALLS += interfaces target schemas

TRANSLATIONS = \
    translations/zh_CN.ts \
    translations/en_US.ts \
    translations/bo_CN.ts \
    translations/mn.ts \
    translations/kk.ts \
    translations/ky.ts \
    translations/mn.ts \
    translations/zh_HK.ts \
    translations/ug.ts \
    translations/vi.ts \
    translations/ms.ts \
    translations/th.ts \
    translations/ar.ts


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

qmfile.path = $${PREFIX}/share/kylin-ai-subsystem-plugin/translations/
qmfile.files = $${TRANSLATIONS_FILES}

DEFINES += KYLIN_AI_SUBSYSTEM_PLUGIN_PATH=\\\"$$[QT_INSTALL_LIBS]/kylin-ai-subsystem-plugin\\\"

HEADERS += \
    aisubsystemplugininterface.h \
    configmanager.h \
    subsysteminstallmodule.h \
    subsystempackagemanager.h \
    subsystemwidget.h

SOURCES += \
    configmanager.cpp \
    subsysteminstallmodule.cpp \
    subsystempackagemanager.cpp \
    subsystemwidget.cpp

RESOURCES += \
    image.qrc
