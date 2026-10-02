cmake_minimum_required(VERSION 3.14)

project(ukui-config-backup-service LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)

set(BACKUP_SERVICE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(GNUInstallDirs)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network DBus Xml REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network DBus Xml REQUIRED)

set(TEMPLATE_INSTALL_PATH /usr/share/ukui/config-backup-service)

configure_file (backup-service-config.h.in backup-service-config.h @ONLY)
set(3rdParties_DIR  ../3rd-parties/qtsingleapplication/src)
set(BACKUP_SERVICE_SOURCES
        ${3rdParties_DIR}/qtsinglecoreapplication.h
        ${3rdParties_DIR}/qtsinglecoreapplication.cpp
        ${3rdParties_DIR}/qtlocalpeer.h
        ${3rdParties_DIR}/qtlocalpeer.cpp
        main.cpp
        log-utils.h
        log-utils.cpp
        service-application.h
        service-application.cpp
        backup-service.cpp
        backup-service.h
        backup-service-private.h
)
if(COMMAND BACKUP_SERVICE_SOURCES)
    qt_add_dbus_adaptor(BACKUP_SERVICE_SOURCES dbus/org.ukui.ConfigBackupService.xml backup-service-private.h UkuiConfigBackup::BackupServicePrivate)
else()
    qt5_add_dbus_adaptor(BACKUP_SERVICE_SOURCES dbus/org.ukui.ConfigBackupService.xml backup-service-private.h UkuiConfigBackup::BackupServicePrivate)

endif()

add_executable(ukui-config-backup-service ${BACKUP_SERVICE_SOURCES})
target_include_directories(ukui-config-backup-service PUBLIC ${3rdParties_DIR})
target_link_libraries(ukui-config-backup-service
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Xml
)

install(TARGETS ukui-config-backup-service DESTINATION /usr/bin)
install(FILES data/org.ukui.ConfigBackupService.service DESTINATION /usr/share/dbus-1/services)
install(FILES data/ukui-backup-config-template.json DESTINATION ${TEMPLATE_INSTALL_PATH})
