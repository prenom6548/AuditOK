QT       += core gui dbus xml testlib KWindowSystem

TEMPLATE = app

CONFIG += c++14 link_pkgconfig no_keywords

PKGCONFIG += \
    gsettings-qt \
    Qt5Svg  \
    gio-2.0 \
    Qt5Multimedia \
    dconf \
    x11 \
    libcanberra \
    libpulse \
    libpulse-mainloop-glib \
    alsa \
    kysdk-sysinfo \
    kysdk-hardware \
    kysdk-waylandhelper \
    Qt5X11Extras \
    kysdk-qtwidgets

TARGET = unit_test_ukui_media_tray
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control

INCLUDEPATH   += /usr/include/gtest

LIBS +=  -lgtest_main -lpthread
LIBS +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lgtest -lgcov

HEADERS += \
    ../../common/json.h \
    ../../common/user_config.h \
    ../../common/user_info_json.h \
    ../../common/ukmedia_common.h \
    ../../common/sound_effect_json.h \
    ../../ukui-volume-control-applet-qt/config.h \
    ../../ukui-volume-control-applet-qt/custom_sound.h \
        ../../ukui-volume-control-applet-qt/DBusClient.h \
    ../../ukui-volume-control-applet-qt/ukmedia_main_widget.h \
    ../../ukui-volume-control-applet-qt/ukmedia_custom_class.h \
    ../../ukui-volume-control-applet-qt/ukmedia_volume_control.h \
    ../../ukui-volume-control-applet-qt/ukmedia_application_volume_widget.h \
    ../../ukui-volume-control-applet-qt/ukmedia_osd_display_widget.h \
    ../../ukui-volume-control-applet-qt/ukmedia_system_volume_widget.h \
    ../../ukui-volume-control-applet-qt/ukui_volume_control_user_config.h \
    ../../ukui-volume-control-applet-qt/ukui_list_widget_item.h \
    ../../ukui-volume-control-applet-qt/ukui_media_set_headset_widget.h \
    ../../ukui-volume-control-applet-qt/dbus-adaptor/dbus-adaptor.h \
    ../../ukui-volume-control-applet-qt/dbus-adaptor/bluez-adaptor.h \
    ../../ukui-volume-control-applet-qt/QtSingleApplication/qtlocalpeer.h \
    ../../ukui-volume-control-applet-qt/QtSingleApplication/qtsingleapplication.h

SOURCES += \
    ../../common/user_config.cpp \
    ../../common/user_info_json.cpp \
    ../../common/ukmedia_common.cpp \
    ../../common/sound_effect_json.cpp \
    ../../ukui-volume-control-applet-qt/custom_sound.cpp \
        ../../ukui-volume-control-applet-qt/DBusClient.cpp \
    ../../ukui-volume-control-applet-qt/ukmedia_main_widget.cpp \
    ../../ukui-volume-control-applet-qt/ukmedia_custom_class.cpp \
    ../../ukui-volume-control-applet-qt/ukmedia_volume_control.cpp \
    ../../ukui-volume-control-applet-qt/ukmedia_osd_display_widget.cpp \
    ../../ukui-volume-control-applet-qt/ukmedia_system_volume_widget.cpp \
    ../../ukui-volume-control-applet-qt/ukmedia_application_volume_widget.cpp \
    ../../ukui-volume-control-applet-qt/ukui_list_widget_item.cpp \
    ../../ukui-volume-control-applet-qt/ukui_media_set_headset_widget.cpp \
    ../../ukui-volume-control-applet-qt/ukui_volume_control_user_config.cpp \
    ../../ukui-volume-control-applet-qt/dbus-adaptor/dbus-adaptor.cpp \
    ../../ukui-volume-control-applet-qt/dbus-adaptor/bluez-adaptor.cpp \
    ../../ukui-volume-control-applet-qt/QtSingleApplication/qtlocalpeer.cpp \
    ../../ukui-volume-control-applet-qt/QtSingleApplication/qtsingleapplication.cpp \
    main.cpp

INSTALLS += target
