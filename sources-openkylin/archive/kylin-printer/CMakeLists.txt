cmake_minimum_required(VERSION 3.5)
#Set Project Name
project(kylin-printer LANGUAGES CXX C)

project(${PROJECT_NAME} VERSION 1.3.1.0)

set (APPLET_NAME kylin-printer-applet)
set (LIB_NAME lib-kylin-printer)
set (DBUS_NAME kylin-printer-applet-dbus)
set (ONCE_NAME kylin-printer-applet-once)

include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/printer.cmake)

option(TEST_DEBUG "option for debug" OFF)
option(SHELL_EXEC "option for shell exec" ON)
option("USE_SELF_CONTAINED_DEP" "Using self-contained dependencies" OFF)


if (TEST_DEBUG)
    add_definitions(-DTEST_DEBUG)
    add_definitions(-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
    add_definitions(-DPROJECT_BINARY_DIR=\"${PROJECT_BINARY_DIR}\")
endif(TEST_DEBUG)

if (USE_SELF_CONTAINED_DEP)
    add_definitions(-DUSE_SELF_CONTAINED_DEP)
endif(USE_SELF_CONTAINED_DEP)

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")
add_definitions(-DPROJECT_VERSION=\"${PROJECT_VERSION}\")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
add_compile_options(
    -Werror=return-type
)

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Network DBus REQUIRED)

if (SHELL_EXEC)
    execute_process(COMMAND python3 ${PROJECT_SOURCE_DIR}/python/copy_translations.py
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    execute_process(COMMAND bash ${PROJECT_SOURCE_DIR}/sh/update_translations.sh
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    execute_process(COMMAND bash ${PROJECT_SOURCE_DIR}/sh/apply_translations.sh
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif(SHELL_EXEC)

#add include
include_directories(${PROJECT_SOURCE_DIR}/printer_manager/
                    ${PROJECT_SOURCE_DIR}/device_manager/
                    ${PROJECT_SOURCE_DIR}/ui/
                    ${PROJECT_SOURCE_DIR}/ui/custom_ui/
                    ${PROJECT_SOURCE_DIR}/ui/main_win_ui/
                    ${PROJECT_SOURCE_DIR}/ui/new_pop_window/
                    ${PROJECT_SOURCE_DIR}/util/
                    ${PROJECT_SOURCE_DIR}/dictionary/
                    ${PROJECT_SOURCE_DIR}/common/
                    ${PROJECT_SOURCE_DIR}/avahi_qt/
                    ${PROJECT_SOURCE_DIR}/qt_zeroconf/
                    ${PROJECT_SOURCE_DIR}/ui/property_window/
                    ${PROJECT_SOURCE_DIR}/ui/troubleshooting/
                    ${PROJECT_SOURCE_DIR}/ui/job_manager/
                    ${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/backend
                    ${PROJECT_SOURCE_DIR}/backend/ui
                    )

add_definitions("-Wall")

configure_file(config.h.in
    config.h
    @ONLY
    )

file(GLOB common_SRC
    "common/*"
    "printer_manager/*"
    "device_manager/*"
    "dictionary/*"
    "ui/*.cpp"
    "ui/*.h"
    "ui/main_win_ui/*.cpp"
    "ui/main_win_ui/*.h"
    "ui/new_pop_window/*.cpp"
    "ui/new_pop_window/*.h"
    "ui/custom_ui/*"
    "ui/property_window/*.h"
    "ui/property_window/*.cpp"
    "ui/troubleshooting/*.h"
    "ui/troubleshooting/*.cpp"
    "ui/job_manager/*.h"
    "ui/job_manager/*.cpp"
    "util/*.cpp"
    "util/*.h"
    )

SET(ui_RESOURCE ${PROJECT_SOURCE_DIR}/ui/picture.qrc)


file(GLOB applet_SRC
    "backend/*.cpp"
    "backend/*.h"
    "backend/ui/tray_win.*"
)


add_library(${LIB_NAME}
    ${common_SRC}
    ${ui_RESOURCE}
)

add_executable(${PROJECT_NAME}
    main.cpp
    ${ui_RESOURCE}
)

add_executable(${APPLET_NAME}
    ${applet_SRC}
)

add_executable(${DBUS_NAME}
    "dbus/main.cpp"
    "dbus/dbus.cpp"
    "dbus/dbus.h"
)

add_executable(${ONCE_NAME}
    "once/main.cpp"
)

target_link_libraries(${DBUS_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB_NAME})
target_link_libraries(${APPLET_NAME} PRIVATE ${LIB_NAME})
target_link_libraries(${ONCE_NAME} PRIVATE ${LIB_NAME})

target_link_kysdk_libraries(${LIB_NAME})
target_link_glib_libraries(${LIB_NAME})
target_link_snmp_libraries(${LIB_NAME})
target_link_qgsettings_libraries(${LIB_NAME})
target_link_cups_libraries(${LIB_NAME})
target_link_cupsfilters_libraries(${LIB_NAME})
target_link_avahi_libraries(${LIB_NAME})
target_link_usb_libraries(${LIB_NAME})
target_link_udev_libraries(${LIB_NAME})
target_link_KWindowSystem_libraries(${LIB_NAME})
target_link_smbclient_libraries(${LIB_NAME})
target_link_avahi_libraries(${LIB_NAME})

target_link_libraries(${LIB_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)

#dependencies
target_link_libraries(${LIB_NAME} PUBLIC "-lpthread -lz")
target_link_libraries(${LIB_NAME} PUBLIC "-lboundscheck")
target_link_libraries(${LIB_NAME} PUBLIC "-ldriver-acquirer-widget")



########### install ###############
install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
install(FILES ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.desktop DESTINATION /usr/share/applications/)
install(FILES ${PROJECT_SOURCE_DIR}/data/org.kylin.printer.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
install(FILES ${PROJECT_SOURCE_DIR}/data/kylin-printer.json DESTINATION /etc/ukui/ukui-notification/default-app/)

#applet
install(TARGETS ${APPLET_NAME} DESTINATION /usr/bin/)
install(PROGRAMS ${PROJECT_SOURCE_DIR}/data/kylin-printer-applet-start DESTINATION /usr/share/kylin-printer/)
install(FILES ${PROJECT_SOURCE_DIR}/data/kylin-printer-applet.service DESTINATION /usr/lib/systemd/user/)
install(FILES ${PROJECT_SOURCE_DIR}/backend/data/${APPLET_NAME}.desktop DESTINATION /usr/share/kylin-printer/)

#once
install(TARGETS ${ONCE_NAME} DESTINATION /usr/share/kylin-printer/)

# dbus
install(TARGETS ${DBUS_NAME} DESTINATION /usr/share/kylin-printer/)
install(FILES ${PROJECT_SOURCE_DIR}/data/kylin-printer-applet-dbus.service DESTINATION /usr/lib/systemd/system/)

# udev
install(FILES ${PROJECT_SOURCE_DIR}/data/99-kylin-printer-applet.rules DESTINATION /usr/lib/udev/rules.d/)

# translations
file(GLOB TRANS "${PROJECT_SOURCE_DIR}/translations/*.qm")

install(FILES ${TRANS} DESTINATION  /usr/share/${PROJECT_NAME}/translations/)

# v10 SP1 only
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/guide/${PROJECT_NAME} DESTINATION /usr/share/kylin-user-guide/data/guide/)


#-------------------------------------------test_mode----------------------------------------------------------------#
if (TEST_DEBUG)
    set (TOOL_NAME kylin-printer-tools)

    file(GLOB tool_SRC
    "tools/*.cpp"
    "tools/*.h"
    )

    add_executable(${TOOL_NAME}
        tools/main.cpp
        ${tool_SRC}
    )

    target_link_libraries(${TOOL_NAME} PRIVATE ${LIB_NAME})

endif(TEST_DEBUG)

#-------------------------------------------test_mode-end---------------------------------------------------------------#
