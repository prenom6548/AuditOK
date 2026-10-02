cmake_minimum_required(VERSION 3.16)

project(test_ukui_style_plugin)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.66.0")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Widgets Gui Test)
find_package(PkgConfig REQUIRED)
find_package(KF5WindowSystem ${KF5_MIN_VERSION} CONFIG REQUIRED COMPONENTS)
pkg_check_modules(QGSETTINGS REQUIRED gsettings-qt)

include_directories(
    ${QGSETTINGS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libqt5-ukui-style
    ${CMAKE_SOURCE_DIR}/ukui-styles
    ${CMAKE_SOURCE_DIR}/ukui-styles/qt5-config-style-ukui
)

add_executable(${PROJECT_NAME} test_ukui_style_plugin.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Test
    qt5-ukui-style
    ${QGSETTINGS_LIBRARIES}
)

# Load the style plugin built in-tree (ukui-styles -> libqt5-style-ukui.so).
add_dependencies(${PROJECT_NAME} qt5-style-ukui)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    UKUI_STYLE_PLUGIN_PATH="$<TARGET_FILE:qt5-style-ukui>")

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
