cmake_minimum_required(VERSION 3.5)

project(server LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# kyai-business-framework 动态库里已包含头文件，不用再指定 include_directories
#include_directories(${PROJECT_SOURCE_DIR}/../../include)
#message(${PROJECT_SOURCE_DIR}/../../include)

file(GLOB SOURCE_LIST ${PROJECT_SOURCE_DIR}/*.cpp)
#message(${SOURCE_LIST})

#include_directories(${PROJECT_SOURCE_DIR}/../gdbus)

add_executable(server ${SOURCE_LIST})

#target_link_directories(server PUBLIC ${PROJECT_SOURCE_DIR}/../../lib)
target_link_libraries(server PUBLIC kyai-business-framework)
