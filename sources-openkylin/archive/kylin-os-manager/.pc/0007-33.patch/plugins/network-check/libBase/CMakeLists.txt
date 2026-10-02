cmake_minimum_required(VERSION 3.5)

project(libBase LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools X11Extras Network DBus Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools X11Extras Network DBus Concurrent REQUIRED)
find_package(KF5WindowSystem REQUIRED)

#Gsetting
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)

include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories(${PROJECT_SOURCE_DIR}/../Observer_Notifier/)

file(GLOB source_SRC
    "src/*.cpp"
    "include/*.h"
    "../Observer_Notifier/*.cpp"
    "../Observer_Notifier/*.h"
    )
add_library(libBase SHARED
  ${source_SRC}
)

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::X11Extras Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus Qt${QT_VERSION_MAJOR}::Concurrent)
target_link_libraries(${PROJECT_NAME} KF5::WindowSystem)

target_link_libraries(${PROJECT_NAME} PkgConfig::QGSETTINGS)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib)

target_compile_definitions(libBase PRIVATE LIBBASE_LIBRARY)
install(TARGETS libBase DESTINATION ${PROJECT_NAME}/../lib/)
