QT += core gui network KWindowSystem KWindowSystem dbus x11extras

greaterThan(QT_MAJOR_VERSION , 4) : QT += widgets

QMAKE_CXXFLAGS += -g -Wall

TARGET = kylin-calculator

LIBS += -lpthread
LIBS += -lX11
LIBS += -lgmp

CONFIG += link_pkgconfig c++11 link_pkgconfig lrelease
PKGCONFIG += gsettings-qt gsl kysdk-alm kysdk-ukenv kysdk-log kysdk-diagnostics kysdk-waylandhelper kysdk-sysinfo kysdk-qtwidgets kysdk-ukuiwindowhelper kysdk-datacollect

message($$system(./translations/generate_translations_qm.sh))

INCLUDEPATH +=                \
    $$PWD/src/                \
    $$PWD/src/menumodule/     \
    $$PWD/common/             \
    $$PWD/calculate/          \
    $$PWD/calculate/BigFloat/ \
    $$PWD/calculate/input/    \
    $$PWD/calculate/cal/    \
    $$PWD/kabase \
    $$PWD/kabase/Qt \

SOURCES +=                                       \
    $$PWD/calc_programmer/calc/calc.cpp \
    $$PWD/calc_programmer/calc/conversion.cpp \
    $$PWD/calc_programmer/processformula.cpp \
    $$PWD/common/HorizontalOrVerticalMode.cpp \
    $$PWD/common/InputSymbols.cpp                \
    $$PWD/common/picturetowhite.cpp \
    $$PWD/calculate/BigFloat/BigFloat.cpp        \
    $$PWD/calculate/cal/QStringCalculator.cpp    \
    $$PWD/calculate/input/InputJudgmentGraph.cpp \
    $$PWD/calculate/input/InputTools.cpp         \
    $$PWD/calculate/input/InputProcess.cpp       \
    $$PWD/src/mainwindow.cpp                     \
    $$PWD/src/outputbase.cpp \
    $$PWD/src/programmer/basebinary.cpp \
    $$PWD/src/programmer/binarykeyboary.cpp \
    $$PWD/src/programmer/programdisplay.cpp \
    $$PWD/src/programmer/programkeyboary.cpp \
    $$PWD/src/programmer/programmodel.cpp \
    $$PWD/src/programmer/toolbar.cpp \
    $$PWD/src/titlebar.cpp                       \
    $$PWD/src/widgetstyle.cpp                    \
    $$PWD/src/funclist.cpp                       \
    $$PWD/src/standardmodel.cpp                  \
    $$PWD/src/scientificmodel.cpp                \
    $$PWD/src/toolmodel.cpp                      \
    $$PWD/src/basicbutton.cpp                    \
    $$PWD/src/menumodule/menumodule.cpp          \
    $$PWD/data_warehouse.cpp                     \
    $$PWD/logic_center.cpp

HEADERS +=                                     \
    $$PWD/calc_programmer/calc/calc.h \
    $$PWD/calc_programmer/calc/conversion.h \
    $$PWD/calc_programmer/processformula.h \
    $$PWD/common/HorizontalOrVerticalMode.h \
    $$PWD/common/InputSymbols.h                \
    $$PWD/common/picturetowhite.h \
    $$PWD/common/highlight-effect.h            \
    $$PWD/calculate/cal.h                      \
    $$PWD/calculate/BigFloat/BigFloat.h        \
    $$PWD/calculate/cal/QStringCalculator.h    \
    $$PWD/calculate/input/InputJudgmentGraph.h \
    $$PWD/calculate/input/InputTools.h         \
    $$PWD/calculate/input/InputProcess.h       \
    $$PWD/src/mainwindow.h                     \
    $$PWD/src/outputbase.h \
    $$PWD/src/programmer/basebinary.h \
    $$PWD/src/programmer/binarykeyboary.h \
    $$PWD/src/programmer/programdisplay.h \
    $$PWD/src/programmer/programkeyboary.h \
    $$PWD/src/programmer/programmodel.h \
    $$PWD/src/programmer/toolbar.h \
    $$PWD/src/titlebar.h                       \
    $$PWD/src/widgetstyle.h                    \
    $$PWD/src/scientificmodel.h                \
    $$PWD/src/funclist.h                       \
    $$PWD/src/standardmodel.h                  \
    $$PWD/src/toolmodel.h                      \
    $$PWD/src/basicbutton.h                    \
    $$PWD/src/menumodule/menumodule.h          \
    $$PWD/data_warehouse.h                     \
    $$PWD/logic_center.h

RESOURCES += \
    $$PWD/image.qrc

#TRANSLATIONS += $$PWD/translations/kylin-calculator_zh_CN.ts \
#                $$PWD/translations/kylin-calculator_zh_HK.ts \
#                $$PWD/translations/kylin-calculator_bo_CN.ts \
#                $$PWD/translations/kylin-calculator_mn.ts
#QM_FILES_INSTALL_PATH = /usr/share/kylin-calculator/translations/

DISTFILES += \
    intel_plug/Calculator.json

# 安装
target.source += $$TARGET
target.path += /usr/bin

contains(CONFIG, KAIMING){
    guide.files = data/guide/kylin-calculator/
    guide.path = /opt/apps/top.openkylin.kylin-calculator/files/share/kylin-user-guide/data/guide/
} else{
    guide.files = data/guide/kylin-calculator/
    guide.path = /usr/share/kylin-user-guide/data/guide/
}

lib.files = $$PWD/libkylin-calculator.so
lib.path = /opt/small-plugin/children/bin

contains(CONFIG, KAIMING){
    desktop.files += $$PWD/kylin-calculator.desktop
    desktop.path = /opt/apps/top.openkylin.kylin-calculator/files/share/applications
} else{
    DEFINES += NOTKAIMING
    desktop.files += $$PWD/kylin-calculator.desktop
    desktop.path = /usr/share/applications/
}

contains(CONFIG, KAIMING){
    translationsFiles.files += $$PWD/translations/*.qm
    translationsFiles.path = /opt/apps/top.openkylin.kylin-calculator/files
} else{
    DEFINES += NOTKAIMING
    translationsFiles.files += $$PWD/translations/*.qm
    translationsFiles.path = /usr/share/kylin-calculator/translations/
}

INSTALLS +=           \
    target            \
    lib               \
    desktop           \
    translationsFiles \
    guide             \
