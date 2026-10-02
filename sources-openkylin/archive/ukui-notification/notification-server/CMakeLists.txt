set(VERSION_MAJOR 1)
set(VERSION_MINOR 1)
set(VERSION_MICRO 0)
#changelog v1.0.0->v1.1.0：超时时间统一为单位为ms
set(NOTIFICATION_SERVER_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

find_package(PkgConfig REQUIRED)
set(UKUI_NOTIFICATION_PC_PKGS ukui-search)

foreach(PC_LIB IN ITEMS ${UKUI_NOTIFICATION_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND UKUI_NOTIFICATION_EXTERNAL_LIBS ${${PC_LIB}_LIBRARIES})
    endif()
endforeach()

configure_file (notification-server-config.h.in notification-server-config.h @ONLY)
set(3rdParties_DIR  ../3rd-parties/qtsingleapplication/src/)
set(notificationServer_SRCS
    server.h
    server-private.h
    server.cpp
    main.cpp
    ${3rdParties_DIR}qtsinglecoreapplication.h
    ${3rdParties_DIR}qtsinglecoreapplication.cpp
    ${3rdParties_DIR}qtlocalpeer.h
    ${3rdParties_DIR}qtlocalpeer.cpp
    notification-server-application.cpp
    notification-server-application.h
    log-utils.h
    log-utils.cpp
    notification-control-register.h
    notification-control-register.cpp
    notification-manager.cpp
    notification-manager.h
    notification.cpp
    notification.h
    notification-default-dir-watcher.cpp
    notification-default-dir-watcher.h
    notificaiton-settings-query.cpp
    notificaiton-settings-query.h
    security-control-manager.cpp
    security-control-manager.h
)

if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(notificationServer_SRCS ../dbus/org.freedesktop.Notifications.xml server-private.h NotificationServer::ServerPrivate)
    qt_add_dbus_adaptor(notificationServer_SRCS ../dbus/org.ukui.NotificationServer.xml server-private.h NotificationServer::ServerPrivate)
    qt_add_dbus_adaptor(notificationServer_SRCS ../dbus/org.ukui.NotificationSettings.xml server-private.h NotificationServer::ServerPrivate)

else()
    qt5_add_dbus_adaptor(notificationServer_SRCS ../dbus/org.freedesktop.Notifications.xml server-private.h NotificationServer::ServerPrivate)
    qt5_add_dbus_adaptor(notificationServer_SRCS ../dbus/org.ukui.NotificationServer.xml server-private.h NotificationServer::ServerPrivate)
    qt5_add_dbus_adaptor(notificationServer_SRCS ../dbus/org.ukui.NotificationSettings.xml server-private.h NotificationServer::ServerPrivate)

endif()

file(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts)
if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
endif()

add_executable(ukui-notification-server ${notificationServer_SRCS} ${QM_FILES})

include_directories(../libukui-notification)
target_include_directories(ukui-notification-server PUBLIC ${3rdParties_DIR})


target_link_libraries(ukui-notification-server
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Network
        ukui-notification
        ${UKUI_NOTIFICATION_EXTERNAL_LIBS}
        )


set(DBUS_SERVICE_FILE data/org.freedesktop.Notifications.service)
set(DESKTOP_FILE data/ukui-notification-server.desktop)
install(TARGETS ukui-notification-server DESTINATION /usr/bin)
install(FILES ${DBUS_SERVICE_FILE} DESTINATION /usr/share/dbus-1/services)
install(FILES ${DESKTOP_FILE} DESTINATION /etc/xdg/autostart)
install(FILES ${TS_FILES} ${QM_FILES} DESTINATION /usr/share/ukui-notification/notification-server/translations)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/conf/notification-allowlist.json DESTINATION /etc/ukui/ukui-notification/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/conf/ukui-notification-v11.json DESTINATION /opt/system/conf/ukui-security-support/)
