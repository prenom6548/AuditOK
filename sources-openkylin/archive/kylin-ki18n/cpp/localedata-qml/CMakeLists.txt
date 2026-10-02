cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Qml)

# KI18N_LOCALEDATA_HEADERS_DIR 由顶层 CMakeLists 定义

add_library(ki18nlocaledataqmlplugin MODULE ki18nlocaledataqmlplugin.cpp)

target_include_directories(ki18nlocaledataqmlplugin PRIVATE
    ${CMAKE_SOURCE_DIR}/cpp/include
    ${CMAKE_SOURCE_DIR}/cpp/localedata
    ${KI18N_LOCALEDATA_HEADERS_DIR}
)

target_link_libraries(ki18nlocaledataqmlplugin PRIVATE
    Qt5::Core
    Qt5::Qml
    KF5I18nLocaleData
)

set_target_properties(ki18nlocaledataqmlplugin PROPERTIES PREFIX "")

add_dependencies(ki18nlocaledataqmlplugin KF5I18nLocaleData)

# 安装见根目录 cmake/kf5_install_targets.cmake
