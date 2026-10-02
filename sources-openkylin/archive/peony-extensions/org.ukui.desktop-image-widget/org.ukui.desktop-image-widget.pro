QT += quick widgets gui

TEMPLATE = lib
CONFIG += plugin

TARGET = desktopimagebackend

include(../common.pri)

RESOURCES += desktopimage.qrc

SOURCES += \
    src/plugin.cpp \
    src/imagebackend.cpp

HEADERS += \
    src/plugin.h \
    src/imagebackend.h

qmldir.files += qmldir
qmldir.path = $$[QT_INSTALL_QML]/org/ukui/desktopimage

target.path = $$[QT_INSTALL_QML]/org/ukui/desktopimage

# 编译 translations/ 目录下所有 .ts 为 .qm（qmake 内建 lrelease 特性）
CONFIG += lrelease
TRANSLATIONS += $$files($$PWD/translations/*.ts)

# 将生成的 .qm 安装到小插件的 translations 目录，供 ukui-quick 框架按系统语言加载
QM_FILES_INSTALL_PATH = /usr/share/ukui/widgets/org.ukui.desktop-image-widget/translations

widget.files += \
    metadata.json \
    ui/main.qml

widget.path = /usr/share/ukui/widgets/org.ukui.desktop-image-widget

INSTALLS += target qmldir widget
