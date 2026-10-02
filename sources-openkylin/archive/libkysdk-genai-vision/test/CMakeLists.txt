cmake_minimum_required(VERSION 3.5)

project(kylin-genai-vision-test LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(KylinAiProto REQUIRED)
kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES genaivisionservice)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

pkg_check_modules(KYAICONFIG kyai-config)
include_directories(${KYAICONFIG_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(testGenAiVisionApi testGenVision.cpp ${SERVICE_SOURCES} ${COMMON_PROTO_FILES})

target_link_libraries(testGenAiVisionApi
    ${GIO_LIBRARIES}
    pthread
    jsoncpp
    ${OPENSSL_LIBRARIES}
    ${KYAICONFIG_LIBRARIES}
)
