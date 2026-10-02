cmake_minimum_required(VERSION 3.16)

project(test LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/../include
)

find_package(GTest)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GMOCK REQUIRED IMPORTED_TARGET gmock)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-unix-2.0)

find_package(nlohmann_json REQUIRED)
find_package(KylinAiProto REQUIRED)
find_package(Protobuf REQUIRED)

file(GLOB_RECURSE TESTS_SOURCE_FLIES ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(test ${TESTS_SOURCE_FLIES})
target_link_libraries(test
        PRIVATE kysdk-coreai-embedding
        GTest::GTest GTest::Main ${GMOCK_LIBRARIES}
        PkgConfig::GIO
        pthread
        )
# target_compile_definitions(test
#         PRIVATE COREAI_EMBEDDING_ENABLE_MOCK)