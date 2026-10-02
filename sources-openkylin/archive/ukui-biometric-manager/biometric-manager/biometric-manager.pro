#-------------------------------------------------
#
# Project created by QtCreator 2017-07-10T09:27:42
#
#-------------------------------------------------

QT       += core gui dbus KWindowSystem svg x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += APP_API_MAJOR=0  \
            APP_API_MINOR=11    \
            APP_API_FUNC=0


INCLUDEPATH +=\
	    src

PREFIX = /usr/share/biometric-manager
LIBS +=-lpthread
LIBS +=-lukui-log4qt


include ($$PWD/qt-solutions/qtsingleapplication/src/qtsingleapplication.pri)

TARGET = biometric-manager
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += x11 gsettings-qt gio-2.0 gio-unix-2.0 kysdk-waylandhelper kysdk-ukuiwindowhelper kysdk-qtwidgets kysdk-sysinfo

SOURCES += src/main.cpp\
    preloadwidget.cpp \
    src/lightlabel.cpp \
    src/loadingdialog.cpp \
    src/KAButton.cpp \
    src/attributewindow.cpp \
    src/giodbus.cpp \
    src/imageutil.cpp \
    src/kalabel.cpp \
    src/leftsiderbarwidget.cpp \
    src/mainwindow.cpp \
    src/customtype.cpp \
    src/mytreeview.cpp \
    src/promptdialog.cpp \
    src/contentpane.cpp \
    src/treeitem.cpp \
    src/treemodel.cpp \
    src/inputdialog.cpp \
    src/messagedialog.cpp \
    src/configuration.cpp \
    src/servicemanager.cpp \
    src/uniauthservice.cpp


HEADERS  += src/mainwindow.h \
    preloadwidget.h \
    src/lightlabel.h \
    src/loadingdialog.h \
    src/KAButton.h \
    src/attributewindow.h \
    src/customtype.h \
    src/giodbus.h \
    src/imageutil.h \
    src/kalabel.h \
    src/leftsiderbarwidget.h \
    src/mytreeview.h \
    src/promptdialog.h \
    src/contentpane.h \
    src/treeitem.h \
    src/treemodel.h \
    src/inputdialog.h \
    src/messagedialog.h \
    src/configuration.h \
    src/servicemanager.h \
    src/uniauthservice.h


FORMS    += src/attributewindow.ui \
    src/promptdialog.ui \
    src/contentpane.ui \
    src/inputdialog.ui \
    src/messagedialog.ui \
    src/stackedWidget.ui


RESOURCES += \
    assets.qrc

TRANSLATIONS += i18n_ts/zh_CN.ts \
                i18n_ts/fr.ts \
                i18n_ts/pt.ts \
                i18n_ts/ru.ts \
                i18n_ts/tr.ts \
                i18n_ts/es.ts \
                i18n_ts/bo_CN.ts \
		i18n_ts/kk.ts \
		i18n_ts/ky.ts \
		i18n_ts/ug.ts \
		i18n_ts/mn.ts \
		i18n_ts/de.ts \
		i18n_ts/zh_HK.ts \
		i18n_ts/vi.ts \
		i18n_ts/ms.ts \
		i18n_ts/th.ts

system("lrelease i18n_ts/*.ts")

qm_file.files = i18n_ts/*.qm
qm_file.path = $${PREFIX}/i18n_qm/

target.path = /usr/bin/

ICON.files = assets/biometric-manager.png
ICON.path = /usr/share/pixmaps/

desktop.files = data/biometric-manager.desktop
desktop.path = /usr/share/applications/

INSTALLS += target qm_file ICON desktop
