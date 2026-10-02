cmake_minimum_required(VERSION 3.5)

set(TARGET baidu-nlp-engine)
project(baidu-nlp-engine)

set(CMAKE_CXX_STANDARD 17)
include(GNUInstallDirs)

find_package(KylinAiEngine CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KysdkAiCommon REQUIRED)

add_subdirectory(third-party)
include_directories(third-party/cpr/include)

include_directories(src)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_compile_definitions(RUN_IN_DEBUG)
else ()
    add_compile_definitions(RUN_IN_RELEASE)
endif ()

add_library(${TARGET} SHARED
    src/baidunlpengine_p.h
    src/baidunlpengine_p.cpp
    src/baidunlpengine.h
    src/baidunlpengine.cpp
    src/servererror.h
    src/servererror.cpp
    src/token.h
    src/token.cpp
    src/logger.h
    src/logger.cpp
    src/util.h
    src/util.cpp
)

target_link_libraries(${TARGET}
    cpr
    jsoncpp
)

install(TARGETS ${TARGET}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

set(HEADER_PATH kylin-ai/plugins/ai-engines)
install(FILES src/baidunlpengine.h DESTINATION include/${HEADER_PATH})

set_target_properties(baidu-nlp-engine PROPERTIES VERSION 1.0.0 SOVERSION 1)

if (ENABLE_TEST)
    add_executable(test-baidu-nlp-engine
        src/baidunlpengine_p.h
        src/baidunlpengine_p.cpp
        src/baidunlpengine.h
        src/baidunlpengine.cpp
        src/servererror.h
        src/servererror.cpp
        src/util.h
        src/util.cpp
        src/token.h
        src/token.cpp
        src/logger.h
        src/logger.cpp
        test/test_baidu_nlp_engine.cpp
    )
    target_link_libraries(test-baidu-nlp-engine
        cpr
        curl
        jsoncpp
    )
endif ()