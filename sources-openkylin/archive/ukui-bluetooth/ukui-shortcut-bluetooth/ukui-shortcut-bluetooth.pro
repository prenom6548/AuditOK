TEMPLATE = lib
TARGET = ukui-shortcut-bluetooth
QT += quick dbus core
CONFIG += plugin c++11  x11extras  link_pkgconfig

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_INFO_OUTPUT
DEFINES += QT_NO_DEBUG_OUTPUT

LIBS += -lpthread \
        -lukui-log4qt

PKGCONFIG  +=  x11  xi

TARGET = $$qtLibraryTarget($$TARGET)

# Input
SOURCES += \
        plugin/bluetoothinterface.cpp \
        plugin/bluetoothqml_plugin.cpp \
        plugin/commontool.cpp \
        plugin/devicemanager.cpp \
        plugin/platformadaptor.cpp

HEADERS += \
        plugin/CSingleton.h \
        plugin/bluetoothinterface.h \
        plugin/bluetoothqml_plugin.h \
        plugin/commontool.h \
        plugin/devicemanager.h \
        plugin/platformadaptor.h

RESOURCES += widget/qml.qrc

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}


inst1.files += widget/**
inst1.path = /usr/share/ukui/widgets/org.ukui.shortcut.bluetooth

inst2.files += plugin/qmldir
inst2.path = $$[QT_INSTALL_LIBS]/qt5/qml/org/ukui/shortcut/bluetooth


qm_files.files += translations/*.qm
qm_files.path = /usr/share/ukui/widgets/org.ukui.shortcut.bluetooth/translations

target.path = $$[QT_INSTALL_LIBS]/qt5/qml/org/ukui/shortcut/bluetooth

TRANSLATIONS += \
    translations/ukui-shortcut-bluetooth_zh_CN.ts \
    translations/ukui-shortcut-bluetooth_ar.ts     \
    translations/ukui-shortcut-bluetooth_kk.ts  \
    translations/ukui-shortcut-bluetooth_mn.ts  \
    translations/ukui-shortcut-bluetooth_bo_CN.ts  \
    translations/ukui-shortcut-bluetooth_ky.ts  \
    translations/ukui-shortcut-bluetooth_ug.ts  \
    translations/ukui-shortcut-bluetooth_zh_Hant.ts \
    translations/ukui-shortcut-bluetooth_zh_HK.ts

INSTALLS += inst1 \
    inst2 \
    qm_files \
    target 

OBJECTS_DIR = ./obj/
MOC_DIR = ./moc/

DISTFILES += \
    widget/ui/EllipsisLabel.qml



