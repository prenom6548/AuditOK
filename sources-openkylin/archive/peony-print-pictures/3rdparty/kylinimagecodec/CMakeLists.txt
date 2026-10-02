cmake_minimum_required(VERSION 3.5)

project(kylinimagecodec)
project(${PROJECT_NAME} VERSION 1.0.0)


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
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Svg REQUIRED)


include_directories(${SOURCE_PATH}/image_conver
                    ${SOURCE_PATH}/image_load
                    ${SOURCE_PATH}/image_save
                    ${SOURCE_PATH}
                    )


file(GLOB SRC
    "${SOURCE_PATH}/*.h"
    "${SOURCE_PATH}/*.cpp"
    "${SOURCE_PATH}/image_conver/*.h"
    "${SOURCE_PATH}/image_conver/*.cpp"
    "${SOURCE_PATH}/image_load/*.h"
    "${SOURCE_PATH}/image_load/*.cpp"
    "${SOURCE_PATH}/image_save/*.h"
    "${SOURCE_PATH}/image_save/*.cpp"
)


add_library(${PROJECT_NAME} STATIC
    ${SRC}
)


target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg)

target_link_libraries(${PROJECT_NAME} "-lpthread -lz")

target_link_libraries(${PROJECT_NAME} "-lstb")
target_link_libraries(${PROJECT_NAME} "-lgif")
target_link_libraries(${PROJECT_NAME} "-lfreeimage")
target_link_libraries(${PROJECT_NAME} "-lfreeimageplus")


find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# include_directories(/usr/include/opencv4/)
# target_link_libraries(${PROJECT_NAME} "-lopencv_core")
# target_link_libraries(${PROJECT_NAME} "-lopencv_imgcodecs")
# target_link_libraries(${PROJECT_NAME} "-lopencv_imgproc")
