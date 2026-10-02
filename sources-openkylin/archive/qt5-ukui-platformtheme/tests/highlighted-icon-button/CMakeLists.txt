cmake_minimum_required(VERSION 3.16)

project(test_highlighted_icon_button)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(QT_MIN_VERSION "5.12.0")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Test
)

# 只编译单元测试文件
set(TEST_SOURCES
    test_highlighted-icon-button.cpp
)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Test
)

add_test(NAME test_highlighted_icon_button COMMAND ${PROJECT_NAME})
