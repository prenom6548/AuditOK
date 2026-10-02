QT += widgets gui dbus

CONFIG += link_pkgconfig c++14
PKGCONFIG += \
            gtest \
            gtest_main \
            dbus-1 \
            glib-2.0 \
            kysdk-conf2


QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline
QMAKE_CFLAGS += --coverage -fno-inline

INCLUDEPATH += /usr/include/kysdk/kysdk-base

LIBS += -lgcov \
        -lkylog \
        -lcjson \
        -lkyconf2

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
                ../../packages

SOURCES += \
    kypackage-test.cpp \
    ../../packages/libkypackages.c

HEADERS += \
    ../../packages/libkypackages.h

# Default rules for deployment.
headers.files = $${HEADERS}
headers.path = /usr/include/kysdk/base/
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers

