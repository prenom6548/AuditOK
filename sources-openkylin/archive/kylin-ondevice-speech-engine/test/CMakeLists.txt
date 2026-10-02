include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

find_package(OpenSSL REQUIRED)
pkg_check_modules(PULSEAUDIO REQUIRED libpulse)
pkg_check_modules(PULSEAUDIO_SIMPLE REQUIRED libpulse-simple)

# 设置调试模式宏定义，默认是Info权限Log输出:cmake -DENABLE_TEST=ON -DCMAKE_BUILD_TYPE=Debug ..
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DRUN_IN_DEBUG)
endif()

# 连续构造销毁测试
add_executable(testcontinuousconstruction
    testcontinuousconstruction.cpp
    logger.cpp
)

target_compile_options(testcontinuousconstruction PRIVATE -fsanitize=address -g -fno-omit-frame-pointer)
target_link_options(testcontinuousconstruction PRIVATE -fsanitize=address)

target_link_libraries(testcontinuousconstruction
    kylin-ondevice-speech-engine
    pthread
)

add_executable(testondevicespeechengine
    testondevicespeechengine.cpp
    audiorecorder.cpp
    audioplayer.cpp
    logger.cpp
    util.cpp
)

target_link_libraries(testondevicespeechengine
    kylin-ondevice-speech-engine
    pthread
    ${OPENSSL_LIBRARIES}
    ${PULSEAUDIO_LIBRARIES}
    ${PULSEAUDIO_SIMPLE_LIBRARIES}
)

# 非流式标点符号恢复功能测试
add_executable(testpunctuation
    testpunctuation.cpp
)
target_link_libraries(testpunctuation
    kylin-ondevice-speech-engine
    pthread
)

# 流式语音激活检测功能测试
add_executable(testvad
    testvad.cpp
)
target_link_libraries(testvad
    kylin-ondevice-speech-engine
    pthread
)

# 非流式语音识别功能测试
add_executable(testasrnonstreaming
    testasrnonstreaming.cpp
)
target_link_libraries(testasrnonstreaming
    kylin-ondevice-speech-engine
    pthread
)

# 流式语音识别功能测试
add_executable(testasrstreaming
    testasrstreaming.cpp
    audiorecorder.cpp
    logger.cpp
    util.cpp
)
target_link_libraries(testasrstreaming
    kylin-ondevice-speech-engine
    ${OPENSSL_LIBRARIES}
    ${PULSEAUDIO_LIBRARIES}
    ${PULSEAUDIO_SIMPLE_LIBRARIES}
    pthread
)

# 语音合成功能测试
add_executable(testmellotts
    testmellotts.cpp
    audioplayer.cpp
)
target_link_libraries(testmellotts
    kylin-ondevice-speech-engine
    ${PULSEAUDIO_LIBRARIES}
    ${PULSEAUDIO_SIMPLE_LIBRARIES}
    pthread
)
