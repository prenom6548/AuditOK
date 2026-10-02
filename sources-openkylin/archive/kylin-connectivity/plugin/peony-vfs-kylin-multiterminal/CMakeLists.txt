cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(peony-vfs-kylin-multiterminal)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(VFS_SOURCES
        multiterminal-vfs-dbus-interface.cpp
        multiterminal-vfs-dbus-interface.h
        multiterminal-vfs-file-enumerator.cpp
        multiterminal-vfs-file-enumerator.h
        multiterminal-vfs-file-monitor.cpp
        multiterminal-vfs-file-monitor.h
        multiterminal-vfs-file.cpp
        multiterminal-vfs-file.h
        multiterminal-vfs-register.cpp
        multiterminal-vfs-register.h
        peony-vfs-multiterminal.cpp
        peony-vfs-multiterminal.h
        )

add_library(peony-vfs-kylin-multiterminal SHARED ${VFS_SOURCES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB
        glib-2.0
        gobject-2.0
        gio-2.0
        )

target_include_directories(peony-vfs-kylin-multiterminal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(peony-vfs-kylin-multiterminal PUBLIC /usr/include/peony-qt/)
target_include_directories(peony-vfs-kylin-multiterminal PUBLIC ${GLIB_INCLUDE_DIRS})

target_link_libraries(peony-vfs-kylin-multiterminal PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(peony-vfs-kylin-multiterminal PUBLIC peony)
target_link_libraries(peony-vfs-kylin-multiterminal PUBLIC ${GLIB_LIBRARIES})
target_link_libraries(peony-vfs-kylin-multiterminal PUBLIC common)

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION} /lib/aarch64-linux-gnu/peony-extensions/)
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION} /lib/x86_64-linux-gnu/peony-extensions/)
endif()