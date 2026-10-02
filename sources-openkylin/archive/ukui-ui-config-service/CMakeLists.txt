cmake_minimum_required(VERSION 3.5)
project(ukui-ui-config-service VERSION 1.0 LANGUAGES C CXX)

include(GNUInstallDirs)
include_directories(src)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_PREFIX "/usr")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets Core Gui DBus REQUIRED)
find_package(PkgConfig)

pkg_check_modules(Qsettings REQUIRED gsettings-qt)
include_directories(${Qsettings_INCLUDE_DIRS})

set(LIBEXEC_PATH ${CMAKE_INSTALL_FULL_LIBEXECDIR})
configure_file(data/com.kylin.ThemeConfig.desktop.in ${CMAKE_CURRENT_SOURCE_DIR}/data/com.kylin.ThemeConfig.desktop)
configure_file(data/com.kylin.ThemeConfig.service.in ${CMAKE_CURRENT_SOURCE_DIR}/data/com.kylin.ThemeConfig.service)
set(ukui-ui-config-service_SRCS
        src/ukuithemeconfig.cpp src/ukuithemeconfig.h
        src/ukui-ui-config-service.cpp
        src/dtconfig/ukuidtparameters.cpp src/dtconfig/ukuidtparameters.h
        src/dtconfig/dtreadconfig.cpp src/dtconfig/dtreadconfig.h
)

add_executable(ukui-ui-config-service ${ukui-ui-config-service_SRCS})

target_link_libraries(ukui-ui-config-service PRIVATE
        Qt5::Core
        Qt5::DBus
        Qt5::Gui
        Qt5::Widgets
        ${GIO_LIBRARIES}
        gsettings-qt
        -lukui-log4qt
)



install(TARGETS ukui-ui-config-service DESTINATION ${CMAKE_INSTALL_FULL_LIBEXECDIR})
install(FILES data/com.kylin.ThemeConfig.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications)
install(FILES data/com.kylin.ThemeConfig.service DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/dbus-1/services)
