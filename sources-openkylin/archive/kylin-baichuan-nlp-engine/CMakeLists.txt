cmake_minimum_required(VERSION 3.5)

set(TARGET baichuan-nlp-engine)
project(baichuan-nlp-engine)

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
    baichuannlpengine_p.h
    baichuannlpengine_p.cpp
    baichuannlpengine.h
    baichuannlpengine.cpp
    servererror.h
    servererror.cpp
    util.h
    util.cpp
)

target_link_libraries(${TARGET}
    cpr
    jsoncpp
)

install(TARGETS ${TARGET}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

set(HEADER_PATH kylin-ai/plugins/ai-engines)
install(FILES baichuannlpengine.h DESTINATION include/${HEADER_PATH})

set_target_properties(baichuan-nlp-engine PROPERTIES VERSION 1.0.0 SOVERSION 1)

if (ENABLE_TEST)
    add_executable(test_baichuan_nlp_engine
        baichuannlpengine_p.h
        baichuannlpengine_p.cpp
        baichuannlpengine.h
        baichuannlpengine.cpp
        servererror.h
        servererror.cpp
        util.h
        util.cpp
        test_baichuan_nlp_engine.cpp
    )
    target_link_libraries(test_baichuan_nlp_engine
        cpr
        curl
        jsoncpp
        ${OPENSSL_LIBRARIES}
    )
endif ()