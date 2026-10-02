QT       += core gui dbus network concurrent KWindowSystem

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras

CONFIG += c++11 link_pkgconfig
PKGCONFIG += gsettings-qt kysdk-waylandhelper kysdk-qtwidgets


QMAKE_CXXFLAGS += -std=c++0x

LIBS  +=  -lX11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DESTDIR = ../
TARGET = Revision
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
#    border_shadow_effect.cpp \
    dbusconnect.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindowresult.cpp \
    restorepage/prerestorefailure.cpp \
    restorepage/prerestoresucess.cpp \
    restorepage/restorefailure.cpp \
    restorepage/restorenotification.cpp \
    restorepage/restorepage1.cpp \
    restorepage/restorepage2.cpp \
    restorepage/restoreprogress.cpp \
    restorepage/restoresuccess.cpp \
    resultpage/restorefail.cpp \
    resultpage/restoresucc.cpp \
    resultpage/upgradefail.cpp \
    resultpage/upgradefaillist.cpp \
    resultpage/upgradenote.cpp \
    resultpage/upgradesucc.cpp \
    single/qtlocalpeer.cpp \
    single/qtlockedfile.cpp \
    single/qtlockedfile_unix.cpp \
    single/qtsingleapplication.cpp \
 #    utils/xatom-helper.cpp
    upgradepage/advancedoptions.cpp \
    upgradepage/attentionpage.cpp \
    upgradepage/importactivationfilespage.cpp \
    upgradepage/initpage.cpp \
    upgradepage/preupgradefailure.cpp \
    upgradepage/preupgradesuccess.cpp \
    upgradepage/upgradefailure.cpp \
    upgradepage/upgradenotification.cpp \
    upgradepage/upgradepage1.cpp \
    upgradepage/upgradepage2.cpp \
    upgradepage/upgradeprocess.cpp \
    upgradepage/upgradesuccess.cpp

HEADERS += \
#    border_shadow_effect.h \
    dbusconnect.h \
    mainwindow.h \
    mainwindowresult.h \
    restorepage/prerestorefailure.h \
    restorepage/prerestoresucess.h \
    restorepage/restorefailure.h \
    restorepage/restorenotification.h \
    restorepage/restorepage1.h \
    restorepage/restorepage2.h \
    restorepage/restoreprogress.h \
    restorepage/restoresuccess.h \
    resultpage/restorefail.h \
    resultpage/restoresucc.h \
    resultpage/upgradefail.h \
    resultpage/upgradefaillist.h \
    resultpage/upgradenote.h \
    resultpage/upgradesucc.h \
    single/qtlocalpeer.h \
    single/qtlockedfile.h \
#    single/qtlockedfile_unix.h \
    single/qtsingleapplication.h \
 #    utils/xatom-helper.h
    upgradepage/advancedoptions.h \
    upgradepage/attentionpage.h \
    upgradepage/importactivationfilespage.h \
    upgradepage/initpage.h \
    upgradepage/preupgradefailure.h \
    upgradepage/preupgradesuccess.h \
    upgradepage/upgradefailure.h \
    upgradepage/upgradenotification.h \
    upgradepage/upgradepage1.h \
    upgradepage/upgradepage2.h \
    upgradepage/upgradeprocess.h \
    upgradepage/upgradesuccess.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc
