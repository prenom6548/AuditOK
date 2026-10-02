cmake_minimum_required(VERSION 3.16)
project(ukui-shortcut)

add_subdirectory(shortcuts)

include(cmake/UkuiShortCommon.cmake)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui REQUIRED)

include_directories(.)
include_directories(interface)

set(SOURCE_FILES
        short-cut-manager.cpp
        interface/status-info.cpp
        interface/ukui-shortcut-plugin.cpp
        )

set(LIBRARY_HEADERS
        short-cut-manager.h
        interface/value-type.h
        interface/plugin-common-data.h
        interface/ukui-shortcut_global.h
        interface/ukui-shortcut.h
        interface/ukui-shortcut-plugin.h
        interface/status-info.h
        )

set(PC_INSTALL_DIR "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig")
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/${PROJECT_NAME}")
set(LIBRARY_HEADERS_DIR "/usr/include/${PROJECT_NAME}")
set(UKUI_SHORTCUT_LIBRARY_VERSION "1.0.0")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${LIBRARY_HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "1.0.0" SOVERSION 1)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${LIBRARY_HEADERS_DIR}
        LIBRARY DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}"
        )

install(FILES ${LIBRARY_HEADERS} DESTINATION "${LIBRARY_HEADERS_DIR}")

## 生成开发配置文件
include(CMakePackageConfigHelpers)
target_include_directories(${PROJECT_NAME} PUBLIC  $<INSTALL_INTERFACE:${LIBRARY_HEADERS_DIR}>)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR}
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config-version.cmake
        VERSION ${UKUI_SHORTCUT_LIBRARY_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}.pc"
        INSTALL_DESTINATION  ${PC_INSTALL_DIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}.pc DESTINATION ${PC_INSTALL_DIR})

install(EXPORT ${PROJECT_NAME}
        FILE ${PROJECT_NAME}-targets.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
