cmake_minimum_required(VERSION 3.16)

project(kysdk-coreai-embedding LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_compile_definitions(RUN_IN_DEBUG)
else ()
    add_compile_definitions(RUN_IN_RELEASE)
endif ()

find_package(nlohmann_json REQUIRED)
find_package(KylinAiProto REQUIRED)
find_package(KysdkAiCommon REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(AI_COMMON REQUIRED kysdk-ai-common)

kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES coretextembeddingservice coreimageembeddingservice)

file(GLOB SERVICE_SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/textEmbedding/*.cpp
    ${PROJECT_SOURCE_DIR}/src/imageEmbedding/*.cpp
    ${PROJECT_SOURCE_DIR}/src/server/*.cpp
    ${PROJECT_SOURCE_DIR}/log/logger.cpp)

include_directories(
    ${PROJECT_SOURCE_DIR}/include/kylin-ai/coreai/embedding
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/textEmbedding
    ${PROJECT_SOURCE_DIR}/src/imageEmbedding
    ${PROJECT_SOURCE_DIR}/src/server
    ${PROJECT_SOURCE_DIR}/log
    ${GIO_INCLUDE_DIRS}
    ${AI_COMMON_INCLUDE_DIRS}
)

add_library(kysdk-coreai-embedding SHARED
    ${SERVICE_SOURCES}
    ${COMMON_PROTO_FILES}
)

set_target_properties(kysdk-coreai-embedding PROPERTIES VERSION 1.0.0 SOVERSION 1)

target_link_libraries(kysdk-coreai-embedding
        PUBLIC ${GIO_LIBRARIES}
        PRIVATE nlohmann_json::nlohmann_json
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
install(TARGETS kysdk-coreai-embedding
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/kylin-ai
    DESTINATION include)

install(DIRECTORY usr/ DESTINATION /usr)

if (DEFINED ENABLE_TEST)
    unset(ENABLE_TEST CACHE)
endif(DEFINED ENABLE_TEST)
option(ENABLE_TEST "Build Test" OFF)
if (ENABLE_TEST)
    add_subdirectory(test)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif (ENABLE_TEST)