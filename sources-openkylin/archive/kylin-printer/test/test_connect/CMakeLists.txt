cmake_minimum_required(VERSION 3.5)
# Set Project Name
project(test_connect LANGUAGES C CXX)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets)



add_executable(${PROJECT_NAME}
    common.h
    common.cpp
    custom_qstring_converter.h
    custom_qstring_converter.cpp
    main.cpp
)

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(${PROJECT_NAME} "-lpthread -lz")
