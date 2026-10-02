#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
# 
# See the LICENSE file for more details.
#

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(UnitTests)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")

set(ONLY_TO_TERMINAL 1)
configure_file(config.h.in config.h)

# 查找GTest
find_package(GTest REQUIRED)

# 源文件目录
aux_source_directory(app TEST_SRC_APP)
aux_source_directory(common TEST_SRC_COMMON)
aux_source_directory(ok TEST_SRC_OK)
aux_source_directory(../../app/builder OUTER_SRC_BUILDER)

# 添加你的源文件
add_executable(kmtest
        main.cpp
        ${TEST_SRC_APP}
        ${TEST_SRC_COMMON}
        ${TEST_SRC_OK}
        ${OUTER_SRC_BUILDER}
        ../../ok/engine/KMOABElf.cpp
        ../../ok/engine/KMOABExtract.cpp
        ../../ok/engine/KMOABPrintMeta.cpp
        ../../ok/engine/KMOABUtils.cpp
        ../../ok/engine/KMOABRepackage.cpp
        ../../ok/engine/KMOABPrintDataDir.cpp
        ../../ok/engine/KMOABMount.cpp
        ../../ok/engine/KMOABRun.cpp
        ../../ok/engine/squashfuse/cache.cpp
        ../../ok/engine/squashfuse/decompress.cpp
        ../../ok/engine/squashfuse/dir.cpp
        ../../ok/engine/squashfuse/file.cpp
        ../../ok/engine/squashfuse/fs.cpp
        ../../ok/engine/squashfuse/fuseprivate.cpp
        ../../ok/engine/squashfuse/hl.cpp
        ../../ok/engine/squashfuse/nonstd-enoattr.cpp
        ../../ok/engine/squashfuse/nonstd-makedev.cpp
        ../../ok/engine/squashfuse/nonstd-pread.cpp
        ../../ok/engine/squashfuse/nonstd-stat.cpp
        ../../ok/engine/squashfuse/stack.cpp
        ../../ok/engine/squashfuse/swap.cpp
        ../../ok/engine/squashfuse/table.cpp
        ../../ok/engine/squashfuse/traverse.cpp
        ../../ok/engine/squashfuse/util.cpp
        ../../ok/engine/squashfuse/xattr.cpp
        )

# 包含GTest头文件
target_include_directories(kmtest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/run
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/install
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/search
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/uninstall
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/update
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/kill
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/info
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/list
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/repair
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/ps
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/builder
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ok/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ok/engine/squashfuse
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/enter
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/which
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/remoteModify
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/upgrade
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/deltas
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ${GTEST_INCLUDE_DIRS}
)

# 链接GTest库到你的可执行文件
target_link_libraries(kmtest PUBLIC  
    kaiming-common
    ${GTEST_LIBRARIES}
    pthread
    lzma
    zstd
    lz4
    fuse
    lzo2
)
