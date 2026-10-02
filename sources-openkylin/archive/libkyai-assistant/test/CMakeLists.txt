cmake_minimum_required(VERSION 3.5)

project(kyai-assistant LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#######################################################################
# Options
#######################################################################

include_directories(include)
include_directories(src)

find_package(PkgConfig REQUIRED)
find_package(KylinAiProto REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
pkg_check_modules(GIO REQUIRED gio-unix-2.0)

find_package(jsoncpp REQUIRED)

kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES assistantservice)
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS})

pkg_check_modules(KYAICONFIG REQUIRED kyai-config)
include_directories(${KYAICONFIG_INCLUDE_DIRS})
message(${KYAICONFIG_INCLUDE_DIRS}22222)

add_executable(test_assistant
    test_assistant.cpp
    ../include/kylin-ai/private/osassistant/osassistant.h
    ../include/kylin-ai/private/osassistant/error.h
    ../promptidmanager.h
    ../promptidmanager.cpp
    ../error.cpp
    ../osassistant.cpp
    ../assistantserviceproxy.cpp
    ../serverproxy.cpp
    ${COMMON_PROTO_FILES}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(kyai-assistant stdc++fs)
    endif ()
endif ()

target_link_libraries(
    test_assistant
    ${GIO_LIBRARIES}
    jsoncpp
    ${JSON_LIBRARIES}
    pthread
    ${KYAIASSIST_LIBRARIES}
    ${KYAICONFIG_LIBRARIES}
)

file(COPY 1.png DESTINATION /tmp)