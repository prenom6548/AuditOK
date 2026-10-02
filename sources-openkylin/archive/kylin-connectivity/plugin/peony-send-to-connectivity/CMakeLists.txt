cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(peony-send-to-connectivity)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(RIGHTSEND_SOURCES
        send-to-connectivity.cpp
        send-to-connectivity.h
        send-to-connectivity_global.h
        )

add_library(peony-send-to-connectivity SHARED ${RIGHTSEND_SOURCES})

find_package(Qt5DBus COMPONENTS Dbus REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB
        glib-2.0
        gobject-2.0
        gio-2.0
        )

target_include_directories(peony-send-to-connectivity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(peony-send-to-connectivity PUBLIC /usr/include/peony-qt/)
target_include_directories(peony-send-to-connectivity PUBLIC ${GLIB_INCLUDE_DIRS})

target_link_libraries(peony-send-to-connectivity PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(peony-send-to-connectivity PUBLIC peony)
target_link_libraries(peony-send-to-connectivity PUBLIC ${Qt5DBus_LIBRARIES})
target_link_libraries(peony-send-to-connectivity PUBLIC ${GLIB_LIBRARIES})

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION} /lib/aarch64-linux-gnu/peony-extensions/)
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION} /lib/x86_64-linux-gnu/peony-extensions/)
endif()