cmake_minimum_required(VERSION 3.5)

project(kysdk-genai-nlp-examples LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# jsoncpp
find_package(jsoncpp REQUIRED)

# openssl
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# package config
find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

pkg_check_modules(KYLIN_AI_CONFIG kyai-config)
include_directories(${KYLIN_AI_CONFIG_INCLUDE_DIRS})

# protocol
find_package(KylinAiProto REQUIRED)
kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES gennlpservice)

# inner
include_directories(${CMAKE_SOURCE_DIR}/include)

set(LIBRARIES
    pthread
    jsoncpp
    ${OPENSSL_LIBRARIES}
    ${GIO_LIBRARIES}
    ${KYLIN_AI_CONFIG_LIBRARIES}
    atomic
)

add_executable(chatbot chatbot.cpp ${SERVICE_SOURCES} ${COMMON_PROTO_FILES})

target_link_libraries(chatbot ${LIBRARIES})
