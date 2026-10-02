cmake_minimum_required(VERSION 3.5)

project(speechinput)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME} SHARED)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g -rdynamic)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

find_package(Qt5 COMPONENTS Multimedia REQUIRED)
target_link_libraries(speechinput PRIVATE  Qt5::Multimedia)

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/audioinfo.h
    ${CMAKE_CURRENT_LIST_DIR}/audioinfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/speechinput.h
    ${CMAKE_CURRENT_LIST_DIR}/speechinput.cpp
    )

target_sources(${PROJECT_NAME} PRIVATE ${SRCS})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/)
