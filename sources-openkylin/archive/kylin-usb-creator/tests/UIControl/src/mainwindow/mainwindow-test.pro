TEMPLATE = app
TARGET = test_mainwindow

QT += core gui widgets testlib dbus network x11extras KWindowSystem

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 link_pkgconfig
PKGCONFIG += kysdk-datacollect

SOURCES += \
    test_mainwindow.cpp \
    ../../../../UIControl/src/mainwindow.cpp \
    ../../../../UIControl/src/page1.cpp \
    ../../../../UIControl/src/page2.cpp \
    ../../../../UIControl/src/line.cpp \
    ../../../../UIControl/src/QRoundProgressBar.cpp \
    ../../../../UIControl/src/bottompointlabel.cpp \
    ../../../../UIControl/src/dbusadaptor.cpp \
    ../../../../UIControl/src/include/aboutdialog.cpp \
    ../../../../UIControl/src/include/global.cpp \
    ../../../../UIControl/src/include/menumodule.cpp \
    ../../../../UIControl/src/include/menufactory.cpp \
    ../../../../UIControl/src/include/qtlocalpeer.cpp \
    ../../../../UIControl/src/include/qtsingleapplication.cpp \
    ../../../../UIControl/src/include/xatom-helper.cpp \
    ../../../../UIControl/src/include/daemonipcdbus.cpp \
    ../../../../UIControl/src/include/theme.cpp \
    ../../../../UIControl/src/topborder.cpp

# 排除moc文件，避免重复定义
SOURCES -= moc_test_mainwindow.cpp

HEADERS += \
    test_mainwindow.h \
    ../../../../UIControl/src/mainwindow.h \
    ../../../../UIControl/src/page1.h \
    ../../../../UIControl/src/page2.h \
    ../../../../UIControl/src/line.h \
    ../../../../UIControl/src/QRoundProgressBar.h \
    ../../../../UIControl/src/bottompointlabel.h \
    ../../../../UIControl/src/dbusadaptor.h \
    ../../../../UIControl/src/include/aboutdialog.h \
    ../../../../UIControl/src/include/constant_class.h \
    ../../../../UIControl/src/include/daemonipcdbus.h \
    ../../../../UIControl/src/include/global.h \
    ../../../../UIControl/src/include/menumodule.h \
    ../../../../UIControl/src/include/menufactory.h \
    ../../../../UIControl/src/include/theme.h \
    ../../../../UIControl/src/include/qtlocalpeer.h \
    ../../../../UIControl/src/include/qtlockedfile.h \
    ../../../../UIControl/src/include/qtlockedfile.cpp \
    ../../../../UIControl/src/include/qtlockedfile_unix.cpp \
    ../../../../UIControl/src/include/qtsingleapplication.h \
    ../../../../UIControl/src/include/xatom-helper.h \
    ../../../../UIControl/src/topborder.h

INCLUDEPATH += \
    ../../../../UIControl/src \
    ../../../../UIControl/src/include \
    /usr/include/x86_64-linux-gnu/qt5 \
    /usr/include/x86_64-linux-gnu/qt5/QtCore \
    /usr/include/x86_64-linux-gnu/qt5/QtTest \
    /usr/include/x86_64-linux-gnu/qt5/QtWidgets

LIBS += -lpthread -lX11 -lukui-log4qt

# 覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

# 链接必要的Qt库和pkgconfig
PKGCONFIG += gsettings-qt kysdk-waylandhelper kysdk-qtwidgets kysdk-soundeffects kysdk-ukuiwindowhelper