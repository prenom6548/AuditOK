cmake_minimum_required(VERSION 3.13)
#project(libkysdk-system-javascript-httpserver-1.0)
project(libkysdk-system-javascript-websocket)
include(CMakePackageConfigHelpers)

include_directories("${PROJECT_BINARY_DIR}")

set(TARGET kysdk-dbus-websocket)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES 
    kysdk/bios.cpp 
    kysdk/cpuinfo.cpp 
    kysdk/disk.cpp 
    kysdk/gps.cpp 
    kysdk/kysdkdbus.cpp 
    main.cpp 
    kysdk/mainboard.cpp 
    kysdk/net.cpp 
    kysdk/netcard.cpp 
    kysdk/netlink.cpp 
    kysdk/packageinfo.cpp 
    kysdk/peripheralsenum.cpp 
    kysdk/print.cpp 
    kysdk/process.cpp 
    kysdk/resolutionctl.cpp 
    kysdk/resource.cpp 
    kysdk/runinfo.cpp 
    kysdk/sysinfo.cpp 
    shared/websocketclientwrapper.cpp 
    shared/websockettransport.cpp 
    )
set(HEADERS 
    kysdk/bios.h 
    kysdk/cpuinfo.h 
    kysdk/dependence.h 
    kysdk/disk.h 
    kysdk/gps.h 
    kysdk/kysdkdbus.h 
    kysdk/mainboard.h 
    kysdk/net.h 
    kysdk/netcard.h 
    kysdk/netlink.h 
    kysdk/packageinfo.h 
    kysdk/peripheralsenum.h 
    kysdk/print.h 
    kysdk/process.h 
    kysdk/resolutionctl.h 
    kysdk/resource.h 
    kysdk/runinfo.h 
    kysdk/sysinfo.h 
    shared/websocketclientwrapper.h 
    shared/websockettransport.h 
    )
#set(FORMS kysdk/widget.ui)
#set(RESOURCES qrc.qrc)
set(QT Core Gui Widgets DBus Network Concurrent WebChannel WebSockets)
find_package(Qt5 REQUIRED ${QT})
add_executable(${TARGET} ${SOURCES} ${HEADERS} )
target_link_libraries(${TARGET}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::DBus
        Qt5::Network
        Qt5::Concurrent
        Qt5::WebChannel
        Qt5::WebSockets
        )
set (CMAKE_INSTALL_PREFIX /usr)
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
