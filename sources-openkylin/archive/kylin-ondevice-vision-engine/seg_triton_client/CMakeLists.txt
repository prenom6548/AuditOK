set(TARGET seg_triton_client)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(${TARGET}
    client.cpp
    ../utils/base64_transformer.cpp
)

option(LLAMA_SERVER_VERBOSE "Build verbose logging option for Server" ON)
set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${TARGET} PRIVATE
    SERVER_VERBOSE=$<BOOL:${LLAMA_SERVER_VERBOSE}>
)
target_link_libraries(${TARGET} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(${TARGET} PRIVATE cxx_std_11)
