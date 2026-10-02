QT += core dbus xml sensors concurrent
LIBS += -lukui-log4qt -lukuiinputgatherclient

PKGCONFIG += kysdk-sysinfo
QMAKE_CXXFLAGS += -Wdeprecated-declarations
TARGET = kylin-status-manager
include(QtSingleApplication/qtsinglecoreapplication.pri)
target.source += $$TARGET
target.path = /usr/bin


INSTALLS += target

CONFIG += c++11 link_pkgconfig qt warn_on
CONFIG -= app_bundle
QMAKE_CXXFLAGS += -Werror=return-type

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
    DbusManager/dbusservice.cpp \
    devicerotationmanager.cpp \
    main.cpp \
    ConfigurationManager/configurationmanager.cpp \
    KmreAppManager/kmremanager.cpp \
    KmreAppManager/kmrenotificationwindowmanager.cpp \
    keyboardmanager/keyboardmanager.cpp \
    DeviceModeManager/devicemodemanager.cpp \
    StatusManager/statusmanager.cpp


HEADERS += \
    DbusManager/dbusservice.h \
    devicerotationmanager.h \
    ConfigurationManager/configurationmanager.h \
    KmreAppManager/kmremanager.h \
    KmreAppManager/kmrenotificationwindowmanager.h \
    keyboardmanager/keyboardmanager.h \
    DeviceModeManager/devicemodemanager.h \
    StatusManager/statusmanager.h

DBUS_ADAPTORS += DbusManager/com.kylin.statusmanager.interface.xml


RESOURCES += \
    src.qrc




#lrelease, .ts
QM_FILES_INSTALL_PATH = /usr/share/$${TARGET}

TRANSLATIONS += \
    translate/zh_CN.ts

qtPrepareTool(QMAKE_LRELEASE, lrelease)
lrelease.name = lrelease
lrelease.input = TRANSLATIONS
lrelease.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_IN_BASE}.qm
lrelease.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
lrelease.CONFIG = no_link

QMAKE_EXTRA_COMPILERS += \
    lrelease

PRE_TARGETDEPS += \
    compiler_lrelease_make_all

qm_files.files = $$replace(TRANSLATIONS, .ts, .qm)
qm_files.path = $$QM_FILES_INSTALL_PATH
qm_files.CONFIG = no_check_exist
INSTALLS += qm_files
