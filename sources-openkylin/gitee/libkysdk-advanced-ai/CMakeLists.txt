cmake_minimum_required(VERSION 3.5)

project(kysdk-advanced-ai LANGUAGES CXX C)

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
kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES advaiknowledgebaseservice)

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
include_directories(include/kylin-ai/advai)
include_directories(include/kylin-ai/advai/knowledgebase)
include_directories(src)
set(SERVICE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_error.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core.h


    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chathistory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chathistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatresponse.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatresponse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatresult.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatresult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatsource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatsource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatreference.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_chatreference.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_retrievalconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_retrievalconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_retrievalresponse.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_retrievalresponse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_kbsession.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_kbsession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_knowledgebaseinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_knowledgebaseinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kbsession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/knowledgebaseserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/knowledgebaseserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/knowledgebaseserviceproxy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/knowledgebaseserviceproxy.cpp
)

add_library(kysdk-advanced-ai SHARED
    include/kylin-ai/advai/knowledgebase/types.h
    include/kylin-ai/advai/knowledgebase/error.h
    ${SERVICE_SOURCES}
    ${COMMON_PROTO_FILES}
)

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(tests)
endif ()

set_target_properties(kysdk-advanced-ai PROPERTIES VERSION 1.0.0 SOVERSION 1)

target_link_libraries(
    kysdk-advanced-ai
    jsoncpp
    ${GIO_LIBRARIES}
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
install(TARGETS kysdk-advanced-ai
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/kylin-ai
    DESTINATION include)

install(DIRECTORY usr/ DESTINATION /usr)
