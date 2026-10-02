# Copyright (C) 2019-2024 Zilliz. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License

cmake_minimum_required(VERSION 3.16)
project(kylin-ai-vector-engine)

option(ENABLE_UNIT_TESTS "Enable unit tests" OFF)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(COMPILER_SUPPORTS_CXX17 True)
set(CMAKE_VERBOSE_MAKEFILE ON)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "-g  -Wall -Wl,-z,now ${CMAKE_CXX_FLAGS}")
else()
    set(CMAKE_CXX_FLAGS "-O3 -Wall -Wl,-z,now ${CMAKE_CXX_FLAGS}")
endif()
if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(gRPC REQUIRED)
include_directories(${gRPC_INCLUDE_DIRS})

find_package(TBB REQUIRED)
include_directories(${TBB_tbb_INCLUDE_DIRS})

find_package(nlohmann_json REQUIRED)
include_directories(${nlohmann_json_INCLUDE_DIRS})

find_package(gflags REQUIRED)
include_directories(${gflags_INCLUDE_DIRS})

find_package(double-conversion REQUIRED)
include_directories(${double-conversion_INCLUDE_DIRS})

find_package(yaml-cpp REQUIRED)
include_directories(${yaml-cpp_INCLUDE_DIRS})

find_package(absl REQUIRED)
include_directories(${absl_type_traits_INCLUDE_DIRS})

################################### proto ###################################
# 设置grpc生成文件的路径
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/pb)
set(GRPC_GENERATE_FILES_DIR "${CMAKE_BINARY_DIR}/pb")

# Protobuf KylinAiProto 要一起使用，并有顺序
find_package(Protobuf REQUIRED)
find_package(KylinAiProto REQUIRED)

kylin_ai_cmake_generate_grpc_proto_source(RETURNED_GRPC_SOURCE_FILES vector-db plan schema common segcore milvus msg feder rg)
message("grpc source files: ${RETURNED_GRPC_SOURCE_FILES}")
kylin_ai_cmake_generate_grpc_proto_service(RETURNED_GRPC_SERVICE_FILES vector-db milvus)
message("grpc service files: ${RETURNED_GRPC_SERVICE_FILES}")

add_library(milvus_proto STATIC ${RETURNED_GRPC_SOURCE_FILES})

add_library(milvus_grpc_service STATIC ${RETURNED_GRPC_SERVICE_FILES})
target_link_libraries(milvus_grpc_service milvus_proto gRPC::grpc++)
################################### proto ###################################

################################### parser ###################################
find_package(antlr4-runtime REQUIRED)
include_directories("${ANTLR4_INCLUDE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/src/parser/")
include_directories("${CMAKE_SOURCE_DIR}/src/parser/antlr/")
include_directories("${CMAKE_BINARY_DIR}") # 引用 milvus_proto 库的头文件 pb/plan.pb.h

add_library(parser STATIC "${CMAKE_SOURCE_DIR}/src/parser/parser.cc"
    "${CMAKE_SOURCE_DIR}/src/parser/antlr/PlanBaseVisitor.cpp"
    "${CMAKE_SOURCE_DIR}/src/parser/antlr/PlanLexer.cpp"
    "${CMAKE_SOURCE_DIR}/src/parser/antlr/PlanParser.cpp"
    "${CMAKE_SOURCE_DIR}/src/parser/antlr/PlanVisitor.cpp"
)

target_link_libraries(parser milvus_proto antlr4-runtime)
################################### parser ###################################

################################### folly ###################################

# 如果是 x86_64 架构，启用 SSE4.2
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    # knowhere需要引用符号F14IntrinsicsMode::SimdAndCrc但这个符号需要编译folly时开启see4.2才支持
    message(STATUS "Detected x86_64 architecture, enabling SSE4.2 support.")
    add_compile_options(-msse4.2)
else()
    message(STATUS "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}, skipping SSE4.2.")
endif()
set(FOLLY_NO_EXCEPTION_TRACER ON CACHE BOOL "No use exception tracer in folly")

# folly没有deb包，暂时采用直接自包含源码包的方式引用
include_directories("${CMAKE_SOURCE_DIR}/thirdparty/folly/")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/thirdparty/folly")
add_subdirectory(thirdparty/folly)

################################### folly ###################################

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set( KNOWHERE_VERSION v2.3.1 )
if ( BUILD_DISK_ANN STREQUAL "ON" )
    set(WITH_DISKANN ON CACHE BOOL "" FORCE )
else ()
    set(WITH_DISKANN OFF CACHE BOOL "" FORCE )
endif ()

if ( INDEX_ENGINE STREQUAL "cardinal" )
    set(WITH_CARDINAL ON CACHE BOOL "" FORCE )
else ()
    set(WITH_CARDINAL OFF CACHE BOOL "" FORCE )
endif()
if ( MILVUS_GPU_VERSION STREQUAL "ON" )
    set(WITH_RAFT ON CACHE BOOL "" FORCE )
endif ()
add_subdirectory(thirdparty/milvus/internal/core/thirdparty/knowhere)

include_directories("${CMAKE_SOURCE_DIR}/thirdparty/milvus/internal/core/thirdparty/")

include_directories("${knowhere_SOURCE_DIR}/include")
function(MILVUS_ADD_PKG_CONFIG MODULE)
    configure_file(${MODULE}.pc.in "${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.pc" @ONLY)
    # install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.pc"
    #       DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig/")
endfunction()

include_directories("${CMAKE_SOURCE_DIR}/thirdparty/milvus/internal/core/src/")

message(STATUS "Add simdjson with version: v3.1.7")
add_subdirectory(thirdparty/simdjson)
include_directories("${CMAKE_SOURCE_DIR}/thirdparty/simdjson/include/")

set(MILVUS_ENGINE_SRC
    "${CMAKE_SOURCE_DIR}/thirdparty/milvus/internal/core/src")
add_subdirectory(thirdparty/milvus/internal/core/src/log)
add_subdirectory(thirdparty/milvus/internal/core/src/config)
add_subdirectory(thirdparty/milvus/internal/core/src/common)
add_subdirectory(thirdparty/milvus/internal/core/src/query)
add_subdirectory(thirdparty/milvus/internal/core/src/exec)
add_subdirectory(thirdparty/milvus/internal/core/src/index)
add_subdirectory(thirdparty/milvus/internal/core/src/segcore)
add_subdirectory(thirdparty/milvus/internal/core/src/bitset)

if(ENABLE_UNIT_TESTS)
  include(CTest)
  enable_testing()
endif()

add_subdirectory(src)

find_program(MEMORYCHECK_COMMAND NAMES valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --track-origins=yes --leak-check=full --show-leak-kinds=all")

install(
    TARGETS kylin-ai-vector-engine
    RUNTIME DESTINATION /usr/bin
)
install(
    FILES conf/kylin-ai-vector-engine.service
    DESTINATION /usr/lib/systemd/user
)
install(
    FILES conf/vector_engine.json
    DESTINATION /usr/share/kylin-ai-vector-engine
)
