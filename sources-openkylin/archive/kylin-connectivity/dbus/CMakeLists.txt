set(DBUS_SOURCES
        dbusvfsinterface.h
        dbusvfsinterface.cpp
        )

add_library(dbus ${DBUS_SOURCES})

target_include_directories(dbus PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(dbus PUBLIC
        ${Qt5DBus_LIBRARIES}
        ui
        common
        )
