set(VERSION_MAJOR 1)
set(VERSION_MINOR 1)
set(VERSION_MICRO 0)
#changelog v1.0.0->v1.1.0：超时时间统一为单位为ms
set(UKUI_NOTIFICATION_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

include(FeatureSummary)

find_package(ECM)
option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" TRUE)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMAddQch)

find_package(PkgConfig REQUIRED)
set(UKUI_NOTIFICATION_EXTERNAL_LIBS "")
set(UKUI_NOTIFICATION_PC_PKGS ukui-search)

foreach(PC_LIB IN ITEMS ${UKUI_NOTIFICATION_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND UKUI_NOTIFICATION_EXTERNAL_LIBS ${${PC_LIB}_LIBRARIES})
    endif()
endforeach()

include_directories(notification-settings)
set(ukui-notification_LIB_SRCS
        notification-client.cpp
        notification-client.h
        popup-notification.h
        popup-notification.cpp
        ukui-notification_global.h
        notification-client-private.h
        utils.h
        utils.cpp
        notification-close-reason.h
        security-control-config.h
        security-control-config.cpp
        notification-settings/settings-properties.h
        notification-settings/settings-properties-info.h
        notification-settings/settings-properties-info.cpp
        notification-settings/settings-manager-private.h
        notification-settings/settings-manager.h
        notification-settings/settings-manager.cpp
        notification-settings/notification-global-settings.cpp
        notification-settings/notification-global-settings.h
        notification-settings/single-application-settings.cpp
        notification-settings/single-application-settings.h
        notification-settings/applications-settings.cpp
        notification-settings/applications-settings.h
        control-config.cpp
        control-config.h)
set(HEADERS
        notification-client.h
        popup-notification.h
        notification-close-reason.h
        notification-settings/settings-properties.h
        notification-settings/settings-properties-info.h
        notification-settings/notification-global-settings.h
        notification-settings/single-application-settings.h
        notification-settings/applications-settings.h
        ukui-notification_global.h
        utils.h)
if(COMMAND qt_add_dbus_interface)
    qt_add_dbus_interface(ukui-notification_LIB_SRCS ../dbus/org.freedesktop.Notifications.xml notifications_interface)
    qt_add_dbus_adaptor(ukui-notification_LIB_SRCS ../dbus/org.ukui.NotificationClient.xml notification-client-private.h UkuiNotification::NotificationClientPrivate)
else()
    qt5_add_dbus_interface(ukui-notification_LIB_SRCS ../dbus/org.freedesktop.Notifications.xml notifications_interface)
    qt5_add_dbus_adaptor(ukui-notification_LIB_SRCS ../dbus/org.ukui.NotificationClient.xml notification-client-private.h UkuiNotification::NotificationClientPrivate)
endif()
add_library(ukui-notification SHARED ${ukui-notification_LIB_SRCS})
target_link_libraries(ukui-notification
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        ${UKUI_NOTIFICATION_EXTERNAL_LIBS}
        ukui-quick::platform
        )

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/ukui-notification")
set(HEADERS_INSTALL_DIR /usr/include/ukui-notification)
set(PC_INSTALL_DIR "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig")

target_include_directories(ukui-notification PUBLIC  $<INSTALL_INTERFACE:${HEADERS_INSTALL_DIR}>)
if (BUILD_QCH)
    set(QCH_INSTALL_DIR "/usr/share/qt5/doc/")
    ecm_add_qch(
            ukui-notification-qch
            NAME ukui-notification
            BASE_NAME ukui-notification
            VERSION "${UKUI_NOTIFICATION_VERSION}"
            ORG_DOMAIN org.ukui
            SOURCES ${HEADERS}
            MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
            LINK_QCHS
            INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
            TAGFILE_INSTALL_DESTINATION ${QCH_INSTALL_DIR}
            QCH_INSTALL_DESTINATION ${QCH_INSTALL_DIR}
            COMPONENT Devel
    )
    ecm_install_qch_export(
            TARGETS ukui-notification-qch
            FILE ukui-notification-qch-targets.cmake
            DESTINATION "${CMAKE_CONFIG_INSTALL_DIR}"
            COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/ukui-notification-qch-targets.cmake\")")
endif()


configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-notification-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-notification-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR})
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-notification-config-version.cmake
        VERSION ${UKUI_NOTIFICATION_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-notification.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-notification.pc"
        INSTALL_DESTINATION  ${PC_INSTALL_DIR})
set_target_properties(ukui-notification PROPERTIES
        VERSION ${UKUI_NOTIFICATION_VERSION}
        SOVERSION ${VERSION_MAJOR}
        OUTPUT_NAME ukui-notification
        )
install(TARGETS ukui-notification
        EXPORT ukui-notification
        PUBLIC_HEADER DESTINATION ${HEADERS_INSTALL_DIR}
        LIBRARY DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        )
install(EXPORT ukui-notification
        FILE ukui-notification-targets.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

install(FILES ${HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-notification.pc DESTINATION ${PC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-notification-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/ukui-notification-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

if(BUILD_TEST)
    add_subdirectory(autotest)
endif ()
