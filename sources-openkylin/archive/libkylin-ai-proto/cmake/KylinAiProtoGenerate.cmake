# protoc -I=$SRC_DIR --grpc_out=$DST_DIR --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $SRC_DIR/xxx.proto 生成服务端代码
# protoc -I=$SRC_DIR --cpp_out=$DST_DIR $SRC_DIR/xxx.proto

# 调用解析文件函数前需要引用protobuf，find_package(Protobuf REQUIRED)
message(STATUS "using protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")

# 调用解析文件函数前需要设置存放生成文件的路径，未设置使用默认值 CMAKE_CURRENT_BINARY_DIR
if(DEFINED GRPC_GENERATE_FILES_DIR)
        message("grpc output dir: ${GRPC_GENERATE_FILES_DIR}")
else()
        set(GRPC_GENERATE_FILES_DIR ${CMAKE_CURRENT_BINARY_DIR})
        message("grpc output dir not set, use default dir: ${GRPC_GENERATE_FILES_DIR}")
endif()

# 示例：kylin_ai_generate_grpc_proto_source(RETURNED_GRPC_SOURCE_FILES vector-db common feder msg rg schema milvus)
function(kylin_ai_generate_grpc_proto_source RETURNED_GRPC_SOURCE_FILES PATH)
    set(PROTO_FILE_NAMES)
    set(PROTO_FILES)
    set(GENERATE_PROTO_FILES)
    set(SRC_DIR /usr/share/kylin-ai/protocols/grpc/${PATH})

    # ARGN 是函数传入以外的参数
    foreach(ARG ${ARGN})
        list(APPEND PROTO_FILE_NAMES ${ARG})
    endforeach()

    foreach(PROTO_FILE_NAME ${PROTO_FILE_NAMES})
        set(PROTO_FILE ${SRC_DIR}/${PROTO_FILE_NAME}.proto)
        if(NOT EXISTS ${PROTO_FILE})
            message(FATAL_ERROR "grpc-proto-gen: Unable to find proto file: ${PROTO_FILE}")
        else()
            list(APPEND PROTO_FILES ${PROTO_FILE})
        endif()
    endforeach()

    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_FILE_NAME ${PROTO_FILE} NAME)
        string(REPLACE ".proto" "" PROTO_FILE_NAME ${PROTO_FILE_NAME})

        add_custom_command(
                OUTPUT ${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.pb.cc
                       ${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.pb.h
                DEPENDS ${PROTO_FILE}
                        ${PROTOBUF_PROTOC_EXECUTABLE}
                COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
                ARGS --cpp_out ${GRPC_GENERATE_FILES_DIR}
                     -I ${SRC_DIR}
                     ${PROTO_FILE}
        )

        list(APPEND GENERATE_PROTO_FILES "${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.pb.cc")
        list(APPEND GENERATE_PROTO_FILES "${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.pb.h")
    endforeach()

    include_directories(${GRPC_GENERATE_FILES_DIR})
    set(${RETURNED_GRPC_SOURCE_FILES} ${GENERATE_PROTO_FILES} PARENT_SCOPE)
endfunction(kylin_ai_generate_grpc_proto_source RETURNED_GRPC_SOURCE_FILES)

function(kylin_ai_generate_grpc_proto_service RETURNED_GRPC_SERVICE_FILES PATH)
    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
    message(STATUS "using protoc: ${GRPC_CPP_PLUGIN}")

    set(PROTO_FILE_NAMES)
    set(PROTO_FILES)
    set(GENERATE_PROTO_FILES)
    set(SRC_DIR /usr/share/kylin-ai/protocols/grpc/${PATH})

    # ARGN 是函数传入以外的参数
    foreach(ARG ${ARGN})
        list(APPEND PROTO_FILE_NAMES ${ARG})
    endforeach()

    foreach(PROTO_FILE_NAME ${PROTO_FILE_NAMES})
        set(PROTO_FILE ${SRC_DIR}/${PROTO_FILE_NAME}.proto)
        if(NOT EXISTS ${PROTO_FILE})
            message(FATAL_ERROR "grpc-proto-gen: Unable to find proto file: ${PROTO_FILE}")
        else()
            list(APPEND PROTO_FILES ${PROTO_FILE})
        endif()
    endforeach()

    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_FILE_NAME ${PROTO_FILE} NAME)
        string(REPLACE ".proto" "" PROTO_FILE_NAME ${PROTO_FILE_NAME})

        add_custom_command(
                OUTPUT ${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.grpc.pb.cc
                       ${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.grpc.pb.h
                DEPENDS ${PROTO_FILE}
                        ${PROTOBUF_PROTOC_EXECUTABLE}
                        ${GRPC_CPP_PLUGIN}
                COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
                ARGS --grpc_out ${GRPC_GENERATE_FILES_DIR}
                     -I ${SRC_DIR}
                     --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
                     ${PROTO_FILE}
        )

        list(APPEND GENERATE_PROTO_FILES "${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.grpc.pb.cc")
        list(APPEND GENERATE_PROTO_FILES "${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.grpc.pb.h")
    endforeach()

    include_directories(${GRPC_GENERATE_FILES_DIR})
    set(${RETURNED_GRPC_SERVICE_FILES} ${GENERATE_PROTO_FILES} PARENT_SCOPE)
endfunction(kylin_ai_generate_grpc_proto_service RETURNED_GRPC_SERVICE_FILES)

# cmake 阶段生成文件
function(kylin_ai_cmake_generate_grpc_proto_source RETURNED_GRPC_SOURCE_FILES PATH)
    set(PROTO_FILE_NAMES)
    set(PROTO_FILES)
    set(GENERATE_PROTO_FILES)
    set(SRC_DIR /usr/share/kylin-ai/protocols/grpc/${PATH})

    # ARGN 是函数传入以外的参数
    foreach(ARG ${ARGN})
        list(APPEND PROTO_FILE_NAMES ${ARG})
    endforeach()

    foreach(PROTO_FILE_NAME ${PROTO_FILE_NAMES})
        set(PROTO_FILE ${SRC_DIR}/${PROTO_FILE_NAME}.proto)
        if(NOT EXISTS ${PROTO_FILE})
            message(FATAL_ERROR "grpc-proto-gen: Unable to find proto file: ${PROTO_FILE}")
        else()
            list(APPEND PROTO_FILES ${PROTO_FILE})
        endif()
    endforeach()

    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_FILE_NAME ${PROTO_FILE} NAME)
        string(REPLACE ".proto" "" PROTO_FILE_NAME ${PROTO_FILE_NAME})

        execute_process(
                COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
                    --cpp_out ${GRPC_GENERATE_FILES_DIR}
                    -I ${SRC_DIR}
                    ${PROTO_FILE}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE PROTOC_RESULT
                OUTPUT_VARIABLE PROTOC_OUTPUT
                ERROR_VARIABLE PROTOC_ERROR
        )

        if (NOT PROTOC_RESULT EQUAL 0)
            message(FATAL_ERROR "protoc failed for ${PROTO_FILE}: ${PROTOC_RESULT}, ${PROTOC_OUTPUT}, ${PROTOC_ERROR}")
        endif ()

        list(APPEND GENERATE_PROTO_FILES "${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.pb.cc")
        list(APPEND GENERATE_PROTO_FILES "${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.pb.h")
    endforeach()

    include_directories(${GRPC_GENERATE_FILES_DIR})
    set(${RETURNED_GRPC_SOURCE_FILES} ${GENERATE_PROTO_FILES} PARENT_SCOPE)
endfunction(kylin_ai_cmake_generate_grpc_proto_source RETURNED_GRPC_SOURCE_FILES)

function(kylin_ai_cmake_generate_grpc_proto_service RETURNED_GRPC_SERVICE_FILES PATH)
    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
    message(STATUS "using grpc_cpp_plugin: ${GRPC_CPP_PLUGIN}")

    set(PROTO_FILE_NAMES)
    set(PROTO_FILES)
    set(GENERATE_PROTO_FILES)
    set(SRC_DIR /usr/share/kylin-ai/protocols/grpc/${PATH})

    # ARGN 是函数传入以外的参数
    foreach(ARG ${ARGN})
        list(APPEND PROTO_FILE_NAMES ${ARG})
    endforeach()

    foreach(PROTO_FILE_NAME ${PROTO_FILE_NAMES})
        set(PROTO_FILE ${SRC_DIR}/${PROTO_FILE_NAME}.proto)
        if(NOT EXISTS ${PROTO_FILE})
            message(FATAL_ERROR "grpc-proto-gen: Unable to find proto file: ${PROTO_FILE}")
        else()
            list(APPEND PROTO_FILES ${PROTO_FILE})
        endif()
    endforeach()

    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_FILE_NAME ${PROTO_FILE} NAME)
        string(REPLACE ".proto" "" PROTO_FILE_NAME ${PROTO_FILE_NAME})

        execute_process(
                COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
                    --grpc_out ${GRPC_GENERATE_FILES_DIR}
                    -I ${SRC_DIR}
                    --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
                    ${PROTO_FILE}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE PROTOC_RESULT
                OUTPUT_VARIABLE PROTOC_OUTPUT
                ERROR_VARIABLE PROTOC_ERROR
        )

        if (NOT PROTOC_RESULT EQUAL 0)
            message(FATAL_ERROR "protoc failed for ${PROTO_FILE}: ${PROTOC_RESULT}, ${PROTOC_OUTPUT}, ${PROTOC_ERROR}")
        endif ()

        list(APPEND GENERATE_PROTO_FILES "${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.grpc.pb.cc")
        list(APPEND GENERATE_PROTO_FILES "${GRPC_GENERATE_FILES_DIR}/${PROTO_FILE_NAME}.grpc.pb.h")
    endforeach()

    include_directories(${GRPC_GENERATE_FILES_DIR})
    set(${RETURNED_GRPC_SERVICE_FILES} ${GENERATE_PROTO_FILES} PARENT_SCOPE)
endfunction(kylin_ai_cmake_generate_grpc_proto_service RETURNED_GRPC_SERVICE_FILES)
