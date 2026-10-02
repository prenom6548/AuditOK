cmake_minimum_required(VERSION 3.16)
project(search-config-test LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core DBus Test REQUIRED)
pkg_check_modules(GIO REQUIRED gio-2.0)

get_filename_component(FRAMEWORK_DBUS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../" ABSOLUTE)

include_directories(
    ..
    ${FRAMEWORK_DBUS_DIR}/common/QGSettings
    ${GIO_INCLUDE_DIRS}
)

add_executable(search-config-test
    search-config-test.cpp
    ../search-config.cpp
    ../search-config.h
    ${FRAMEWORK_DBUS_DIR}/common/QGSettings/qgsettings.cpp
    ${FRAMEWORK_DBUS_DIR}/common/QGSettings/qgsettings.h
    ${FRAMEWORK_DBUS_DIR}/common/QGSettings/qconftype.cpp
    ${FRAMEWORK_DBUS_DIR}/common/QGSettings/qconftype.h
)

target_link_libraries(search-config-test
    Qt5::Core
    Qt5::DBus
    Qt5::Test
    gio-2.0
    ${GIO_LIBRARIES}
)
