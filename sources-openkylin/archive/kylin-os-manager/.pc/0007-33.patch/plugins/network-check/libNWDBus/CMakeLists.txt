cmake_minimum_required(VERSION 3.5)

project(NWDBusLib LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")


find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets LinguistTools X11Extras Network DBus Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools X11Extras Network DBus Concurrent REQUIRED)
find_package(KF5WindowSystem REQUIRED)

#Gsetting
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)

#link_directories(/usr/lib/nw_check_tool_plugins/lib)
link_directories(${PROJECT_SOURCE_DIR}/../lib)

include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories(${PROJECT_SOURCE_DIR}/../libBase/include/)

file(GLOB source_SRC
    "src/*.cpp"
    "include/*.h"
    )
add_library(NWDBusLib SHARED
  ${source_SRC}
)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib)

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::X11Extras Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus Qt${QT_VERSION_MAJOR}::Concurrent)
target_link_libraries(${PROJECT_NAME} KF5::WindowSystem)

target_link_libraries(${PROJECT_NAME} PkgConfig::QGSETTINGS)
target_compile_definitions(NWDBusLib PRIVATE NWDBUSLIB_LIBRARY)
install(TARGETS NWDBusLib DESTINATION /usr/lib/)
