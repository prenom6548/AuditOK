#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T14:35:43
#
#-------------------------------------------------
include(../../../env.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/FlowLayout/flowlayout.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Label/label.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/AddBtn/addbtn.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/SettingWidget/settingwidget.pri)
include($$PROJECT_ROOTDIR/libukcc/widgets/Button/button.pri)
include($$PROJECT_ROOTDIR/libukcc/interface.pri)


QT       += widgets dbus gui

TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig

TARGET = $$qtLibraryTarget(userinfo)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

LIBS          += -L$$[QT_INSTALL_LIBS] -lcrypt -lpolkit-qt5-core-1 -lpam -lpwquality -lkylin_chkname
DEFINES += ENABLEPQ

##加载gio库和gio-unix库
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt \
                 kysdk-qtwidgets \
                 kysdk-sysinfo \
                 kysdk-datacollect \


#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    changeusergroup.cpp \
    changeuserlogo.cpp \
    changeusernickname.cpp \
    changeuserpwd.cpp \
    changeusertype.cpp \
    circlelabel.cpp \
    creategroupdialog.cpp \
    createusernew.cpp \
    deleteuserexists.cpp \
    messagebar.cpp \
    mulverifydialog.cpp \
    otpbinddialog.cpp \
    pwdcheckthread.cpp \
    qtdbus/uniauthservice.cpp \
    qtdbus/pwdlessservice.cpp \
    securitykeysetdlg.cpp \
    userinfo.cpp \
    qtdbus/systemdbusdispatcher.cpp \
    passwdcheckutil.cpp \
    utilsforuserinfo.cpp

HEADERS += \
    changeusergroup.h \
    changeuserlogo.h \
    changeusernickname.h \
    changeuserpwd.h \
    changeusertype.h \
    circlelabel.h \
    creategroupdialog.h \
    createusernew.h \
    deleteuserexists.h \
    loginedusers.h \
    messagebar.h \
    mulverifydialog.h \
    otpbinddialog.h \
    pwdcheckthread.h \
    qtdbus/uniauthservice.h \
    qtdbus/pwdlessservice.h \
    securitykeysetdlg.h \
    userinfo.h \
    qtdbus/systemdbusdispatcher.h \
    passwdcheckutil.h \
    utilsforuserinfo.h

FORMS += \

INSTALLS += target
