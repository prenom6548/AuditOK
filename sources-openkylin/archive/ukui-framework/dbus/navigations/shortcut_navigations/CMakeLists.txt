cmake_minimum_required(VERSION 3.5)
project(shortcutnavigations VERSION 1.0)

find_package(PkgConfig)

# 索引动态库的头文件地址
include_directories(${GOBJECT_INCLUDE_DIRS})
#add_subdirectory(test)

cmake_minimum_required(VERSION 3.16)
# 查找Qt5及其模块
find_package(QT NAMES Qt6 Qt5 COMPONENTSCONFIG)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus  REQUIRED)

set(NAVIGATION_LINK
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${GOBJECT_LIBRARIES}
)

set(CMAKE_AUTOMOC ON)

file(GLOB LIBNATIVATION_SOURCES src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${LIBNATIVATION_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    ${NAVIGATION_LINK})

