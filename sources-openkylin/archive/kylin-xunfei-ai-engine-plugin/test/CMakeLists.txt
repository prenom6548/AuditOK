configure_file(16k_10.pcm ${CMAKE_CURRENT_BINARY_DIR}/16k_10.pcm COPYONLY)
find_package(OpenSSL REQUIRED)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})
add_executable(testspeechengine
    testspeechengine.cpp
)
target_link_libraries(testspeechengine
    xunfei-engine-plugin
    ${OPENSSL_LIBRARIES}
)
add_test(NAME testspeechengine COMMAND testspeechengine)

include_directories(${CMAKE_SOURCE_DIR})
add_executable(testvisionengine
    testvisionengine.cpp
)
target_link_libraries(testvisionengine
    xunfei-engine-plugin
    freeimage
    ${OPENSSL_LIBRARIES}
)
add_test(NAME testvisionengine COMMAND testvisionengine)

include_directories(${CMAKE_SOURCE_DIR})
add_executable(testnlpengine
    testnlpengine.cpp
)
target_link_libraries(testnlpengine
    xunfei-engine-plugin
)
add_test(NAME testnlpengine COMMAND testnlpengine)