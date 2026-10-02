cmake_minimum_required(VERSION 3.16)

project(printerbackend)

set (SOURCE_PATH ${CMAKE_CURRENT_LIST_DIR})

include(${SOURCE_PATH}/cmake/common.cmake)
include(${SOURCE_PATH}/cmake/printer.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Network DBus REQUIRED)

execute_process(COMMAND bash ${SOURCE_PATH}/translations/generate_translations_qm.sh
    WORKING_DIRECTORY ${SOURCE_PATH})

#add include 
include_directories(
    ${SOURCE_PATH}/backend/
    ${SOURCE_PATH}/device_manager/
    ${SOURCE_PATH}/printer_manager/
    ${SOURCE_PATH}/device_manager/
    ${SOURCE_PATH}/ui/
    ${SOURCE_PATH}/ui/new_pop_window/
    ${SOURCE_PATH}/util
    ${SOURCE_PATH}/common
    ${SOURCE_PATH}/../../../kom/
    ${SOURCE_PATH}
    )

file(GLOB SRC
    "${SOURCE_PATH}/backend/*"
    "${SOURCE_PATH}/common/*.cpp"
    "${SOURCE_PATH}/common/*.h"
    "${SOURCE_PATH}/printer_manager/*.cpp"
    "${SOURCE_PATH}/printer_manager/*.c"
    "${SOURCE_PATH}/printer_manager/*.h"
    "${SOURCE_PATH}/device_manager/*"
    "${SOURCE_PATH}/ui/*.cpp"
    "${SOURCE_PATH}/ui/*.h"
    "${SOURCE_PATH}/ui/new_pop_window/*.cpp"
    "${SOURCE_PATH}/ui/new_pop_window/*.h"
    "${SOURCE_PATH}/util/*.cpp"
    "${SOURCE_PATH}/util/*.h"
    )

add_library(${PROJECT_NAME} STATIC
    ${SRC}
    printer_backend.cpp
    printer_backend.h
)


message("========================= Use Compatibility: ${PROJECT_NAME} =========================")
set(COMPATIBILITY_NAME compatibility)

set(COMPATIBILITY_DIR ${SOURCE_PATH}/../../../${COMPATIBILITY_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-alm)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-conf)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-datacollect)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-devicesec)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-diagnostics)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-kabase)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-log)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-qtwidgets)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-systime)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-ukenv)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-utils)
target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-waylandhelper)
target_link_libraries(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_NAME})


target_link_libraries(${PROJECT_NAME} PRIVATE kom)
target_link_glib_libraries(${PROJECT_NAME})
target_link_qgsettings_libraries(${PROJECT_NAME})
target_link_cups_libraries(${PROJECT_NAME})
target_link_cupsfilters_libraries(${PROJECT_NAME})
target_link_usb_libraries(${PROJECT_NAME})
target_link_udev_libraries(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)

#dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE "-lpthread -lz")

file(GLOB TRANS "${SOURCE_PATH}/translations/*.qm")
install(FILES ${TRANS} DESTINATION  /usr/share/kylin-os-manager/daemons/sessionservice/printerbackend/translations)
install(FILES ${SOURCE_PATH}/data/kylin-os-manager-printer-settings.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
