cmake_minimum_required(VERSION 3.5)

project(client LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCE_LIST ${PROJECT_SOURCE_DIR}/*.cpp)

find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-unix-2.0)

add_executable(client ${SOURCE_LIST})

target_include_directories(client PRIVATE ${GIO_INCLUDE_DIRS})
target_link_libraries(client PUBLIC kyai-business-framework PRIVATE PkgConfig::GIO)
