QT       += dbus xml testlib widgets

TEMPLATE = app

CONFIG += c++14 link_pkgconfig no_keywords

PKGCONFIG += gio-2.0 \
             libxml-2.0 \
             Qt5Multimedia \
             gsettings-qt \
             libcanberra \
             dconf  \
             libpulse \
             libpulse-mainloop-glib \
             kysdk-qtwidgets \
             kysdk-sysinfo \
             kysdk-hardware

TARGET = unit_test_ukcc_audio_plugin
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control

INCLUDEPATH   += /usr/include/gtest

LIBS +=  -lgtest_main -lpthread
LIBS +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lgtest -lgcov
LIBS += -lukcc

HEADERS += \
    ../../common/ukmedia_common.h \
#    ../../audio/audio.h \
    ../../audio/customstyle.h \
    ../../audio/ukui_custom_style.h \
    ../../audio/ukui_list_widget_item.h  \
    ../../audio/ukmedia_main_widget.h \
    ../../audio/ukmedia_input_widget.h \
    ../../audio/ukmedia_output_widget.h \
    ../../audio/ukmedia_volume_control.h \
    ../../audio/ukmedia_settings_widget.h \
    ../../audio/ukmedia_sound_effects_widget.h \
    ../../audio/ukmedia_slider_tip_label_helper.h \
    ../../audio/app-device-control/ukmedia_app_device_ctrl.h \
    ../../audio/app-device-control/ukmedia_app_item_widget.h \
    ../../audio/audio-device-control/ukmedia_device_control_widget.h

SOURCES += \
    ../../common/ukmedia_common.cpp \
#    ../../audio/audio.cpp \
    ../../audio/customstyle.cpp \
    ../../audio/ukui_custom_style.cpp \
    ../../audio/ukui_list_widget_item.cpp \
    ../../audio/ukmedia_main_widget.cpp \
    ../../audio/ukmedia_input_widget.cpp \
    ../../audio/ukmedia_output_widget.cpp \
    ../../audio/ukmedia_volume_control.cpp \
    ../../audio/ukmedia_settings_widget.cpp \
    ../../audio/ukmedia_sound_effects_widget.cpp \
    ../../audio/ukmedia_slider_tip_label_helper.cpp \
    ../../audio/app-device-control/ukmedia_app_device_ctrl.cpp \
    ../../audio/app-device-control/ukmedia_app_item_widget.cpp \
    ../../audio/audio-device-control/ukmedia_device_control_widget.cpp \
    main.cpp

INSTALLS += target
