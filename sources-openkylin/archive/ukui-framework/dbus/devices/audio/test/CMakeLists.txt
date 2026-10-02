project(my_qt_app)

include_directories(../src)
find_package(PkgConfig)
cmake_minimum_required(VERSION 3.16)

# 查找Qt5及其模块
find_package(Qt5 COMPONENTS Core DBus)
# 接下来，你可以使用Qt${QT_VERSION_MAJOR}::Core, Qt${QT_VERSION_MAJOR}::DBus来链接你的可执行文件或库

set(CMAKE_AUTOMOC ON)

# 示例：添加可执行文件
add_executable(my_qt_app
    test.cpp
    ../src/audio.cpp)

target_include_directories(my_qt_app PRIVATE)

# 链接Qt模块
target_link_libraries(my_qt_app
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus)
