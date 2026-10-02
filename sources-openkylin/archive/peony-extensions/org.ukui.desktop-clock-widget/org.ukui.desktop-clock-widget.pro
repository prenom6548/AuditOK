QT       += quick widgets dbus

TEMPLATE = lib
CONFIG  += plugin \
           link_pkgconfig

PKGCONFIG += kysdk-qtwidgets

TARGET = desktopclockbackend

include(../common.pri)

RESOURCES += desktopclock.qrc

SOURCES += \
    src/plugin.cpp \
    src/clockbackend.cpp \
    src/pinyin.cpp

HEADERS += \
    src/plugin.h \
    src/clockbackend.h \
    src/pinyin.h

qmldir.files += qmldir
qmldir.path  = $$[QT_INSTALL_QML]/org/ukui/desktopclock

target.path = $$[QT_INSTALL_QML]/org/ukui/desktopclock

# 编译 translations/ 目录下所有 .ts 为 .qm（qmake 内建 lrelease 特性）
CONFIG += lrelease
TRANSLATIONS += $$files($$PWD/translations/*.ts)

# 将生成的 .qm 安装到小插件的 translations 目录，供 ukui-quick 框架按系统语言加载
QM_FILES_INSTALL_PATH = /usr/share/ukui/widgets/org.ukui.desktop-clock-widget/translations

widget.files += \
    metadata.json \
    ui/main.qml \
    ui/AnalogClockFace.qml \
    fonts/DMSerifDisplay-Regular.ttf

widget.path = /usr/share/ukui/widgets/org.ukui.desktop-clock-widget

INSTALLS += target qmldir widget
