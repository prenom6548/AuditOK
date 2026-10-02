TEMPLATE = app
TARGET = test_leftsiderbarwidget

# 添加包含路径
INCLUDEPATH += ../../../kybackup \
               ../../../common \
               ../../../backup-daemon \
               ../../..

# 添加Qt模块
QT += core gui widgets testlib dbus xml KWindowSystem x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt
PKGCONFIG += kysdk-qtwidgets kysdk-waylandhelper kysdk-soundeffects kysdk-systime kysdk-ukuiwindowhelper
PKGCONFIG += kysdk-appconf2
PKGCONFIG += kysdk-datacollect

# 添加源文件
SOURCES += test_leftsiderbarwidget.cpp \
           ../../../kybackup/leftsiderbarwidget.cpp \
           ../../../kybackup/functypeconverter.cpp \
           ../../../kybackup/component/mylabel.cpp \
           ../../../kybackup/component/pixmaplabel.cpp \
           ../../../kybackup/component/doubleclickwidget.cpp \
           ../../../kybackup/component/clicklabel.cpp \
           ../../../kybackup/gsettingswrapper.cpp \
           ../../../common/mydefine.cpp \
           ../../../common/utils.cpp \
           ../../../kybackup/component/imageutil.cpp \
           ../../../backup-daemon/parsebackuplist.cpp \
           ../../../kybackup/messageboxutils.cpp \
           ../../../common/mylittleparse.cpp

# 添加头文件
HEADERS += test_leftsiderbarwidget.h \
           ../../../kybackup/leftsiderbarwidget.h \
           ../../../kybackup/functypeconverter.h \
           ../../../kybackup/component/mylabel.h \
           ../../../kybackup/component/pixmaplabel.h \
           ../../../kybackup/component/doubleclickwidget.h \
           ../../../kybackup/component/clicklabel.h \
           ../../../kybackup/gsettingswrapper.h \
           ../../../kybackup/globalbackupinfo.h \
           ../../../kybackup/globalsignals.h \
           ../../../common/mydefine.h \
           ../../../common/utils.h \
           ../../../common/singleton.h \
           ../../../common/spinlock_mutex.h \
           ../../../kybackup/component/imageutil.h \
           ../../../backup-daemon/parsebackuplist.h \
           ../../../kybackup/messageboxutils.h \
           ../../../common/mylittleparse.h

# 添加覆盖率标志
QMAKE_CXXFLAGS += --coverage
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

# 设置目标路径
DESTDIR = ./