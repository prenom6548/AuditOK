QT += testlib widgets dbus network x11extras KWindowSystem
CONFIG += c++11 console
CONFIG -= app_bundle

TEMPLATE = app

# 测试源文件
SOURCES += \
    test_page2.cpp

# 测试头文件
HEADERS += \
    test_page2.h

# 被测源文件（相对于项目根目录）
INCLUDEPATH += ../../../../UIControl/src \
               ../../../../UIControl/src/include

# 包含被测源文件
SOURCES += \
    ../../../../UIControl/src/page2.cpp \
    ../../../../UIControl/src/QRoundProgressBar.cpp \
    ../../../../UIControl/src/include/menumodule.cpp \
    ../../../../UIControl/src/include/menufactory.cpp \
    ../../../../UIControl/src/include/theme.cpp \
    ../../../../UIControl/src/include/qtlocalpeer.cpp \
    ../../../../UIControl/src/include/qtsingleapplication.cpp \
    ../../../../UIControl/src/include/xatom-helper.cpp \
    ../../../../UIControl/src/include/daemonipcdbus.cpp \
    ../../../../UIControl/src/include/global.cpp

# 包含被测头文件
HEADERS += \
    ../../../../UIControl/src/page2.h \
    ../../../../UIControl/src/QRoundProgressBar.h \
    ../../../../UIControl/src/include/menumodule.h \
    ../../../../UIControl/src/include/menufactory.h \
    ../../../../UIControl/src/include/theme.h \
    ../../../../UIControl/src/include/qtlocalpeer.h \
    ../../../../UIControl/src/include/qtlockedfile.h \
    ../../../../UIControl/src/include/qtsingleapplication.h \
    ../../../../UIControl/src/include/xatom-helper.h \
    ../../../../UIControl/src/include/daemonipcdbus.h \
    ../../../../UIControl/src/include/global.h

# 库依赖
LIBS += -lpthread \
        -lX11 \
        -lukui-log4qt

# pkg-config依赖
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-waylandhelper kysdk-qtwidgets kysdk-soundeffects kysdk-ukuiwindowhelper

# 覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

# 目标文件名
TARGET = test_page2