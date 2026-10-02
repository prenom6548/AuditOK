QT -= gui
QT += dbus

CONFIG += c++11 console
CONFIG -= app_bundle
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
SOURCES += \
        clockprovider.cpp \
        main.cpp \

# Default rules for deployment.
qnx: target.path = /usr/bin
else: unix:!android: target.path = /usr/bin


clockservice.files += provider/org.ukui.appwidget.provider.democlock.service
clockservice.path += /usr/share/dbus-1/services/
!isEmpty(target.path): INSTALLS += target clockservice


HEADERS += \
    clockprovider.h \

RESOURCES += \
    provider/src.qrc

#QMAKE_LFLAGS += -Wl,-rpath=$$OUT_PWD/../../../libukui-app-widget/libukui-appwidget-manager/
#QMAKE_LFLAGS += -Wl,-rpath=$$OUT_PWD/../../../libukui-app-widget/libukui-appwidget-provider/

LIBS += -L$$OUT_PWD/../../../libukui-app-widget/libukui-appwidget-manager -lukui-appwidget-manager
LIBS += -L$$OUT_PWD/../../../libukui-app-widget/libukui-appwidget-provider -lukui-appwidget-provider

INCLUDEPATH += $$PWD/../../../libukui-app-widget/libukui-appwidget-manager
DEPENDPATH += $$PWD/../../../libukui-app-widget/libukui-appwidget-manager

INCLUDEPATH += $$PWD/../../../libukui-app-widget/libukui-appwidget-provider
DEPENDPATH += $$PWD/../../../libukui-app-widget/libukui-appwidget-provider
