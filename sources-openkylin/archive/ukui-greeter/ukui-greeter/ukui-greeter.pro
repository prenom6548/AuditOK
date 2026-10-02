#-------------------------------------------------
#
# Project created by QtCreator 2017-12-18T09:53:53
#
#-------------------------------------------------

QT  += core gui widgets dbus x11extras svg network sensors

greaterThan(QT_MAJOR_VERSION, 4)

TARGET = ukui-greeter
TEMPLATE = app

PREFIX = /usr/share/ukui-greeter

DEFINES += QT_MESSAGELOGCONTEXT \    #使用qInstallMessageHandler定制日志输出格式
           GREETER_CONFIG=/usr/share/lightdm/ukui-greeter/ \
           RESOURCE_PATH=$${PREFIX} \
           QT_DISABLE_DEPRECATED_BEFORE=0 \
           USEKSCREEN=0 \
           SURPORT_SAVEPARAM=1 \
           APP_API_MAJOR=0 \
           APP_API_MINOR=11 \
           APP_API_FUNC=0

include(display-switch/display-switch.pri)
include(common/common.pri)
include(VirtualKeyboard/VirtualKeyboard.pri)
include(BiometricAuth/BiometricAuth.pri)

INCLUDEPATH += \
            common \
            VirtualKeyboard/src \
            BiometricAuth \
            /usr/include/opencv4

SOURCES += \
    greeter/agreementwindow.cpp \
    greeter/batterywidget.cpp \
    greeter/device.cpp \
    greeter/enginedevice.cpp \
    greeter/greeterservice.cpp \
    greeter/greeterutils.cpp \
    greeter/greeterwindow.cpp \
    greeter/hoverwidget.cpp \
    greeter/iconedit.cpp \
    greeter/imageutil.cpp \
    greeter/klabel.cpp \
    greeter/ktiplabel.cpp \
    greeter/ktooltip.cpp \
    greeter/lockinfo.cpp \
    greeter/loginwindow.cpp \
    greeter/main.cpp \
    greeter/modebutton.cpp \
    greeter/securityuser.cpp \
    greeter/servicemanager.cpp \
    greeter/sessionsview.cpp \
    greeter/pluginsloader.cpp \
    greeter/surewidget.cpp \
    greeter/usersmodel.cpp \
    greeter/greeterwrapper.cpp \
    greeter/screenmodel.cpp \
    greeter/mainwindow.cpp \
    greeter/sessionwindow.cpp \
    greeter/powerwindow.cpp \
    greeter/proxymodel.cpp \
    greeter/userentry.cpp \
    greeter/usersview.cpp \
    greeter/language.cpp \
    greeter/languagewidget.cpp \
    greeter/rootWindowBackground.cpp \
    greeter/pam-tally.c  \
    greeter/mytabwidget.cpp \
    greeter/loginoptionswidget.cpp
    

HEADERS  += \
    greeter/agreementwindow.h \
    greeter/batterywidget.h \
    greeter/device.h \
    greeter/enginedevice.h \
    greeter/greeterservice.h \
    greeter/greeterutils.h \
    greeter/greeterwindow.h \
    greeter/hoverwidget.h \
    greeter/iconedit.h \
    greeter/imageutil.h \
    greeter/klabel.h \
    greeter/ktiplabel.h \
    greeter/ktooltip.h \
    greeter/lockinfo.h \
    greeter/modebutton.h \
    greeter/loginauthinterface.h \
    greeter/loginplugininterface.h \
    greeter/pam-tally.h \
    greeter/loginwindow.h \
    greeter/pluginsloader.h \
    greeter/securityuser.h \
    greeter/servicemanager.h \
    greeter/sessionsview.h \
    greeter/surewidget.h \
    greeter/usersmodel.h \
    greeter/greeterwrapper.h \
    greeter/screenmodel.h \
    greeter/mainwindow.h \
    greeter/sessionwindow.h \
    greeter/powerwindow.h \
    greeter/proxymodel.h \
    greeter/userentry.h \
    greeter/usersview.h \
    greeter/language.h \
    greeter/languagewidget.h \
    greeter/rootWindowBackground.h \
    greeter/mytabwidget.h \
    greeter/loginoptionswidget.h \

CONFIG += c++11 debug link_pkgconfig

LIBS += -lrt -llibnm-icon-kylin -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lukuiinputgatherclient

QMAKE_CXXFLAGS += -Wdeprecated-declarations

PKGCONFIG += liblightdm-qt5-3 \
	     x11 \
	     xrandr \
	     xtst \
             imlib2 \
	     gio-2.0 \
             gio-unix-2.0 \
	     xi \
	     gsettings-qt \
             kylin-nm-base \
             kysdk-sysinfo \
             kysdk-systime

RESOURCES += image.qrc \
             qss.qrc \
             data.qrc

TRANSLATIONS += translations/zh_CN.ts \
                translations/ru.ts \
                translations/fr.ts \
                translations/pt.ts \
                translations/bo_CN.ts \
                translations/tr.ts \
                translations/es.ts \
                translations/mn.ts \
		translations/kk.ts \
		translations/ky.ts \
		translations/ug.ts \
		translations/de.ts \
		translations/es.ts \
		translations/zh_HK.ts 

system("lrelease translations/*.ts")

qm_file.files = translations/*.qm
qm_file.path = $${PREFIX}/translations/

resourcefiles.files = images
resourcefiles.path = $${PREFIX}/

configfile.files = 95-ukui-greeter.conf
configfile.path = /usr/share/lightdm/lightdm.conf.d/

displayfile.files = display-switch/lightdm-session-setup.sh
displayfile.path = /usr/lib/lightdm/

etcfile.files = ukui-greeter.conf
etcfile.path = /etc/lightdm/

kconf2.files = ukui-greeter.yaml
kconf2.path = /usr/share/ukui-greeter/kylin-config/

desktopfile.files = ukui-greeter.desktop
desktopfile.path = /usr/share/xgreeters/

manfile.files = man/ukui-greeter.1.gz
manfile.path = /usr/share/man/man1/

target.path = /usr/sbin/

INSTALLS += target configfile etcfile desktopfile resourcefiles qm_file manfile displayfile kconf2

FORMS += \
    greeter/surewidget.ui
