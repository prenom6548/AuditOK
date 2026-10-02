QT -= gui
QT += network dbus sql
CONFIG += c++11 console 
CONFIG -= app_bundle

TARGET = kylin-daq-daemon
CONFIG += link_pkgconfig

PKGCONFIG += kysdk-log
TEMPLATE = app

LIBS +=  -ldl -lcrypto
#QMAKE_CC += -g
#QMAKE_CXX += -g
#QMAKE_LINK += -g
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
# DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_DEBUG_OUTPUT
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#DEFINES += DEBUG
#if(contains(DEFINES,DEBUG))
#{
#conf.files = conf/kylinDaqConf.ini
#conf.path =$$OUT_PWD
#COPIES += conf
#}

SOURCES += \
#	basic class
    src/BaseModule.cpp \
    src/ConfModule.cpp \
    src/InfoCollectionBase.cpp \
    src/NetworkInfoCollection.cpp \
    src/OnlineInfoCollection.cpp \
    src/RegisterNumberInfoCollection.cpp \
    src/main.cpp \
    src/KylinDaqCommon.cpp\
    src/SingleApplication.cpp \
    src/DataCommon.cpp\
#	conf module
#	basic module
    src/BasicInfoModule.cpp\
    src/BasicInfoCollection.cpp\
#	common module
    src/CommonInfoModule.cpp\
    src/CommonInfoCollection.cpp\
#	cache module
    src/DataBaseInterface.cpp\
    src/CacheModule.cpp\
    src/CacheBase.cpp\
    src/CacheCheck.cpp\
    src/CacheCreate.cpp\
    src/CacheRecv.cpp\
    src/CacheUpload.cpp\
#	transmit module
    src/TransmitModule.cpp\
#	dbus module
    src/DBusModule.cpp\
#	Log module
#    src/LogCheck.cpp \
#    src/LogControl.cpp \
#    src/LogRecord.cpp \
    src/OnlineInfoModule.cpp

HEADERS += \
#	basic class
    src/BaseModule.h \
    src/ConfModule.h \
    src/DBusModule.h \
    src/InfoCollectionBase.h \
    src/KylinDaqCommon.h\
    src/NetworkInfoCollection.h \
    src/OnlineInfoCollection.h \
    src/RegisterNumberInfoCollection.h \
    src/SingleApplication.h \
    src/DataCommon.h\
#	conf Module
#	basic module
    src/BasicInfoModule.h\
    src/BasicInfoCollection.h\
#	common module
    src/CommonInfoModule.h\
    src/CommonInfoCollection.h\
#	cache module
    src/DataBaseInterface.h\
    src/CacheModule.h\
    src/CacheBase.h\
    src/CacheCheck.h\
    src/CacheCreate.h\
    src/CacheRecv.h\
    src/CacheUpload.h\
#	transmit module
    src/TransmitModule.h\
#	dbus module
    src/DBusModule.h\
#	Log module
#    src/LogCheck.h \
#    src/LogControl.h \
#    src/LogRecord.h \
    src/OnlineInfoModule.h

#DEFINES += CACHEFOLDERPATH=\\\"/var/cache/kylin-daq/\\\"
#DEFINES += CACHEFILENAME=\\\"data.db\\\"
#DEFINES += CONFFOLDERPATH=\\\"/etc/kylin-daq/\\\"
#DEFINES += CONFFILENAME=\\\"kylinDaq.ini\\\"


# Default rules for deployment.
unix{
    target.path = /usr/sbin/
    INSTALLS += target

    systemService.files = data/kylin-daq.service
    systemService.path = /usr/lib/systemd/system/
    INSTALLS += systemService

    dbusServer.files = data/com.kylin.daq.service
    dbusServer.path = /usr/share/dbus-1/system-services
    INSTALLS += dbusServer

    dbusConf.files = data/com.kylin.daq.conf
    dbusConf.path  = /etc/dbus-1/system.d/
    INSTALLS += dbusConf

    dbusInterface.files = data/com.kylin.daq.xml
    dbusInterface.path = /usr/share/dbus-1/interfaces/
    INSTALLS += dbusInterface

    kylinDaqConf.files = conf/kylinDaqConf.json
    kylinDaqConf.path = /var/cache/kylin-daq
    INSTALLS += kylinDaqConf

    yaml.files += data/com.kylin.daq.yaml
    yaml.path = /etc/kylin-config/basic/


    limit.files += data/com.kylin.daq.limit
    limit.files += data/com.kylin.daq.limit.verify
    limit.path = /etc/dbus-1/conf/

    limit2.files += data/com.kylin.daq.limit
    limit2.files += data/com.kylin.daq.limit.verify

    limit2.path = /usr/share/dbus-1/conf/
    INSTALLS += limit limit2 yaml


#    testKylinDaq.files = data/tsetKylinDaq.txt
#    testKylinDaq.path = /etc/
#    INSTALLS ++ testKylinDaq

#    systemConf.files = data/kylin-daq.conf
#    systemConf.path = /usr/share/kylin-daq
#    INSTALLS += systemConf

}

RESOURCES += \
    conf.qrc

