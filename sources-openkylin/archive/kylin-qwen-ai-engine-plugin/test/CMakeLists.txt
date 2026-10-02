include_directories(${CMAKE_SOURCE_DIR})
add_executable(testnlpengine
    testnlpengine.cpp
)
target_link_libraries(testnlpengine
    qwen-engine-plugin
    curl
)
add_test(NAME testnlpengine COMMAND testnlpengine)

