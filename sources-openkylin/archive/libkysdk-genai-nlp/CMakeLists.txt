cmake_minimum_required(VERSION 3.5)

project(kysdk-genai-nlp LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#######################################################################
# Options
#######################################################################
option(BUILD_EXAMPLES "Build Examples" ON)
option(ENABLE_TEST "Build Test" OFF)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_compile_definitions(RUN_IN_DEBUG)
else ()
    add_compile_definitions(RUN_IN_RELEASE)
endif ()

# jsoncpp
find_package(jsoncpp REQUIRED)

# kylin-ai-proto
find_package(KylinAiProto REQUIRED)
kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES gennlpservice)

# pkg-config
find_package(PkgConfig REQUIRED)

# glib
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

# kysdk-ai-common
pkg_check_modules(AI_COMMON REQUIRED kysdk-ai-common)
include_directories(${AI_COMMON_INCLUDE_DIRS})

include_directories(include)
include_directories(include/kylin-ai)
include_directories(include/kylin-ai/genai/text)
include_directories(src)
set(SERVICE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatmessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatmessage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatmodelconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatmodelconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatresult.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatresult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_genaitextsession.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_genaitextsession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_textmodelinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_textmodelinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocolversion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocolversion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_toolchoice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_toolchoice.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/genainlpserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/genainlpserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/genainlpserviceproxy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/genainlpserviceproxy.cpp
)

add_library(kysdk-genai-nlp SHARED
    include/kylin-ai/genai/text/error.h
    ${SERVICE_SOURCES}
    ${COMMON_PROTO_FILES}
)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(tests)
endif ()

set_target_properties(kysdk-genai-nlp PROPERTIES VERSION 1.0.0 SOVERSION 1)

target_link_libraries(
    kysdk-genai-nlp
    jsoncpp
    ${GIO_LIBRARIES}
    atomic
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
install(TARGETS kysdk-genai-nlp
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/kylin-ai
    DESTINATION include)

install(DIRECTORY usr/ DESTINATION /usr)
