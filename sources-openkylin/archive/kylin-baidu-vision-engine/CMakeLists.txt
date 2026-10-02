cmake_minimum_required(VERSION 3.5)

set(TARGET baidu-vision-engine)
project(baidu-vision-engine)

set(CMAKE_CXX_STANDARD 17)
include(GNUInstallDirs)

find_package(KylinAiEngine CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

add_subdirectory(third-party)
include_directories(third-party/cpr/include)

include_directories(src)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_compile_definitions(RUN_IN_DEBUG)
else ()
    add_compile_definitions(RUN_IN_RELEASE)
endif ()

add_library(${TARGET} SHARED
    src/baiduvisionengine_p.h
    src/baiduvisionengine_p.cpp
    src/baiduvisionengine.h
    src/baiduvisionengine.cpp
    src/baiduresponseprocessor.h
    src/baiduresponseprocessor.cpp
    src/imageloader.h
    src/imageloader.cpp
    src/servererror.h
    src/servererror.cpp
    src/token.h
    src/token.cpp
    src/util.h
    src/util.cpp
)

target_link_libraries(${TARGET}
    cpr
    jsoncpp
    freeimage
    ${OPENSSL_LIBRARIES}
)

install(TARGETS ${TARGET}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

set(HEADER_PATH kylin-ai/plugins/ai-engines)
install(FILES src/baiduvisionengine.h DESTINATION include/${HEADER_PATH})

set_target_properties(baidu-vision-engine PROPERTIES VERSION 1.0.0 SOVERSION 1)

if (ENABLE_TEST)
    find_package(OpenSSL REQUIRED)
    include_directories(${OPENSSL_INCLUDE_DIR})
    add_executable(test_baidu_vision_engine
        src/baiduvisionengine_p.h
        src/baiduvisionengine_p.cpp
        src/baiduvisionengine.h
        src/baiduvisionengine.cpp
        src/baiduresponseprocessor.h
        src/baiduresponseprocessor.cpp
        src/imageloader.h
        src/imageloader.cpp
        src/servererror.h
        src/servererror.cpp
        src/token.h
        src/token.cpp
        src/util.h
        src/util.cpp
        src/logger.h
        src/logger.cpp
        test/test_baidu_vision_engine.cpp
    )

    target_link_libraries(test_baidu_vision_engine
        cpr
        jsoncpp
        freeimage
        ${OPENSSL_LIBRARIES}
    )
endif ()