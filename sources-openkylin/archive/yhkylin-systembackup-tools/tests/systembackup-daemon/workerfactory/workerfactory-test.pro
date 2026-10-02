TEMPLATE = app
TARGET = test_workerfactory

QT += testlib core widgets dbus

QMAKE_CXXFLAGS +=  -fno-exceptions

SOURCES += \
    test_workerfactory.cpp \
    main.cpp \
    ../../../systembackup-daemon/workerfactory.cpp \
    ../../../common/reflect.cpp

HEADERS += \
    test_workerfactory.h \
    ../../../systembackup-daemon/workerfactory.h \
    ../../../common/mydefine.h \
    ../../../common/reflect.h

INCLUDEPATH += \
    ../../../systembackup-daemon \
    ../../../common

# Coverage flags
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

# GTest/GMock flags (if needed)
# LIBS += -lgtest -lgmock -lpthread
