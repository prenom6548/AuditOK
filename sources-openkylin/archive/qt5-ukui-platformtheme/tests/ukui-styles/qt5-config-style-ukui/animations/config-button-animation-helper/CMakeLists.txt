cmake_minimum_required(VERSION 3.16)

# 添加覆盖率参数

project(test_config_button_animation_helper)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Widgets
    Test
    Core
)

# 获取项目根目录
set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})

# 添加头文件路径
include_directories(
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style
)

# 添加被测源代码文件
set(ANIMATION_HELPER_SOURCES
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-button-animation-helper.cpp
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-button-animation-helper.h
)

# 添加依赖的源文件
set(DEPENDENCY_SOURCES
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/animation-helper.cpp
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/animation-helper.h
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/animator-iface.h
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/scrollbar/ukui-scrollbar-default-interaction-animator.cpp
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/scrollbar/ukui-scrollbar-default-interaction-animator.h
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-button-animator.cpp
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-button-animator.h
)

set(TEST_SOURCES
    main.cpp
    test_config-button-animation-helper.cpp
    test_config-button-animation-helper.h
    ${ANIMATION_HELPER_SOURCES}
    ${DEPENDENCY_SOURCES}
)

add_executable(test_config-button-animation-helper ${TEST_SOURCES})

target_link_libraries(test_config-button-animation-helper PRIVATE
    Qt5::Widgets
    Qt5::Test
    Qt5::Core
)

add_test(NAME test_config_button_animation_helper COMMAND test_config-button-animation-helper)
