QT += core testlib dbus
	
CONFIG += c++11 console link_pkgconfig
	
PKGCONFIG += gsettings-qt
		
SOURCES += \
    ../registeredQDbus/sysdbusregister.cpp\
    ../registeredQDbus/brightness-control/brightnessnode.cpp \	
    ../registeredQDbus/cpu-control/upmcpucontrol.cpp \	
    ../registeredQDbus/cpu-control/upmcpucoreinfo.cpp \	
    ../registeredQDbus/cpu-control/upmcpuoccupy.cpp \	
    ../registeredQDbus/dev-power-control/devpowercontrol.cpp \	
    ../registeredQDbus/hardware-power-control/hardwarepowercontrol.cpp \	
    ../registeredQDbus/powerconfig.cpp \	
    ../registeredQDbus/dev-power-control/devicepowerinfo.cpp \	
    ../registeredQDbus/dev-power-control/i2cdevice.cpp \	
    ../registeredQDbus/dev-power-control/pcidevice.cpp \	
    ../registeredQDbus/dev-power-control/satadevice.cpp \	
    ../registeredQDbus/upmcommonfile.cpp \	
    ../registeredQDbus/upmcustomhardwareconfig.cpp \	
    pciDeviceTest.cpp \
    upmCpuControlTest.cpp \
    sysDbusRegisterTest.cpp \
    upmCustomHardwareConfigTest.cpp \
    hardWarePowerControlTest.cpp \
    devicePowerInfoTest.cpp \
    i2cDeviceTest.cpp \
    sataDeviceTest.cpp \
    brightnessNodeTest.cpp \
    main.cpp

HEADERS += \
    ../registeredQDbus/sysdbusregister.h\
    ../registeredQDbus/brightness-control/brightnessnode.h \
    ../registeredQDbus/common.h \
    ../registeredQDbus/cpu-control/upmcpucontrol.h \	
    ../registeredQDbus/cpu-control/upmcpucoreinfo.h \
    ../registeredQDbus/cpu-control/upmcpuoccupy.h \
    ../registeredQDbus/dev-power-control/devpowercontrol.h \
    ../registeredQDbus/hardware-power-control/hardwarepowercontrol.h \
    ../registeredQDbus/powerconfig.h \
    ../registeredQDbus/sysdbusregister.h \
    ../registeredQDbus/dev-power-control/devicepowerinfo.h \
    ../registeredQDbus/dev-power-control/i2cdevice.h \
    ../registeredQDbus/dev-power-control/pcidevice.h \
    ../registeredQDbus/dev-power-control/satadevice.h \
    ../registeredQDbus/upmcommonfile.h \
    ../registeredQDbus/upmcustomhardwareconfig.h 

LIBS += -L/usr/lib -lgtest -lgtest_main -pthread -lpci
	
INCLUDEPATH += /usr/include/gtest

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage -fno-access-control
QMAKE_CFLAGS += -fprofile-arcs -ftest-coverage -fno-access-control
QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage -fno-access-control
QMAKE_CXXFLAGS += --coverage

CONFIG += link_pkgconfig
LIBS += -lgcov     