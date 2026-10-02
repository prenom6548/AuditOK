#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += dbus widgets
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

TARGET = unit_test_manager_treemodel
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control

LIBS          += -lgtest_main -lpthread
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov

#OBJECTS_DIR = ./obj
#MOC_DIR = ./moc

#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

SOURCES += ../../biometric-manager/src/treeitem.cpp \
           ../../biometric-manager/src/treemodel.cpp \
           ../../biometric-manager/src/customtype.cpp \
           ../../biometric-manager/src/configuration.cpp \
           unit_test_manager_treemodel.cpp \
           main.cpp

HEADERS += ../../biometric-manager/src/treeitem.h \
           ../../biometric-manager/src/treemodel.h \
           ../../biometric-manager/src/customtype.h \
           ../../biometric-manager/src/configuration.h
