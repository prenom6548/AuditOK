cmake_minimum_required(VERSION 3.5)
# Set Project Name
project(test_snmp LANGUAGES C CXX)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

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
    ${PROJECT_SOURCE_DIR}/${SOURCE_PATH}/common/
)

file(GLOB ${PROJECT_NAME}_SRC
    "${SOURCE_PATH}/printer_manager/snmp*"
    "${SOURCE_PATH}/common/*"
    "main.cpp"
    "test.*"
)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SRC}
)

find_library(NETSNMPAGENT "netsnmpagent")
find_library(NETSNMPMIBS "netsnmpmibs")
find_library(NETSNMP "netsnmp")

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::X11Extras Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)

target_link_libraries(${PROJECT_NAME} ${NETSNMPAGENT} ${NETSNMPMIBS} ${NETSNMP})