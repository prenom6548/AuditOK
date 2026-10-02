QT += core gui dbus sql widgets testlib

CONFIG += testcase c++11 link_pkgconfig

PKGCONFIG += gsettings-qt kysdk-qtwidgets  kysdk-waylandhelper  kysdk-ukuiwindowhelper kysdk-datacollect

#MOC_DIR = .moc
#OBJECTS_DIR = .obj

TARGET = unitTest

QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov
QMAKE_CXXFLAGS += -fno-exceptions
DEFINES += WHITE_TEST

DEFINES += QT_DEPRECATED_WARNINGS _UNICODE

INCLUDEPATH += \
    ../src  \


INCLUDEPATH += /usr/include/KF5/KWindowSystem/ ./log ./sliderbar ./kwidget /usr/include/kysdk/kysdk-base /usr/include

LIBS += -lKF5WindowSystem \
        -lmediainfo \
        -lukui-log4qt \
        -lkylog 

SOURCES += \
    $$PWD/unitMain.cpp \
    $$PWD/unitcontruct.cpp  \
    $$PWD/playlisttest.cpp  \
    $$PWD/qthelpertest.cpp  \
    $$PWD/sqlitehandletest.cpp  \
    $$PWD/extensionstest.cpp  \
    $$PWD/functionstest.cpp  \
    $$PWD/pathtest.cpp  \
    $$PWD/shortcutsettingtest.cpp  \
    $$PWD/filletwidgettest.cpp  \
    $$PWD/imagelabeltest.cpp  \
    $$PWD/kactiontest.cpp  \
    $$PWD/setupscreenshottest.cpp  \
    $$files(../src/core/*.cpp, true) \
    $$files(../src/global/*.cpp, true) \
    $$files(../src/widget/*.cpp, true) \
    ../src/mainwindow.cpp


FORMS += $$files(../src/*.ui, true)

HEADERS += \
    $$PWD/unitcontruct.h \
    $$PWD/playlisttest.h \
    $$PWD/qthelpertest.h \
    $$PWD/sqlitehandletest.h \
    $$PWD/extensionstest.h \
    $$PWD/functionstest.h \
    $$PWD/pathtest.h \
    $$PWD/shortcutsettingtest.h \
    $$PWD/filletwidgettest.h \
    $$PWD/imagelabeltest.h \
    $$PWD/kactiontest.h \
    $$PWD/setupscreenshottest.h \
    $$files(../src/*.h, true)


