cmake_minimum_required(VERSION 3.16)

project(test_config-checkbox-animation-helper)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS " -fcommon")

set(QT_MIN_VERSION 5.12.0)
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Widgets Core Test)

# 获取项目根目录
set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})

# 添加头文件路径
include_directories(
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui
    ${PROJECT_ROOT_DIR}/ukui-styles
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style
)

# 添加被测源代码文件
set(ANIMATION_HELPER_SOURCES
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-checkbox-animation-helper.cpp
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-checkbox-animation-helper.h
)

# 添加依赖的源文件
set(DEPENDENCY_SOURCES
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-checkbox-animator.cpp
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-checkbox-animator.h
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/animation-helper.cpp
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/animation-helper.h
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/animator-iface.h
)

set(TEST_SOURCES
    main.cpp
    test_config-checkbox-animation-helper.cpp
    test_config-checkbox-animation-helper.h
    ${ANIMATION_HELPER_SOURCES}
    ${DEPENDENCY_SOURCES}
)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Test
)
