set(CONNECTMANAGE_SOURCES
        wifimanager.h
        wifimanager.cpp
        usbmanage.h
        usbmanage.cpp
        connectionservice.h
        connectionservice.cpp
        messageserialize.pb.h
        messageserialize.pb.cc
        )

add_library(connectmanage  ${CONNECTMANAGE_SOURCES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB
        glib-2.0
        gobject-2.0
        gio-2.0
        protobuf
        )

target_include_directories(connectmanage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(connectmanage PUBLIC ${GLIB_INCLUDE_DIRS})

target_link_libraries(connectmanage PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        ${GLIB_LIBRARIES}
        common
        )
