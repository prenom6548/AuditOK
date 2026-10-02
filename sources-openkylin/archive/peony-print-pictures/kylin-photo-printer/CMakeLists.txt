cmake_minimum_required(VERSION 3.5)

project(kylin-photo-printer)

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
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets DBus PrintSupport REQUIRED)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


include_directories(${SOURCE_PATH})

file(GLOB SRC
    "${SOURCE_PATH}/*.h"
    "${SOURCE_PATH}/*.cpp"
)



add_executable(${PROJECT_NAME}
    ${SRC}
)


target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::DBus Qt${QT_VERSION_MAJOR}::PrintSupport)

target_link_libraries(${PROJECT_NAME} "-lpthread -lz")

include_directories(${SOURCE_PATH}/../3rdparty/kylinimagecodec)
target_link_libraries(${PROJECT_NAME} kylinimagecodec)

find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)
target_link_libraries(${PROJECT_NAME} PkgConfig::QGSETTINGS)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
install(FILES ${PROJECT_SOURCE_DIR}/data/org.kylin.photo.printer.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)