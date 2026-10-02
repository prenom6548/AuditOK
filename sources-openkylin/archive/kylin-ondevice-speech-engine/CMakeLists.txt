cmake_minimum_required(VERSION 3.16)

set(TARGET kylin-ondevice-speech-engine)
project(kylin-ondevice-speech-engine)

set(CMAKE_CXX_STANDARD 17)

find_package(TritonClient REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KylinAiEngine CONFIG REQUIRED)
find_package(KysdkAiCommon REQUIRED)

pkg_check_modules(gRPC++ REQUIRED grpc++)
pkg_check_modules(GoogleProtobuf REQUIRED protobuf)
pkg_check_modules(OpenSSL REQUIRED openssl)
pkg_check_modules(JsonCpp REQUIRED jsoncpp)
pkg_check_modules(UUID REQUIRED uuid)

if(TritonClient_NOT_FOUND)
    message(FATAL_ERROR "TritonClient library not found")
else()
    # 下面两个变量都为空，Triton 的 cmake 文件有问题
    message(STATUS ${TritonClient_LIBRARIES})
    message(STATUS ${TritonClient_INCLUDE_DIRS})
endif()

# 检查 TritonClient::grpcclient 目标是否存在
if(NOT TARGET TritonClient::grpcclient)
    message(FATAL_ERROR "TritonClient::grpcclient target not found")
endif()

# 提供 CMAKE_INSTALL_LIBDIR 和 CMAKE_INSTALL_INCLUDEDIR 变量
include(GNUInstallDirs)

add_library(${TARGET} SHARED
    src/ondevicespeechengine.cpp
    src/ondevicespeechengine_p.cpp
    src/punctuation.cpp
    src/voiceactivitydetection.cpp
    src/asrstreaming.cpp
    src/asrnonstreaming.cpp
    src/mellotts.cpp
    src/tritonmodelclient.cpp
    src/utils.cpp
)

target_include_directories(${TARGET} PRIVATE
    ${JsonCpp_INCLUDE_DIRS}
    ${gRPC++_INCLUDE_DIRS}
    ${GoogleProtobuf_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIRS}
)
target_link_directories(${TARGET} PRIVATE
    ${JsonCpp_LIBRARY_DIRS}
    ${OpenSSL_LIBRARY_DIRS}
    ${gRPC++_LIBRARY_DIRS}
    ${GoogleProtobuf_LIBRARY_DIRS}
)
target_link_libraries(${TARGET}
    httpclient
    TritonClient::grpcclient
    ${JsonCpp_LIBRARIES}
    ${UUID_LIBRARIES}
    ${gRPC++_LIBRARIES}
    ${OpenSSL_LIBRARIES}
    ${GoogleProtobuf_LIBRARIES}
)

install(TARGETS ${TARGET}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})

install(FILES src/ondevicespeechengine.h
        DESTINATION
        ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/kylin-ai/plugins/ai-engines)


option(ENABLE_TEST "Build Test" OFF)
if(ENABLE_TEST)
    add_subdirectory(test)
endif()
