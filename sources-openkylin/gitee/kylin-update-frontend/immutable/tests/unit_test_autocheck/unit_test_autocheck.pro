QT += testlib network dbus sql widgets
QT -= gui
CONFIG += qt warn_on depend_includepath testcase c++11 link_pkgconfig

PKGCONFIG += gsettings-qt kysdk-qtwidgets kysdk-sysinfo yaml-cpp kysdk-diagnostics kysdk-systime

TEMPLATE = app

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions

LIBS +=  -lgtest_main -lpthread -lX11
LIBS +=  -L$$[QT_INSTALL_LIBS]  -lXi  -lgsettings-qt -lgtest -lgcov

SOURCES +=  \
    ../../plugin/src/updatedbus.cpp \
    main.cpp \
    unit_test_autocheck.cpp

HEADERS += \
    ../../plugin/src/updatedbus.h
