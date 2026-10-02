project(notifications VERSION 1.0)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core DBus)

set(CMAKE_AUTOMOC ON)

set(NOTIFICATIONS_SOURCES
    notification-control.cpp
    notification-control.h
)

set(NOTIFICATION_CONTROL_XML
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/org.ukui.Framework.Notifications.Control.xml
)

if (COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(NOTIFICATIONS_SOURCES
        ${NOTIFICATION_CONTROL_XML}
        notification-control.h
        NotificationControl
        notificationcontroladaptor
        NotificationControlAdaptor
    )
else ()
    qt5_add_dbus_adaptor(NOTIFICATIONS_SOURCES
        ${NOTIFICATION_CONTROL_XML}
        notification-control.h
        NotificationControl
        notificationcontroladaptor
        NotificationControlAdaptor
    )
endif()

add_library(notifications STATIC ${NOTIFICATIONS_SOURCES})

target_include_directories(notifications
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(notifications
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus)
