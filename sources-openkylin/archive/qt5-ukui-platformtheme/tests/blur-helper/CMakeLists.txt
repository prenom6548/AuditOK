cmake_minimum_required(VERSION 3.16)

project(test_blur_helper)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.66.0")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Widgets Gui Test X11Extras)
find_package(PkgConfig REQUIRED)
find_package(KF5WindowSystem ${KF5_MIN_VERSION} CONFIG REQUIRED COMPONENTS)
pkg_check_modules(QGSETTINGS REQUIRED gsettings-qt)
pkg_check_modules(KYSDKCONF2 REQUIRED kysdk-conf2)
pkg_check_modules(UKUIWINDOWHELPER REQUIRED kysdk-ukuiwindowhelper)

include_directories(
    ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
    ${QGSETTINGS_INCLUDE_DIRS}
    ${KYSDKCONF2_INCLUDE_DIRS}
    ${UKUIWINDOWHELPER_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/ukui-styles
    ${CMAKE_SOURCE_DIR}/libqt5-ukui-style
)

add_executable(${PROJECT_NAME}
    test_blur_helper.cpp
    ${CMAKE_SOURCE_DIR}/ukui-styles/blur-helper.cpp
    ${CMAKE_SOURCE_DIR}/libqt5-ukui-style/settings/ukui-style-settings.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Test
    Qt5::X11Extras
    KF5::WindowSystem
    ${QGSETTINGS_LIBRARIES}
    ${KYSDKCONF2_LIBRARIES}
    ${UKUIWINDOWHELPER_LIBRARIES}
    qt5-ukui-style
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
