cmake_minimum_required(VERSION 3.5)

project(kysdk-coreai-vision LANGUAGES CXX C)

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

# jsoncpp
find_package(jsoncpp REQUIRED)

# kylin-ai-proto
find_package(KylinAiProto REQUIRED)
kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES corevisionservice)

# pkg-config
find_package(PkgConfig REQUIRED)

# openssl
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# kysdk-ai-common
find_package(KysdkAiCommon REQUIRED)

# glib
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

include_directories(include/kylin-ai/coreai/vision)
include_directories(include/kylin-ai)
include_directories(include)
include_directories(src)

set(SERVICE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_textrecognitionresult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_textrecognitionsession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/textrecognition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/corevisionserviceproxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_textrecognitionconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_textrecognitionconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_subjectsegmentationconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_subjectsegmentationconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/subjectsegmentation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_subjectsegmentationsession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_subjectsegmentationresult.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/segmentation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_segmentationresult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_segmentationsession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocolversion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocolversion.cpp

)

add_library(kysdk-coreai-vision SHARED
    include/kylin-ai/coreai/vision/textrecognitionresult.h
    include/kylin-ai/coreai/vision/textrecognition.h
    include/kylin-ai/coreai/vision/error.h
    include/kylin-ai/coreai/vision/config.h
    include/kylin-ai/coreai/vision/subjectsegmentationresult.h
    include/kylin-ai/coreai/vision/subjectsegmentation.h
    ${SERVICE_SOURCES}
    ${COMMON_PROTO_FILES}
)

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(tests)
endif ()

set_target_properties(kysdk-coreai-vision PROPERTIES VERSION 1.0.0 SOVERSION 1)

target_link_libraries(
    kysdk-coreai-vision
    jsoncpp
    ${GIO_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
install(TARGETS kysdk-coreai-vision
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/kylin-ai
    DESTINATION include)

install(DIRECTORY usr/ DESTINATION /usr)
