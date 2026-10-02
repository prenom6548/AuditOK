QT       += widgets dbus gui x11extras KWindowSystem

TEMPLATE = lib
CONFIG += c++11 plugin link_pkgconfig

TARGET = $$qtLibraryTarget(biometrics)
target.path = $$[QT_INSTALL_LIBS]/ukui-control-center 

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR
LIBS += -lukcc
LIBS +=-lX11
TRANSLATIONS += translations/zh_CN.ts

PREFIX = /usr/share/biometric-manager/

TRANSLATIONS += i18n_ts/zh_CN.ts \
                i18n_ts/bo_CN.ts \
		i18n_ts/en_US.ts \
		i18n_ts/zh_HK.ts \
		i18n_ts/mn.ts \
		i18n_ts/kk.ts \
		i18n_ts/ug.ts \
		i18n_ts/ky.ts \
		i18n_ts/de.ts \
		i18n_ts/es.ts \
		i18n_ts/fr.ts \
                i18n_ts/vi.ts \
                i18n_ts/ms.ts \
                i18n_ts/th.ts

system("lrelease i18n_ts/*.ts")

qm_file.files = i18n_ts/*.qm
qm_file.path = $${PREFIX}/biometrics/i18n_qm/

ts_file.files = i18n_ts/*.ts
ts_file.path = $${PREFIX}/biometrics/i18n_ts/


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS ENABLEPQ QT_MESSAGELOGCONTEXT

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lpolkit-qt5-core-1  -lpwquality

CONFIG        += link_pkgconfig \
                 C++11

PKGCONFIG     += gsettings-qt \
                 gio-2.0 \
		 gio-unix-2.0 \
                 x11 \
                 kysdk-qtwidgets \
                 kysdk-ukuiwindowhelper

SOURCES += \
    biometrics.cpp \
    biometricswidget.cpp \
    biometricdeviceinfo.cpp \
    biometricproxy.cpp \
    biometricenroll.cpp \
    biometricmoreinfo.cpp \
    enrollbtn.cpp \
    giodbus.cpp \
    kalabel.cpp \
    qrcodeenroll.cpp \
    securitykeysetdlg.cpp \
    servicemanager.cpp \
    namelabel.cpp \
    changefeaturename.cpp \
    changepwddialog.cpp \
    changeuserpwd.cpp \
    elipsemaskwidget.cpp \
    passwdcheckutil.cpp \
    pwdcheckthread.cpp \
    pwdchangethread.cpp  \
    uniauthservice.cpp \
    xatom-helper.cpp \
    securityquestionanswer.cpp \
    binddialog.cpp \
    messagebar.cpp \
    pwdlessproxy.cpp \
    fontsizeutil.cpp

HEADERS += \
    biometrics.h \
    biometricswidget.h \
    biometricdeviceinfo.h \
    biometricproxy.h \
    biometricenroll.h \
    biometricmoreinfo.h \
    enrollbtn.h \
    giodbus.h \
    kalabel.h \
    qrcodeenroll.h \
    securitykeysetdlg.h \
    servicemanager.h \
    namelabel.h \
    changeuserpwd.h \
    changefeaturename.h \
    changepwddialog.h \
    elipsemaskwidget.h \
    passwdcheckutil.h \  \
    uniauthservice.h \
    xatom-helper.h \
    pwdcheckthread.h \
    pwdchangethread.h \
    securityquestionanswer.h \
    binddialog.h \
    messagebar.h \ 
    pwdlessproxy.h \
    fontsizeutil.h

FORMS += \
    biometricswidget.ui \
    biometricmoreinfo.ui \
    biometricenroll.ui \
    changefeaturename.ui \
    changepwddialog.ui \
    qrcodeenroll.ui

polkit.path = /usr/share/polkit-1/actions/
polkit.files += conf/org.ukui.biometric.manager.security.questions.policy

INSTALLS += target polkit qm_file ts_file

RESOURCES += \
    resource.qrc


