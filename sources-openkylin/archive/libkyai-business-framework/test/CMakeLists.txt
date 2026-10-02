cmake_minimum_required(VERSION 3.5)

project(test LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${PROJECT_SOURCE_DIR}/../include
)

find_package(GTest)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-unix-2.0)

file(GLOB_RECURSE TESTS_SOURCE_FLIES ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.c)
add_executable(test ${TESTS_SOURCE_FLIES})
target_link_libraries(test
        PRIVATE kyai-business-framework
        PRIVATE PkgConfig::GIO
        GTest::GTest GTest::Main ${GMOCK_LIBRARIES}
        nlohmann_json::nlohmann_json)
