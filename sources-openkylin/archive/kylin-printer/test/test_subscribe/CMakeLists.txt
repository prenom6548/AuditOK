cmake_minimum_required(VERSION 3.5)
# Set Project Name
project(test_subscribe LANGUAGES C CXX)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

if (TEST_DEBUG) 
	add_definitions(-DTEST_DEBUG)
    add_definitions(-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
    add_definitions(-DPROJECT_BINARY_DIR=\"${PROJECT_BINARY_DIR}\")
endif(TEST_DEBUG)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools X11Extras Network DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools X11Extras Network DBus REQUIRED)
find_package(KF5WindowSystem REQUIRED)

#Gsetting
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)

set (SOURCE_PATH ../..)

# add ukui log
file(GLOB_RECURSE UKUI_LOG4QT "/usr/lib/libukui-log4qt.so")

#add include 
include_directories(${SOURCE_PATH}/printer_manager/)
include_directories(${SOURCE_PATH}/ui/)
include_directories(${SOURCE_PATH}/ui/main_win_ui/)
include_directories(${SOURCE_PATH}/ui/new_pop_window/)
include_directories(${SOURCE_PATH}/util)
include_directories(${SOURCE_PATH}/dictionary)
include_directories(${SOURCE_PATH}/common)
include_directories(${SOURCE_PATH}/common/custom_ui)
include_directories(${SOURCE_PATH}/backend)
include_directories(
    ${SOURCE_PATH}/avahi_qt/
    ${SOURCE_PATH}/qt_zeroconf/
)

add_definitions("-Wall")

file(GLOB common_SRC 
    "${SOURCE_PATH}/common/*.cpp"
    "${SOURCE_PATH}/common/*.h"
    "${SOURCE_PATH}/common/custom_ui/*.cpp"
    "${SOURCE_PATH}/common/custom_ui/*.h"
    )

file(GLOB avahi_qt_SRC 
    "${SOURCE_PATH}/avahi_qt/*.cpp"
    "${SOURCE_PATH}/avahi_qt/*.h"
    "${SOURCE_PATH}/qt_zeroconf/*.cpp"
    "${SOURCE_PATH}/qt_zeroconf/*.h"
    )

file(GLOB printer_manager_SRC 
    "${SOURCE_PATH}/printer_manager/*.cpp"
    "${SOURCE_PATH}/printer_manager/*.c"
    "${SOURCE_PATH}/printer_manager/*.h"
    )
file(GLOB dictionary_SRC
    "${SOURCE_PATH}/dictionary/*.cpp"
    "${SOURCE_PATH}/dictionary/*.h"
)

file(GLOB ui_SRC 
    "${SOURCE_PATH}/ui/*.cpp"
    "${SOURCE_PATH}/ui/*.h"
    "${SOURCE_PATH}/ui/main_win_ui/*.cpp"
    "${SOURCE_PATH}/ui/main_win_ui/*.h"
    "${SOURCE_PATH}/ui/new_pop_window/*.cpp"
    "${SOURCE_PATH}/ui/new_pop_window/*.h"
    )

file(GLOB util_SRC
    "${SOURCE_PATH}/util/*.cpp"
    "${SOURCE_PATH}/util/*.h"
    )

file(GLOB ${PROJECT_NAME}_SRC
    "main.cpp"
    "test.*"
    "${SOURCE_PATH}/backend/launchPrinter.*"
    "${SOURCE_PATH}/backend/trayWin.*"
    "${SOURCE_PATH}/backend/eventNotifyMonitor.*"
)
SET(ui_RESOURCE ${SOURCE_PATH}/ui/picture.qrc)

QT5_CREATE_TRANSLATION(QM_FILES
    ${ui_SRC}
    ${TS_FILES})

# message(${test_networkdevicebrowse_SRC})
add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SRC}
    ${common_SRC}
    ${printer_manager_SRC}
    ${ui_SRC}
    ${util_SRC}
    ${ui_RESOURCE}
    ${avahi_qt_SRC}
    ${dictionary_SRC}
    ${QM_FILES}
)

target_link_libraries(${PROJECT_NAME} ${UKUI_LOG4QT})

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::X11Extras Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)

#dependencies
target_link_libraries(${PROJECT_NAME} "-lpthread -lz")
#cups
target_link_libraries(${PROJECT_NAME} "-lcups")
#cupsfilter
target_link_libraries(${PROJECT_NAME} "-lcupsfilters")
#deb install
target_link_libraries(${PROJECT_NAME} KF5::WindowSystem)

# avahi
target_link_libraries(${PROJECT_NAME} "-lavahi-common -lavahi-client")

# snmp
find_library(NETSNMPAGENT "netsnmpagent")
find_library(NETSNMPMIBS "netsnmpmibs")
find_library(NETSNMP "netsnmp")

# -lusb-1.0
target_link_libraries(${PROJECT_NAME} "-lusb-1.0")
target_link_libraries(${PROJECT_NAME} "-ludev")
target_link_libraries(${PROJECT_NAME} "-lglib-2.0")

target_link_libraries(${PROJECT_NAME} ${NETSNMPAGENT} ${NETSNMPMIBS} ${NETSNMP})

target_link_libraries(${PROJECT_NAME} -lX11)
target_link_libraries(${PROJECT_NAME} ${UKUI_LOG4QT})
target_link_libraries(${PROJECT_NAME} PkgConfig::QGSETTINGS)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
install(FILES ${PROJECT_SOURCE_DIR}/data/kylin-printer-applet.desktop DESTINATION /etc/xdg/autostart/)
install(FILES ${PROJECT_SOURCE_DIR}/data/org.ukui.log4qt.kylin-printer-applet.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
