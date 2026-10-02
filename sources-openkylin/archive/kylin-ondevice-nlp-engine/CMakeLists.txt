cmake_minimum_required(VERSION 3.16)

set(TARGET kylin-ondevice-nlp-engine)
project(kylin-ondevice-nlp-engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(TritonClient REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(fmt REQUIRED)
find_package(KylinAiEngine CONFIG REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(gRPC++ REQUIRED IMPORTED_TARGET grpc++)

include(GNUInstallDirs)
include_directories(${TRITON_CLIENT_PATH})
include_directories(${GIO_INCLUDE_DIRS})
#######################################################################
# Options
#######################################################################
option(ENABLE_TEST "Build Test" OFF)

add_library(${TARGET} SHARED
    ondevicenlpengine.h
    ondevicenlpengine.cpp
    client/llmtritonclient.h
    client/llmtritonclient.cpp
    utils/logger.h
    utils/logger.cpp
    utils/utils.h
    utils/utils.cpp
)

target_include_directories(${TARGET} PRIVATE ${TRITONCLIENT_INCLUDE_DIRS})
set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${TARGET}
    fmt::fmt
    ${GLIB_LIBRARIES}
    httpclient
    grpcclient
    ${TRITONCLIENT_LIB_DIRS}
)

install(TARGETS ${TARGET}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})

install(FILES ondevicenlpengine.h
        DESTINATION
        ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/kylin-ai/plugins/ai-engines)

if (ENABLE_TEST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")

find_package(Protobuf REQUIRED)
    add_executable(test-nlp-engine test/test_nlp_engine.cpp)
    target_link_libraries(test-nlp-engine
     ${Protobuf_LIBRARIES}
        kylin-ondevice-nlp-engine
        protobuf
    )
    target_compile_options(test-nlp-engine PRIVATE --coverage -fprofile-arcs -ftest-coverage)
endif ()
