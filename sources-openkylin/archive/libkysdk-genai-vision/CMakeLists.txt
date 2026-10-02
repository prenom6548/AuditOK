cmake_minimum_required(VERSION 3.5)

project(kysdk-genai-vision LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#######################################################################
# Options
#######################################################################
option(ENABLE_TEST "Build Test" OFF)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_compile_definitions(RUN_IN_DEBUG)
else ()
    add_compile_definitions(RUN_IN_RELEASE)
endif ()

find_package(jsoncpp REQUIRED)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(KylinAiProto REQUIRED)
kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES genaivisionservice)

find_package(KysdkAiCommon REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

include_directories(include)
include_directories(src)
include_directories(include/kylin-ai/genai/vision)
include_directories(include/kylin-ai)

set(SERVICE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imageconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_imageresult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_genaiimagesession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/genaivisionserviceproxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocolversion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp
)

add_library(kysdk-genai-vision SHARED ${SERVICE_SOURCES} ${COMMON_PROTO_FILES})

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

set_target_properties(kysdk-genai-vision PROPERTIES VERSION 1.0.0 SOVERSION 1)

target_link_libraries(
    kysdk-genai-vision
    jsoncpp
    ${GIO_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
install(TARGETS kysdk-genai-vision
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/kylin-ai
    DESTINATION include)

install(DIRECTORY usr/ DESTINATION /usr)
