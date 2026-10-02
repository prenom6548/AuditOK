cmake_minimum_required(VERSION 3.5)

project(lib-kylin-printer)


set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")


set (SOURCE_PATH ../..)

include(${SOURCE_PATH}/cmake/common.cmake)
include(${SOURCE_PATH}/cmake/printer.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")

add_definitions(-DTEST_DEBUG)
add_definitions(-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_definitions(-DPROJECT_BINARY_DIR=\"${PROJECT_BINARY_DIR}\")

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")
add_definitions(-DPROJECT_VERSION=\"${PROJECT_VERSION}\")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")


find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Network DBus REQUIRED)
include_directories(${SOURCE_PATH}/printer_manager/
                    ${SOURCE_PATH}/device_manager/
                    ${SOURCE_PATH}/ui/
                    ${SOURCE_PATH}/ui/custom_ui/
                    ${SOURCE_PATH}/ui/main_win_ui/
                    ${SOURCE_PATH}/ui/new_pop_window/
                    ${SOURCE_PATH}/util/
                    ${SOURCE_PATH}/dictionary/
                    ${SOURCE_PATH}/common/
                    ${SOURCE_PATH}/avahi_qt/
                    ${SOURCE_PATH}/qt_zeroconf/
                    ${SOURCE_PATH}/ui/new_property_window/
                    ${SOURCE_PATH}/ui/job_manager/
                    ${SOURCE_PATH}/ui/new_property_window/SecondComponent/
                    ${SOURCE_PATH}/include
                    ${SOURCE_PATH}/backend/
                    ${SOURCE_PATH}/backend/ui/
                    )

file(GLOB common_SRC
    "${SOURCE_PATH}/common/*"
    )

file(GLOB printer_manager_SRC
    "${SOURCE_PATH}/printer_manager/*.cpp"
    "${SOURCE_PATH}/printer_manager/*.c"
    "${SOURCE_PATH}/printer_manager/*.h"
    )

file(GLOB device_manager_SRC
    "${SOURCE_PATH}/device_manager/*"
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
    "${SOURCE_PATH}/ui/custom_ui/*"
    "${SOURCE_PATH}/ui/new_property_window/*.h"
    "${SOURCE_PATH}/ui/new_property_window/*.cpp"
    "${SOURCE_PATH}/ui/new_property_window/SecondComponent/*.h"
    "${SOURCE_PATH}/ui/new_property_window/SecondComponent/*.cpp"
    "${SOURCE_PATH}/ui/job_manager/*.h"
    "${SOURCE_PATH}/ui/job_manager/*.cpp"
    )

file(GLOB util_SRC
    "${SOURCE_PATH}/util/*.cpp"
    "${SOURCE_PATH}/util/*.h"
    )

file(GLOB backend_SRC
    "${SOURCE_PATH}/backend/ui/*"
    "${SOURCE_PATH}/backend/device_map.cpp"
    "${SOURCE_PATH}/backend/device_map.h"
    "${SOURCE_PATH}/backend/event_notify_monitor.cpp"
    "${SOURCE_PATH}/backend/event_notify_monitor.h"
    "${SOURCE_PATH}/backend/launch_printer.cpp"
    "${SOURCE_PATH}/backend/launch_printer.h"
    )

file(GLOB include_SRC
    "${SOURCE_PATH}/include/*.h"
    )

add_library(${PROJECT_NAME} STATIC
    ${common_SRC}
    ${printer_manager_SRC}
    ${device_manager_SRC}
    ${ui_SRC}
    ${util_SRC}
    ${ui_RESOURCE}
    ${avahi_qt_SRC}
    ${dictionary_SRC}
    ${backend_SRC}
    ${include_SRC}
)

target_link_kysdk_libraries(${PROJECT_NAME})
target_link_glib_libraries(${PROJECT_NAME})
target_link_snmp_libraries(${PROJECT_NAME})
target_link_qgsettings_libraries(${PROJECT_NAME})
target_link_cups_libraries(${PROJECT_NAME})
target_link_cupsfilters_libraries(${PROJECT_NAME})
target_link_avahi_libraries(${PROJECT_NAME})
target_link_usb_libraries(${PROJECT_NAME})
target_link_udev_libraries(${PROJECT_NAME})
target_link_KWindowSystem_libraries(${PROJECT_NAME})
target_link_smbclient_libraries(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)

target_link_libraries(${PROJECT_NAME} "-lpthread -lz")

target_link_libraries(${PROJECT_NAME} "-lboundscheck")

target_link_libraries(${PROJECT_NAME} "-lgcov")
