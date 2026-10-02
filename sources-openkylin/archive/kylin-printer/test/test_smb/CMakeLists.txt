cmake_minimum_required(VERSION 3.5)
# Set Project Name
project(test_smb LANGUAGES C CXX)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools X11Extras Network DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools X11Extras Network DBus REQUIRED)

#add include_directories
include_directories(
    ${PROJECT_SOURCE_DIR}
    
)

set(${PROJECT_NAME}_SRC
    main.cpp
)

set(${PROJECT_NAME}_HDR
    
)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SRC}
    ${${PROJECT_NAME}_HDR}
)
target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::X11Extras Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)

target_link_libraries(${PROJECT_NAME} "-lsmbclient")
target_link_libraries(${PROJECT_NAME} "-lpopt")
target_link_libraries(${PROJECT_NAME} "-lcups")