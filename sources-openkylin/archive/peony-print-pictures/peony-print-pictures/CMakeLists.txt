cmake_minimum_required(VERSION 3.5)

project(peony-print-pictures)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

set (SOURCE_PATH .)

add_definitions(-DTEST_DEBUG)
add_definitions(-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_definitions(-DPROJECT_BINARY_DIR=\"${PROJECT_BINARY_DIR}\")

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")
add_definitions(-DPROJECT_VERSION=\"${PROJECT_VERSION}\")



set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


include_directories(${SOURCE_PATH}
                    /usr/include/peony-qt)

file(GLOB SRC
    "${SOURCE_PATH}/*.h"
    "${SOURCE_PATH}/*.cpp"
)

SET(ui_RESOURCE ${PROJECT_SOURCE_DIR}/peony-print-pictures.qrc)

add_library(${PROJECT_NAME} SHARED
    ${SRC}
    ${ui_RESOURCE}
)

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(${PROJECT_NAME} "-lpthread -lz")

find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)
target_link_libraries(${PROJECT_NAME} PkgConfig::QGSETTINGS)

pkg_check_modules(PEONY_PKG REQUIRED peony)
target_link_libraries(${PROJECT_NAME} ${PEONY_PKG_LIBRARIES})

pkg_search_module(GLIB REQUIRED gio-2.0)
include_directories(${GLIB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} "-lgio-2.0")

execute_process(
    COMMAND qmake -query QT_INSTALL_LIBS
    OUTPUT_VARIABLE QT_INSTALL_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Qt Install Prefix: ${QT_INSTALL_LIBS}")

install(TARGETS ${PROJECT_NAME} DESTINATION ${QT_INSTALL_LIBS}/peony-extensions)


