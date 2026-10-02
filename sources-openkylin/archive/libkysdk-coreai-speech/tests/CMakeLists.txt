cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_compile_definitions(RUN_IN_DEBUG)
else ()
    add_compile_definitions(RUN_IN_RELEASE)
endif ()

find_package(KylinAiProto REQUIRED)
kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES corespeechservice)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(KYSDK_AI_CONFIG kyai-config)
include_directories(${KYSDK_AI_CONFIG_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_SOURCE_DIR}/include/kylin-ai)
include_directories(${CMAKE_SOURCE_DIR}/tests)

set(SOURCES
    ${COMMON_PROTO_FILES}
    ${SERVICE_SOURCES}
)

set(CATCH2_SOURCES
    ${CMAKE_SOURCE_DIR}/3rdparty/catch2/catch_amalgamated.cpp)

set(LIBRARIES
    ${GIO_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    pthread
    jsoncpp
    pulse
    pulse-simple
    ${KYSDK_AI_CONFIG_LIBRARIES}
)

add_executable(testCoreSpeechApi testCoreSpeech.cpp ${SOURCES})
add_executable(TestAsrToolFromMic TestAsrToolFromMic.cpp ${SOURCES})
add_executable(TestAsrToolOnce TestAsrToolOnce.cpp AudioRecorder.cpp ${SOURCES})
add_executable(TestTtsToolOnce TestTtsToolOnce.cpp AudioPlayer.cpp ${SOURCES})
add_executable(TestAudioConfig TestAudioConfig.cpp ${SOURCES} ${CATCH2_SOURCES})

pkg_check_modules(GTEST REQUIRED gtest)
add_executable(TestSynthesizerWithRequestId TestSynthesizerWithRequestId.cpp ${SOURCES})
target_include_directories(TestSynthesizerWithRequestId PRIVATE ${GTEST_INCLUDE_DIRS})

add_executable(TestRecognizerWithRequestId TestRecognizerWithRequestId.cpp ${SOURCES})
target_include_directories(TestRecognizerWithRequestId PRIVATE ${GTEST_INCLUDE_DIRS})

target_link_libraries(testCoreSpeechApi ${LIBRARIES})
target_link_libraries(TestAsrToolFromMic ${LIBRARIES})
target_link_libraries(TestAsrToolOnce ${LIBRARIES})
target_link_libraries(TestTtsToolOnce ${LIBRARIES})
target_link_libraries(TestAudioConfig ${LIBRARIES})
target_link_libraries(TestSynthesizerWithRequestId ${LIBRARIES} ${GTEST_LIBRARIES})
target_link_libraries(TestRecognizerWithRequestId ${LIBRARIES} ${GTEST_LIBRARIES})
