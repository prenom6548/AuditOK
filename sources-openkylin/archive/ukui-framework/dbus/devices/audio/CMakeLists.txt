cmake_minimum_required(VERSION 3.5)
project(audio VERSION 1.0)

find_package(PkgConfig)

cmake_minimum_required(VERSION 3.16)
# 查找Qt5及其模块
find_package(QT NAMES Qt6 Qt5 COMPONENTSCONFIG)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus  REQUIRED)
SET(CMAKE_AUTOMOC ON)

file(GLOB LIBAUDIO_SOURCES src/*.cpp)

# 编译动态库
add_library(audio STATIC ${LIBAUDIO_SOURCES})

target_link_libraries(audio
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    ${DEVICE_AUDIO_LINK})
