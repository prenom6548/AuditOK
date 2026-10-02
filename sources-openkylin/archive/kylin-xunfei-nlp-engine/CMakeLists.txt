cmake_minimum_required(VERSION 3.5)

set(TARGET xunfei-nlp-engine)
project(xunfei-nlp-engine)

set(CMAKE_CXX_STANDARD 17)
include(GNUInstallDirs)

find_package(KylinAiEngine CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

include_directories(src)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_compile_definitions(RUN_IN_DEBUG)
else ()
    add_compile_definitions(RUN_IN_RELEASE)
endif ()

add_library(${TARGET} SHARED
    src/xunfeinlpengine_p.h
    src/xunfeinlpengine_p.cpp
    src/xunfeinlpengine.h
    src/xunfeinlpengine.cpp
    src/servererror.h
    src/servererror.cpp
    src/token.h
    src/token.cpp
    src/util.h
    src/util.cpp
    src/logger.h
    src/logger.cpp
)

target_link_libraries(${TARGET}
    curl
    jsoncpp
)

install(TARGETS ${TARGET}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

set(HEADER_PATH kylin-ai/plugins/ai-engines)
install(FILES src/xunfeinlpengine.h DESTINATION include/${HEADER_PATH})

set_target_properties(xunfei-nlp-engine PROPERTIES VERSION 1.0.0 SOVERSION 1)

if (ENABLE_TEST)
    find_package(OpenSSL REQUIRED)
    include_directories(${OPENSSL_INCLUDE_DIR})
    add_executable(test_xunfei_nlp_engine
        test/test_xunfei_nlp_engine.cpp
        src/xunfeinlpengine_p.h
        src/xunfeinlpengine_p.cpp
        src/xunfeinlpengine.h
        src/xunfeinlpengine.cpp
        src/servererror.h
        src/servererror.cpp
        src/token.h
        src/token.cpp
        src/util.h
        src/util.cpp
        src/logger.h
        src/logger.cpp
    )
    target_link_libraries(test_xunfei_nlp_engine
        curl
        jsoncpp
        ${OPENSSL_LIBRARIES}
    )
endif ()
