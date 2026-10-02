cmake_minimum_required(VERSION 3.5)

set(TARGET xunfei-speech-engine)
project(xunfei-speech-engine)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_compile_definitions(RUN_IN_DEBUG)
else ()
    add_compile_definitions(RUN_IN_RELEASE)
endif ()

include(GNUInstallDirs)

find_package(KylinAiEngine CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KysdkAiCommon REQUIRED)

include_directories(src)

include_directories(third-party/libhv-v1.3.3)
include_directories(third-party/libhv-v1.3.3/base)
include_directories(third-party/libhv-v1.3.3/cpputil)
include_directories(third-party/libhv-v1.3.3/event)
include_directories(third-party/libhv-v1.3.3/evpp)
include_directories(third-party/libhv-v1.3.3/http)
include_directories(third-party/libhv-v1.3.3/http/client)
include_directories(third-party/libhv-v1.3.3/ssl)
include_directories(third-party/libhv-v1.3.3/util)

find_package(OpenSSL REQUIRED)

add_definitions(-DWITH_OPENSSL)

add_library(${TARGET} SHARED
    third-party/libhv-v1.3.3/base/hbase.c
    third-party/libhv-v1.3.3/base/hlog.c
    third-party/libhv-v1.3.3/base/hsocket.c
    third-party/libhv-v1.3.3/base/htime.c

    third-party/libhv-v1.3.3/http/wsdef.c
    third-party/libhv-v1.3.3/http/websocket_parser.c

    third-party/libhv-v1.3.3/ssl/openssl.c
    third-party/libhv-v1.3.3/ssl/hssl.c

    third-party/libhv-v1.3.3/util/base64.c
    third-party/libhv-v1.3.3/util/sha1.c

    third-party/libhv-v1.3.3/event/hloop.c
    third-party/libhv-v1.3.3/event/hevent.c
    third-party/libhv-v1.3.3/event/epoll.c
    third-party/libhv-v1.3.3/event/nio.c
    third-party/libhv-v1.3.3/event/unpack.c

    third-party/libhv-v1.3.3/cpputil/hstring.cpp
    third-party/libhv-v1.3.3/cpputil/hurl.cpp
    third-party/libhv-v1.3.3/cpputil/ThreadLocalStorage.cpp

    third-party/libhv-v1.3.3/http/http_parser.c
    third-party/libhv-v1.3.3/http/httpdef.c
    third-party/libhv-v1.3.3/http/multipart_parser.c

    third-party/libhv-v1.3.3/http/http_content.cpp
    third-party/libhv-v1.3.3/http/client/WebSocketClient.cpp
    third-party/libhv-v1.3.3/http/WebSocketChannel.cpp
    third-party/libhv-v1.3.3/http/WebSocketParser.cpp
    third-party/libhv-v1.3.3/http/HttpMessage.cpp
    third-party/libhv-v1.3.3/http/HttpParser.cpp
    third-party/libhv-v1.3.3/http/Http1Parser.cpp

    src/xunfeispeechengine_p.h
    src/xunfeispeechengine_p.cpp
    src/xunfeispeechengine.h
    src/xunfeispeechengine.cpp
    src/servererror.h
    src/servererror.cpp
    src/util.h
    src/util.cpp
    src/logger.h
    src/logger.cpp
)

target_link_libraries(${TARGET}
    jsoncpp
)

install(TARGETS ${TARGET}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

set(HEADER_PATH kylin-ai/plugins/ai-engines)
install(FILES src/xunfeispeechengine.h DESTINATION include/${HEADER_PATH})

set_target_properties(xunfei-speech-engine PROPERTIES VERSION 1.0.0 SOVERSION 1)

if (ENABLE_TEST)
    find_package(OpenSSL REQUIRED)
    include_directories(${OPENSSL_INCLUDE_DIR})
    add_executable(test_xunfei_speech_engine
        third-party/libhv-v1.3.3/base/hbase.c
        third-party/libhv-v1.3.3/base/hlog.c
        third-party/libhv-v1.3.3/base/hsocket.c
        third-party/libhv-v1.3.3/base/htime.c

        third-party/libhv-v1.3.3/http/wsdef.c
        third-party/libhv-v1.3.3/http/websocket_parser.c

        third-party/libhv-v1.3.3/ssl/openssl.c
        third-party/libhv-v1.3.3/ssl/hssl.c

        third-party/libhv-v1.3.3/util/base64.c
        third-party/libhv-v1.3.3/util/sha1.c

        third-party/libhv-v1.3.3/event/hloop.c
        third-party/libhv-v1.3.3/event/hevent.c
        third-party/libhv-v1.3.3/event/epoll.c
        third-party/libhv-v1.3.3/event/nio.c
        third-party/libhv-v1.3.3/event/unpack.c

        third-party/libhv-v1.3.3/cpputil/hstring.cpp
        third-party/libhv-v1.3.3/cpputil/hurl.cpp
        third-party/libhv-v1.3.3/cpputil/ThreadLocalStorage.cpp

        third-party/libhv-v1.3.3/http/http_parser.c
        third-party/libhv-v1.3.3/http/httpdef.c
        third-party/libhv-v1.3.3/http/multipart_parser.c

        third-party/libhv-v1.3.3/http/http_content.cpp
        third-party/libhv-v1.3.3/http/client/WebSocketClient.cpp
        third-party/libhv-v1.3.3/http/WebSocketChannel.cpp
        third-party/libhv-v1.3.3/http/WebSocketParser.cpp
        third-party/libhv-v1.3.3/http/HttpMessage.cpp
        third-party/libhv-v1.3.3/http/HttpParser.cpp
        third-party/libhv-v1.3.3/http/Http1Parser.cpp
        src/xunfeispeechengine_p.h
        src/xunfeispeechengine_p.cpp
        src/xunfeispeechengine.h
        src/xunfeispeechengine.cpp
        src/servererror.h
        src/servererror.cpp
        src/util.h
        src/util.cpp
        src/logger.h
        src/logger.cpp
        test/audioplayer.h
        test/audioplayer.cpp
        test/audiorecorder.h
        test/audiorecorder.cpp
        test/test_xunfei_speech_engine.cpp
    )

    target_link_libraries(test_xunfei_speech_engine
        pthread
        jsoncpp
        ${OPENSSL_LIBRARIES}
        pulse
        pulse-simple
    )
endif ()

