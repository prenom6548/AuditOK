cmake_minimum_required(VERSION 3.5)
#Set Project Name
project(test_udev LANGUAGES CXX C)
#message(${PROJECT_NAME})

option(TEST_DEBUG "option for debug" OFF)

if (TEST_DEBUG) 
	add_definitions(-DTEST_DEBUG)
    add_definitions(-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
    add_definitions(-DPROJECT_BINARY_DIR=\"${PROJECT_BINARY_DIR}\")
endif(TEST_DEBUG)


set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})


set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets X11Extras Network DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets X11Extras Network DBus REQUIRED)

set (SOURCE_PATH ../..)
#add include_directories
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/${SOURCE_PATH}/printer_manager/
)

file(GLOB printer_manager_SRC 
    "${SOURCE_PATH}/printer_manager/udev*"
    "${SOURCE_PATH}/printer_manager/deviceInformation.*"
    "main.cpp"
    )


add_executable(${PROJECT_NAME}
    ${printer_manager_SRC}
)

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::X11Extras Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)
target_link_libraries(${PROJECT_NAME} "-lpthread -lz")
target_link_libraries(${PROJECT_NAME} "-lcups")
target_link_libraries(${PROJECT_NAME} "-lusb-1.0")
target_link_libraries(${PROJECT_NAME} "-lbsd")
target_link_libraries(${PROJECT_NAME} "-ludev")
target_link_libraries(${PROJECT_NAME} "-lglib-2.0")