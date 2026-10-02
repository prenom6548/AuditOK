cmake_minimum_required(VERSION 3.10)
project(kylin-ondevice-vision-engine)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

FIND_PACKAGE(OpenCV REQUIRED)
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

# 包含头文件
include_directories(
    ${OpenCV_INCLUDE_DIRS} 
    ${TRITON_CLIENT_PATH}
    utils
)

add_subdirectory(ocr_triton_client)
add_subdirectory(matting_triton_client)
add_subdirectory(image_binaryzation)
add_subdirectory(seg_triton_client)

add_library(${PROJECT_NAME} SHARED
    ondeviceocrengine.cpp
    ondevicesegengine.cpp
    ondeviceportraitmattingengine.cpp
    ondevicevisioncapabilityengine.cpp
    ondeviceimagebinaryzationengine.cpp
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ocr_triton_client
    matting_triton_client
    image_binaryzation
    seg_triton_client
    httpclient
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})

install(FILES ondevicevisioncapabilityengine.h
    DESTINATION 
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/kylin-ai/plugins/ai-engines)

if (ENABLE_TEST)

    FIND_PACKAGE(OpenCV REQUIRED)
    INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")

    # 包含头文件
    include_directories(
        ${OpenCV_INCLUDE_DIRS}
        ${TRITON_CLIENT_PATH}
        utils
    )

    add_executable(test-vision-engine
        ondeviceocrengine.cpp
        ondeviceddcpengine.cpp
        ondeviceportraitmattingengine.cpp
        ondeviceimagebinaryzationengine.cpp
        test/test_vision_engine.cpp
    )

    # 链接库
    target_link_libraries(test-vision-engine
        ${OpenCV_LIBS}
        ${PROJECT_NAME}
        gcov
    )
    target_compile_options(test-vision-engine PRIVATE --coverage -fprofile-arcs -ftest-coverage)
endif ()
