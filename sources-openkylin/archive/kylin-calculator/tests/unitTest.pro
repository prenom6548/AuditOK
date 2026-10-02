QT += core gui network KWindowSystem KWindowSystem dbus x11extras testlib

greaterThan(QT_MAJOR_VERSION , 4) : QT += widgets
QMAKE_CXXFLAGS += -g -Wall


TARGET = test
DEFINES += DISABLE_GMP

LIBS += -lpthread
LIBS += -lX11
LIBS += -lgmp

CONFIG += link_pkgconfig c++11 link_pkgconfig lrelease testcase
PKGCONFIG += gsettings-qt gsl kysdk-alm kysdk-ukenv kysdk-log kysdk-diagnostics kysdk-waylandhelper kysdk-sysinfo kysdk-qtwidgets kysdk-ukuiwindowhelper kysdk-datacollect
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LFLAGS += --coverage -lgcov

#ROOT_DIR = $$dirname($$_PRO_FILE_PWD_)
#ROOT_DIR = ..
#message("111111 $$_PRO_FILE_PWD_")
#message("111111 $$ROOT_DIR")
INCLUDEPATH +=                \
    ../ 			\
    ../src/                \
    ../src/menumodule/     \
    ../common/             \
    ../calculate/          \
    ../calculate/BigFloat/ \
    ../calculate/input/    \
    ../calculate/cal/    \
    ../kabase \
    ../kabase/Qt \

SOURCES += \
    $$PWD/mainTest.cpp \
    $$PWD/testclass.cpp  \
    $$PWD/testfunlist.cpp \
    $$PWD/testscientificmodel.cpp \
    $$PWD/teststandardmodel.cpp \
    $$PWD/testtoolmodel.cpp \
    $$PWD/testtitlebar.cpp \
    $$PWD/testinputtools.cpp \
    $$PWD/testinputprocess.cpp \
    $$PWD/testbigfloat.cpp \
    $$PWD/testprogrammodel.cpp \
    $$PWD/testconversion.cpp \
    ../calc_programmer/calc/calc.cpp \
    ../calc_programmer/calc/conversion.cpp \
    ../calc_programmer/processformula.cpp \
    ../common/HorizontalOrVerticalMode.cpp \
    ../common/InputSymbols.cpp                \
    ../common/picturetowhite.cpp \
    ../calculate/BigFloat/BigFloat.cpp        \
    ../calculate/cal/QStringCalculator.cpp    \
    ../calculate/input/InputJudgmentGraph.cpp \
    ../calculate/input/InputTools.cpp         \
    ../calculate/input/InputProcess.cpp       \
    ../src/mainwindow.cpp                     \
    ../src/outputbase.cpp \
    ../src/programmer/basebinary.cpp \
    ../src/programmer/binarykeyboary.cpp \
    ../src/programmer/programdisplay.cpp \
    ../src/programmer/programkeyboary.cpp \
    ../src/programmer/programmodel.cpp \
    ../src/programmer/toolbar.cpp \
    ../src/titlebar.cpp                       \
    ../src/widgetstyle.cpp                    \
    ../src/funclist.cpp                       \
    ../src/standardmodel.cpp                  \
    ../src/scientificmodel.cpp                \
    ../src/toolmodel.cpp                      \
    ../src/basicbutton.cpp                    \
    ../src/menumodule/menumodule.cpp          \
    ../data_warehouse.cpp                     \
    ../logic_center.cpp




HEADERS += \
    $$PWD/testclass.h \
    $$PWD/testfunlist.h \
    $$PWD/testscientificmodel.h \
    $$PWD/teststandardmodel.h \
    $$PWD/testtoolmodel.h \
    $$PWD/testtitlebar.h \
    $$PWD/testinputtools.h \
    $$PWD/testinputprocess.h \
    $$PWD/testbigfloat.h \
    $$PWD/testprogrammodel.h \
    $$PWD/testconversion.h \
    ../calc_programmer/calc/calc.h \
    ../calc_programmer/calc/conversion.h \
    ../calc_programmer/processformula.h \
    ../common/HorizontalOrVerticalMode.h \
    ../common/InputSymbols.h                \
    ../common/picturetowhite.h \
    ../common/highlight-effect.h            \
    ../calculate/cal.h                      \
    ../calculate/BigFloat/BigFloat.h        \
    ../calculate/cal/QStringCalculator.h    \
    ../calculate/input/InputJudgmentGraph.h \
    ../calculate/input/InputTools.h         \
    ../calculate/input/InputProcess.h       \
    ../src/mainwindow.h                     \
    ../src/outputbase.h \
    ../src/programmer/basebinary.h \
    ../src/programmer/binarykeyboary.h \
    ../src/programmer/programdisplay.h \
    ../src/programmer/programkeyboary.h \
    ../src/programmer/programmodel.h \
    ../src/programmer/toolbar.h \
    ../src/titlebar.h                       \
    ../src/widgetstyle.h                    \
    ../src/scientificmodel.h                \
    ../src/funclist.h                       \
    ../src/standardmodel.h                  \
    ../src/toolmodel.h                      \
    ../src/basicbutton.h                    \
    ../src/menumodule/menumodule.h          \
    ../data_warehouse.h                     \
    ../logic_center.h

#RESOURCES += ../image.qrc

