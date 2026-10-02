QT += core
QT -= gui
QT += dbus xml

TARGET = systembackup-daemon

CONFIG += c++11 console
CONFIG -= app_bundle

CONFIG += link_pkgconfig

LIBS += -lblkid \


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
QMAKE_CXXFLAGS += -Wall -ggdb -Wno-implicit-fallthrough

HEADERS += \
    ../common/mydefine.h \
    ../common/mydusizetool.h \
    ../common/mylittleparse.h \
    ../common/reflect.h \
    ../common/singleton.h \
    ../common/spinlock_mutex.h \
    ../common/utils.h \
    autokarebackupproxy.h \
    autokarerestoreproxy.h \
    autosystembackupproxy.h \
    autosystemrestoreproxy.h \
    backupmanager_adaptor.h \
    calcworker.h \
    customizedatabackupproxy.h \
    customizeghostImageproxy.h \
    customizesystembackupproxy.h \
    databackupproxy.h \
    datarestoreproxy.h \
    deletebackupproxy.h \
    ghostimageproxy.h \
    mybackupmanager.h \
    mymountproxy.h \
    myprocess/OstreeInterface.h \
    myprocess/calcbackupsize.h \
    myprocess/mksquashfsprocess.h \
    myprocess/mountbackupprocess.h \
    myprocess/rsyncpathtodirprocess.h \
    mythread.h \
    parsebackuplist.h \
    systembackupproxy.h \
    systemrestoreproxy.h \
    systemrestoreproxybak.h \
    systemrestoreworker.h \
    translations.h \
    udisksystembackupproxy.h \
    udisksystemrestoreproxy.h \
#    udiskdatabackupproxy.h \
#    udiskdatarestoreproxy.h \
#    udiskghostImageproxy.h \
#    udisksystembackupproxy.h \
#    udisksystemrestoreproxy.h \
    workerfactory.h

SOURCES += \
        ../common/mydefine.cpp \
        ../common/mydusizetool.cpp \
        ../common/mylittleparse.cpp \
        ../common/reflect.cpp \
        ../common/utils.cpp \
        autokarebackupproxy.cpp \
        autokarerestoreproxy.cpp \
        autosystembackupproxy.cpp \
        autosystemrestoreproxy.cpp \
        backupmanager_adaptor.cpp \
        calcworker.cpp \
        customizedatabackupproxy.cpp \
        customizeghostImageproxy.cpp \
        customizesystembackupproxy.cpp \
        databackupproxy.cpp \
        datarestoreproxy.cpp \
        deletebackupproxy.cpp \
        ghostimageproxy.cpp \
        main.cpp \
        mybackupmanager.cpp \
        mymountproxy.cpp \
        myprocess/OstreeInterface.cpp \
        myprocess/calcbackupsize.cpp \
        myprocess/mksquashfsprocess.cpp \
        myprocess/mountbackupprocess.cpp \
        myprocess/rsyncpathtodirprocess.cpp \
        mythread.cpp \
        parsebackuplist.cpp \
        systembackupproxy.cpp \
        systemrestoreproxy.cpp \
        systemrestoreproxybak.cpp \
        systemrestoreworker.cpp \
        translations.cpp \
        udisksystembackupproxy.cpp \
        udisksystemrestoreproxy.cpp \
#        udiskdatabackupproxy.cpp \
#        udiskdatarestoreproxy.cpp \
#        udiskghostImageproxy.cpp \
#        udisksystembackupproxy.cpp \
#        udisksystemrestoreproxy.cpp \
        workerfactory.cpp

# Default rules for deployment.
# qnx: target.path = /tmp/$${TARGET}/bin
# else: unix:!android: target.path = /usr/bin
# !isEmpty(target.path): INSTALLS += target


