cmake_minimum_required(VERSION 3.16)

project(test_ukui_style_conf_settings)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Test)
find_package(PkgConfig REQUIRED)
pkg_check_modules(KYSDKCONF2 REQUIRED kysdk-conf2)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)

include_directories(
    ${KYSDKCONF2_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libqt5-ukui-style
)

add_executable(${PROJECT_NAME}
    test_ukui_style_conf_settings.cpp
    ${CMAKE_SOURCE_DIR}/libqt5-ukui-style/settings/ukui-style-conf-settings.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Test
    ${KYSDKCONF2_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
