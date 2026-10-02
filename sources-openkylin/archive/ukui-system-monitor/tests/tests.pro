QT += dbus concurrent KWindowSystem
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../src/procnet-monitor/procnet-monitor.pri)

TEMPLATE = app

TARGET = unit_test_systemmonitor
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control

LIBS          += -lgtest_main -lpthread
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov -lpcap

CONFIG += link_pkgconfig \
          c++11

PKGCONFIG += libsystemd \
             kysdk-utils \
             kysdk-waylandhelper \
             kysdk-diagnostics \
             kysdk-sysinfo \
             kysdk-proc \
             kysdk-hardware \
             kysdk-net \
             kysdk-realtime \
             kysdk-disk \
             kysdk-package \

#OBJECTS_DIR = ./obj
#MOC_DIR = ./moc

# 打桩工具
# Define paths
TEST_UTILS_PATH = $$PWD/kt-test-utils

INCLUDEPATH += $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

# Header files (if needed)
HEADERS += $$files($$TEST_UTILS_PATH/cpp-stub/*.h) \
           $$files($$TEST_UTILS_PATH/cpp-stub/*.hpp) \
           $$files($$TEST_UTILS_PATH/cpp-stub-ext/*.h)

# Gather source files
SOURCES += $$files($$TEST_UTILS_PATH/cpp-stub/*.cpp) \
           $$files($$TEST_UTILS_PATH/cpp-stub-ext/*.cpp) \

# source files
SOURCES += ../src/filesystem/filesystemworker.cpp \
           ../src/filesystem/filesystemdata.cpp \
           ../src/filesystem/datacdrom.cpp \
           ../src/util.cpp \
           ../src/service/servicedataworker.cpp \
           ../src/service/servicemanager.cpp \
           ../src/service/serviceinfo.cpp \
           ../src/service/kerror.cpp \
           ../src/service/systemd1managerinterface.cpp \
           ../src/service/systemd1serviceinterface.cpp \
           ../src/service/servicefileinfo.cpp \
           ../src/service/dbuspropertiesinterface.cpp \
           ../src/sysresource/commoninfo.cpp \
           ../src/process/process_monitor.cpp \
           ../src/process/process_list.cpp \
           ../src/process/process_network.cpp \
           ../src/process/processwndinfo.cpp \
           ../src/desktopfileinfo.cpp \


HEADERS += ../src/filesystem/filesystemworker.h \
           ../src/filesystem/filesystemdata.h \
           ../src/filesystem/datacdrom.h \
           ../src/util.h \
           ../src/service/servicedataworker.h \
           ../src/service/servicemanager.h \
           ../src/service/serviceinfo.h \
           ../src/service/kerror.h \
           ../src/service/systemd1managerinterface.h \
           ../src/service/systemd1serviceinterface.h \
           ../src/service/servicecommon.h \
           ../src/service/servicefileinfo.h \
           ../src/service/dbuspropertiesinterface.h \
           ../src/sysresource/commoninfo.h \
           ../src/process/process_monitor.h \
           ../src/process/process_list.h \
           ../src/process/process_data.h \
           ../src/process/process_network.h \
           ../src/process/processwndinfo.h \
           ../src/desktopfileinfo.h \


# unit test files
SOURCES += main.cpp \
           unit_test_filesystem.cpp \
           unit_test_service.cpp \
           unit_test_commoninfo.cpp \
           unit_test_utils.cpp \
           unit_test_process.cpp \

