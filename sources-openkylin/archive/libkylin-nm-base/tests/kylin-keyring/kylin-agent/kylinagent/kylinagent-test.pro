QT       += core gui x11extras dbus KWindowSystem svg concurrent network testlib
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_kylinagent
TEMPLATE = app

CONFIG += c++14 qt link_pkgconfig warn_on console no_keywords

PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 libnm libnma libsecret-1 gsettings-qt libcap kysdk-qtwidgets

DEFINES += QT_DEPRECATED_WARNINGS


LIBS += -lgcov
LIBS +=  -L/usr/lib/ -lKF5NetworkManagerQt
#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions

INCLUDEPATH += /usr/include/KF5/NetworkManagerQt
INCLUDEPATH += /usr/include/libsecret-1
INCLUDEPATH += /usr/include/glib-2.0


INCLUDEPATH += $$PWD/../../../../kylin-keyring/kylin-agent



HEADERS +=  \
    ../../../../kylin-keyring/kylin-agent/agentutils.h \
    ../../../../kylin-keyring/kylin-agent/kylinagent.h \
    ../../../../kylin-keyring/kylin-agent/kylinpassworddialog.h \
    ../../../../kylin-keyring/kylin-agent/kylinsecretagent.h \
    test_kylinagent.h \
    test_kylinpassworddialog.h \
    test_kylinsecretagent.h

SOURCES += \
    ../../../../kylin-keyring/kylin-agent/kylinagent.cpp \
    ../../../../kylin-keyring/kylin-agent/kylinpassworddialog.cpp \
    ../../../../kylin-keyring/kylin-agent/kylinsecretagent.cpp \
    test_kylinagent.cpp \
    test_kylinpassworddialog.cpp \
    test_kylinsecretagent.cpp \
    main.cpp
