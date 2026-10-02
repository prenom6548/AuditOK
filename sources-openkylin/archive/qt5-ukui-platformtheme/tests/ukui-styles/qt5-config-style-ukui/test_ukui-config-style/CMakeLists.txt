cmake_minimum_required(VERSION 3.16)

project(test_ukui_config_style)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.66.0")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Test
    DBus
    X11Extras
)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(KF5WindowSystem ${KF5_MIN_VERSION} CONFIG REQUIRED COMPONENTS)
pkg_check_modules(QGSETTINGS REQUIRED gsettings-qt)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0)
pkg_check_modules(UKUIWINDOWHELPER REQUIRED kysdk-ukuiwindowhelper)
pkg_check_modules(KYSDKCONF2 REQUIRED kysdk-conf2)
pkg_check_modules(XCB REQUIRED xcb)

include_directories(
    ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5DBus_PRIVATE_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${QGSETTINGS_INCLUDE_DIRS}
    ${UKUIWINDOWHELPER_INCLUDE_DIRS}
    ${KYSDKCONF2_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/ukui-styles/qt5-config-style-ukui
    ${CMAKE_SOURCE_DIR}/ukui-styles/qt5-config-style-ukui/widget-parameters
    ${CMAKE_SOURCE_DIR}/ukui-styles
    ${CMAKE_SOURCE_DIR}/libqt5-ukui-style
)

# Include UKUIConfigStyle and helper source files
file(GLOB_RECURSE UKUI_CONFIG_STYLE_SOURCES
    ${CMAKE_SOURCE_DIR}/ukui-styles/qt5-config-style-ukui/*.cpp
    ${CMAKE_SOURCE_DIR}/ukui-styles/blur-helper.cpp
    ${CMAKE_SOURCE_DIR}/ukui-styles/window-manager.cpp
    ${CMAKE_SOURCE_DIR}/ukui-styles/gesture-helper.cpp
    ${CMAKE_SOURCE_DIR}/ukui-styles/readconfig.cpp
)

add_executable(${PROJECT_NAME}
    test_ukui_config_style.cpp
    ${UKUI_CONFIG_STYLE_SOURCES}
)

add_executable(test_ukui_config_style_offscreen
    test_ukui_config_style_offscreen.cpp
    ${UKUI_CONFIG_STYLE_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Test
    Qt5::DBus
    Qt5::X11Extras
    KF5::WindowSystem
    ${QGSETTINGS_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${UKUIWINDOWHELPER_LIBRARIES}
    ${KYSDKCONF2_LIBRARIES}
    ${XCB_LIBRARIES}
    qt5-ukui-style
)

target_link_libraries(test_ukui_config_style_offscreen PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Test
    Qt5::DBus
    Qt5::X11Extras
    KF5::WindowSystem
    ${QGSETTINGS_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${UKUIWINDOWHELPER_LIBRARIES}
    ${KYSDKCONF2_LIBRARIES}
    ${XCB_LIBRARIES}
    qt5-ukui-style
)

add_test(NAME test_ukui_config_style COMMAND ${PROJECT_NAME})
add_test(NAME test_ukui_config_style_offscreen COMMAND test_ukui_config_style_offscreen)