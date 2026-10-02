cmake_minimum_required(VERSION 3.5)

set(TARGET sensetime-nlp-engine)
project(sensetime-nlp-engine)

set(CMAKE_CXX_STANDARD 17)
include(GNUInstallDirs)

find_package(KylinAiEngine CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)

add_subdirectory(third-party)
include_directories(third-party/cpr/include)

include_directories(.)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_compile_definitions(RUN_IN_DEBUG)
else ()
    add_compile_definitions(RUN_IN_RELEASE)
endif ()

add_library(${TARGET} SHARED
    sensetimenlpengine_p.h
    sensetimenlpengine_p.cpp
    sensetimenlpengine.h
    sensetimenlpengine.cpp
    servererror.h
    servererror.cpp
    token.h
    token.cpp
    util.h
    util.cpp
)

target_link_libraries(${TARGET}
    cpr
    jsoncpp
    jwt
)

install(TARGETS ${TARGET}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

set(HEADER_PATH kylin-ai/plugins/ai-engines)
install(FILES sensetimenlpengine.h DESTINATION include/${HEADER_PATH})

set_target_properties(sensetime-nlp-engine PROPERTIES VERSION 1.0.0 SOVERSION 1)

if (ENABLE_TEST)
    find_package(OpenSSL REQUIRED)
    include_directories(${OPENSSL_INCLUDE_DIR})
    add_executable(test_sensetime_nlp_engine
        sensetimenlpengine_p.h
        sensetimenlpengine_p.cpp
        sensetimenlpengine.h
        sensetimenlpengine.cpp
        servererror.h
        servererror.cpp
        util.h
        util.cpp
        token.h
        token.cpp
        test_sensetime_nlp_engine.cpp
    )
    target_link_libraries(test_sensetime_nlp_engine
        cpr
        jsoncpp
        ${OPENSSL_LIBRARIES}
        jwt
    )
endif ()