cmake_minimum_required(VERSION 3.5)

project(kylin-ai-config-test LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
find_package(KylinAiProto REQUIRED)

kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES configservice)

include_directories(../include/kylin-ai/private/aiconfig)

include_directories(${GIO_INCLUDE_DIRS})

add_executable(testModelConfig testModelConfig.cpp
    ${SDK_SOURCES}
)

target_link_libraries(testModelConfig
    ${GIO_LIBRARIES}
    pthread
    jsoncpp
)

include(CMakePackageConfigHelpers)
