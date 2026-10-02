cmake_minimum_required(VERSION 3.5)

project(kysdk-coreai-speech LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#######################################################################
# Options
#######################################################################
option(ENABLE_TEST "Build Test" OFF)

include_directories(include)
include_directories(src)

find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(KylinAiProto REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(AI_COMMON REQUIRED kysdk-ai-common)

find_package(jsoncpp REQUIRED)

kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES corespeechservice)
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${GIO_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${AI_COMMON_INCLUDE_DIRS})
include_directories(include/kylin-ai/coreai/speech)
include_directories(include/kylin-ai)

set(SERVICE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_speechrecognitionresult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_speechrecognitionsession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_speechsynthesisresult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_speechsynthesizersession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_speechmodelconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/_speechmodelconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audioconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audiodatastream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/coreaispeechserviceproxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/coreaispeechserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/recognizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/synthesizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocolversion.cpp
)

add_library(kysdk-coreai-speech SHARED
    include/kylin-ai/coreai/speech/audioconfig.h
    include/kylin-ai/coreai/speech/audiodatastream.h
    include/kylin-ai/coreai/speech/recognizer.h
    include/kylin-ai/coreai/speech/result.h
    include/kylin-ai/coreai/speech/synthesizer.h
    include/kylin-ai/coreai/speech/error.h
    include/kylin-ai/coreai/speech/config.h
    ${SERVICE_SOURCES}
    ${COMMON_PROTO_FILES}
)

if (ENABLE_TEST)
    add_subdirectory(tests)
endif ()

set_target_properties(kysdk-coreai-speech PROPERTIES VERSION 1.0.0 SOVERSION 1)

target_link_libraries(
    kysdk-coreai-speech
    jsoncpp
    ${GIO_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
install(TARGETS kysdk-coreai-speech
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/kylin-ai
    DESTINATION include)

install(DIRECTORY usr/ DESTINATION /usr)
