QT       += dbus xml testlib widgets

TEMPLATE = app

CONFIG += c++14 link_pkgconfig no_keywords

PKGCONFIG += gio-2.0 \
             libxml-2.0 \
             dconf \
             gsettings-qt \
             kysdk-qtwidgets \
             Qt5Multimedia \
             kysdk-sysinfo \
             kysdk-hardware

TARGET = unit_test_ukcc_audio_dev_ctrl
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
#    ../../common/ukmedia_common.h \
#    ../../audio/ukui_custom_style.h \
    ../../audio/audio-device-control/ukmedia_device_control_widget.h

SOURCES += \
#    ../../common/ukmedia_common.cpp \
#    ../../audio/ukui_custom_style.cpp \
    ../../audio/audio-device-control/ukmedia_device_control_widget.cpp \
    main.cpp

INSTALLS += target
