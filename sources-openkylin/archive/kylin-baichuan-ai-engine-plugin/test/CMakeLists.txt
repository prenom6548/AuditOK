include_directories(${CMAKE_SOURCE_DIR})
add_executable(testnlpengine
    testnlpengine.cpp
)
target_link_libraries(testnlpengine
    baichuan-engine-plugin
    curl
)
add_test(NAME testnlpengine COMMAND testnlpengine)


