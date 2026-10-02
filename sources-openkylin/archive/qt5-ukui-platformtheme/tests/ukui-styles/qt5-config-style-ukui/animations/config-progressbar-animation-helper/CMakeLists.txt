cmake_minimum_required(VERSION 3.16)


project(test_config_progressbar_animation_helper)

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
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui
    ${PROJECT_ROOT_DIR}/ukui-styles
)

# 添加被测源代码文件
set(ANIMATION_HELPER_SOURCES
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-progressbar-animation-helper.cpp
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-progressbar-animation-helper.h
)

set(TEST_SOURCES
    test_config-progressbar-animation-helper.cpp
    test_config-progressbar-animation-helper.h
    main.cpp
    ${ANIMATION_HELPER_SOURCES}
)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::Test
    Qt5::Core
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
