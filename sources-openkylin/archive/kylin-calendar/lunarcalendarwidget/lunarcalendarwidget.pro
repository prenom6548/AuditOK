QT       += core gui sql svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets KWindowSystem dbus network

TARGET          = lunarcalendarwidget
TEMPLATE        = app
QMAKE_CXXFLAGS += -std=c++14
MOC_DIR         = temp/moc
RCC_DIR         = temp/rcc
UI_DIR          = temp/ui
OBJECTS_DIR     = temp/obj
DESTDIR         = $$PWD/../bin
CONFIG          += qt warn_off
RESOURCES       += main.qrc

CONFIG += \
     link_pkgconfig \

PKGCONFIG += gsettings-qt kysdk-waylandhelper kysdk-systime kysdk-qtwidgets
PKGCONFIG += gsettings-qt kysdk-kabase kysdk-datacollect kysdk-ukuiwindowhelper
LIBS +=  -L/usr/lib/kysdk/kysdk-base  -lkylog -lkyconf
INCLUDEPATH += /usr/include/kabase/
INCLUDEPATH += /usr/include/kysdk/applications/kabase/
INCLUDEPATH += /usr/include/kysdk/applications/kabase/kylin_system/
INCLUDEPATH += /usr/include/kabase/kylin_system/
INCLUDEPATH += kabase/
INCLUDEPATH += kabase/Qt
INCLUDEPATH += /path/to/kdk

SOURCES         += main.cpp \
    colorarea.cpp \
#    custommessagebox.cpp \
    custommessagebox.cpp \
    customscrollarea.cpp \
    notificationsadaptor.cpp \
    schedule_item.cpp
SOURCES         += frmlunarcalendarwidget.cpp
SOURCES         += lunarcalendarinfo.cpp
SOURCES         += lunarcalendaritem.cpp
SOURCES         += lunarcalendaryearitem.cpp
SOURCES         += lunarcalendarmonthitem.cpp
SOURCES         += lunarcalendarwidget.cpp
SOURCES         +=
SOURCES         +=
SOURCES         += calendarcolor.cpp
SOURCES         += schedulewidget.cpp
SOURCES         +=
SOURCES         += calendardatabase.cpp
SOURCES         += calendarinfo.cpp
SOURCES         +=
SOURCES         +=

HEADERS         += frmlunarcalendarwidget.h \
    colorarea.h \
#    custommessagebox.h \
    custommessagebox.h \
    customscrollarea.h \
    notificationsadaptor.h \
    schedule_item.h
HEADERS         += lunarcalendarinfo.h
HEADERS         += lunarcalendaritem.h
HEADERS         += lunarcalendaryearitem.h
HEADERS         += lunarcalendarmonthitem.h
HEADERS         += lunarcalendarwidget.h
HEADERS         +=
HEADERS         +=
HEADERS         += calendarcolor.h
HEADERS         += schedulewidget.h
HEADERS         += schedulestruct.h
HEADERS         +=
HEADERS         += calendarinfo.h
HEADERS         += calendardatabase.h
HEADERS         +=
HEADERS         +=

FORMS           += frmlunarcalendarwidget.ui

INCLUDEPATH     += $$PWD

CALENDAR_TRANSLATION_DIR = $${PACKAGE_DATA_DIR}/plugin-calendar/translation
DEFINES += CALENDAR_TRANSLATION_DIR=\\\"$${CALENDAR_TRANSLATION_DIR}\\\"
DEFINES += KY_SDK_DATE
PKGCONFIG += kysdk-systime

qm_files.files = $${QM_FILES}
qm_files.path = $${CALENDAR_TRANSLATION_DIR}
INSTALL += qm_files
