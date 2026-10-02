cmake_minimum_required(VERSION 3.16)


project(test_config-tree-animator)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
    Test
    Core
)

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
set(ANIMATOR_SOURCES
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-tree-animator.cpp
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-tree-animator.h
)

set(TEST_SOURCES
    test_config-tree-animator.cpp
    test_config-tree-animator.h
    main.cpp
    ${ANIMATOR_SOURCES}
)

add_executable(test_config-tree-animator ${TEST_SOURCES})

target_link_libraries(test_config-tree-animator PRIVATE
    Qt5::Widgets
    Qt5::Test
    Qt5::Core
)

add_test(NAME test_config-tree-animator COMMAND test_config-tree-animator)
