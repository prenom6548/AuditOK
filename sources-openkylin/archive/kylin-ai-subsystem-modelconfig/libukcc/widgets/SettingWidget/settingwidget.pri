QT += dbus
INCLUDEPATH += $$PWD $$PWD/../label


CONFIG += link_pkgconfig

PKGCONFIG += kysdk-qtwidgets

SOURCES += \
        $$PWD/addbutton.cpp \
        $$PWD/comboxwidget.cpp \
        $$PWD/lineeditwidget.cpp \
        $$PWD/pushbuttonwidget.cpp \
        $$PWD/radiobuttonwidget.cpp \
        $$PWD/settinggroup.cpp \
        $$PWD/sliderwidget.cpp \
        $$PWD/switchwidget.cpp \
        $$PWD/ukccframe.cpp

HEADERS += \
        $$PWD/addbutton.h \
        $$PWD/comboxwidget.h \
        $$PWD/lineeditwidget.h \
        $$PWD/pushbuttonwidget.h \
        $$PWD/radiobuttonwidget.h \
        $$PWD/settinggroup.h \
        $$PWD/sliderwidget.h \
        $$PWD/switchwidget.h \
        $$PWD/ukccframe.h
