cmake_minimum_required(VERSION 3.16)

# 添加覆盖率参数

project(test_config-scrollbar-animation-helper)

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

include_directories(${Qt5Widgets_PRIVATE_INCLUDE_DIRS})
include_directories(${Qt5Test_PRIVATE_INCLUDE_DIRS})

# 获取项目根目录
set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})

# 添加头文件路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui
    ${PROJECT_ROOT_DIR}/ukui-styles
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style
)

# 添加被测源代码文件
set(ANIMATION_HELPER_SOURCES
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-scrollbar-animation-helper.cpp
    ${PROJECT_ROOT_DIR}/ukui-styles/qt5-config-style-ukui/animations/config-scrollbar-animation-helper.h
)

# 添加依赖的源文件
set(DEPENDENCY_SOURCES
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/animation-helper.cpp
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/animation-helper.h
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/animator-iface.h
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/scrollbar/ukui-scrollbar-default-interaction-animator.cpp
    ${PROJECT_ROOT_DIR}/libqt5-ukui-style/animations/scrollbar/ukui-scrollbar-default-interaction-animator.h
)

# 创建可执行文件
add_executable(test_config-scrollbar-animation-helper
    main.cpp
    test_config-scrollbar-animation-helper.cpp
    test_config-scrollbar-animation-helper.h
    ${ANIMATION_HELPER_SOURCES}
    ${DEPENDENCY_SOURCES}
)

# 链接库和依赖项
target_link_libraries(test_config-scrollbar-animation-helper PRIVATE
    Qt5::Widgets
    Qt5::Test
    Qt5::Core
)

# 安装目标
install(TARGETS test_config-scrollbar-animation-helper DESTINATION bin)
