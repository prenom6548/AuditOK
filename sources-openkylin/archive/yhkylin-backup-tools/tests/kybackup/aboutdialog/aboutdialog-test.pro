TEMPLATE = app
TARGET = test_aboutdialog

# Qt modules
QT += core gui widgets testlib dbus xml network
QT += KWindowSystem x11extras

# C++ standard
CONFIG += c++14

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
PKGCONFIG += kysdk-qtwidgets kysdk-waylandhelper kysdk-soundeffects kysdk-systime kysdk-ukuiwindowhelper
PKGCONFIG += kysdk-appconf2
PKGCONFIG += kysdk-datacollect

# Include paths
INCLUDEPATH += $$PWD/../../../kybackup
INCLUDEPATH += $$PWD/../../../common
INCLUDEPATH += /usr/include/kysdk/applications

# Source files
SOURCES += \
    test_aboutdialog.cpp \
    main.cpp \
    ../../../kybackup/aboutdialog.cpp \
    ../../../common/utils.cpp \
    ../../../common/mylittleparse.cpp \
    ../../../backup-daemon/parsebackuplist.cpp

# Header files
HEADERS += \
    test_aboutdialog.h \
    ../../../kybackup/aboutdialog.h \
    ../../../kybackup/globalsignals.h

# # UI files
# FORMS += \
#     ../../../kybackup/aboutdialog.ui

# # Resource files
# RESOURCES += \
#     ../../../kybackup/app.qrc

# Coverage flags
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# Define any necessary macros
DEFINES += UNIT_TEST QT_DEPRECATED_WARNINGS

# Link libraries
LIBS += -lpthread
LIBS += -lX11
LIBS += -lgtest -lgtest_main