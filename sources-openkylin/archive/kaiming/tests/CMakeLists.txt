#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
# 
# See the LICENSE file for more details.
#

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(kaiming-unit-test
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 查找GTest PkgConfig Threads
# find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
# find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# 使用 REQUIRED, 如果找不到模块，命令将失败并报错，并停止运行
pkg_check_modules(glib2 REQUIRED glib-2.0)
pkg_check_modules(gio2 REQUIRED gio-2.0)
pkg_check_modules(giounix2 REQUIRED gio-unix-2.0)
pkg_check_modules(ostree1 REQUIRED ostree-1)
pkg_check_modules(dbus1 REQUIRED dbus-1)
pkg_check_modules(dbusglib1 REQUIRED dbus-glib-1)
pkg_check_modules(curl REQUIRED libcurl)
pkg_check_modules(uuid REQUIRED uuid)
pkg_check_modules(elf REQUIRED libelf)
pkg_check_modules(GTest REQUIRED gtest)
pkg_check_modules(GTestMain REQUIRED gtest_main)

pkg_search_module(lz4 REQUIRED IMPORTED_TARGET liblz4)
pkg_search_module(lzma REQUIRED IMPORTED_TARGET liblzma)
pkg_search_module(zlib REQUIRED IMPORTED_TARGET zlib)
pkg_search_module(zstd REQUIRED IMPORTED_TARGET libzstd)
pkg_search_module(fuse REQUIRED IMPORTED_TARGET fuse)
pkg_search_module(lzo2 REQUIRED IMPORTED_TARGET lzo2)

set(ONLY_TO_TERMINAL 1)
configure_file(config.h.in config.h)

aux_source_directory(../common TESTING_SRC_COMMON)
aux_source_directory(../app/deltas TESTING_SRC_APP_DELTAS)
aux_source_directory(../app/builder TESTING_SRC_APP_BUILDER)
aux_source_directory(../app/install TESTING_SRC_APP_INSTALL)
aux_source_directory(../app/update TESTING_SRC_APP_UPDATE)
aux_source_directory(../app/upgrade TESTING_SRC_APP_UPGRADE)
aux_source_directory(../app/list TESTING_SRC_APP_LIST)
aux_source_directory(../app/info TESTING_SRC_APP_INFO)
aux_source_directory(../app/search TESTING_SRC_APP_SEARCH)
aux_source_directory(../app/repair TESTING_SRC_APP_REPAIR)
aux_source_directory(../app/uninstall TESTING_SRC_APP_UNINSTALL)
aux_source_directory(../app/enter TESTING_SRC_APP_ENTER)
aux_source_directory(../app/run TESTING_SRC_APP_RUN)
aux_source_directory(../app/ps TESTING_SRC_APP_PS)
aux_source_directory(../app/print-meta TESTING_SRC_APP_PRINTMETA)
aux_source_directory(../app/kill TESTING_SRC_APP_KILL)
aux_source_directory(../app/which TESTING_SRC_APP_WHICH)
aux_source_directory(../app/remoteModify TESTING_SRC_APP_REMOTE)
aux_source_directory(../app/changelog TESTING_SRC_APP_CHANGELOG)
aux_source_directory(../app/compare-versions TESTING_SRC_APP_COMPAREVERSIONS)
aux_source_directory(../ok/engine/squashfuse TESTING_SRC_OK_ENGINE_SQUASHFUSE)
aux_source_directory(app CASE_SRC_APP)
aux_source_directory(common CASE_SRC_COMMON)
aux_source_directory(ok CASE_SRC_OK)

add_executable(kaiming-unit-test
    ${TESTING_SRC_COMMON}
    ${TESTING_SRC_APP_BUILDER}
    ${TESTING_SRC_APP_CHANGELOG}
    ${TESTING_SRC_APP_COMPAREVERSIONS}
    ${TESTING_SRC_APP_LIST}
    ${TESTING_SRC_APP_PRINTMETA}
    ${TESTING_SRC_OK_ENGINE_SQUASHFUSE}
    ${TESTING_SRC_APP_DELTAS}
    ${TESTING_SRC_APP_INSTALL}
    ${TESTING_SRC_APP_UPDATE}
    ${TESTING_SRC_APP_UPGRADE}
    ${TESTING_SRC_APP_INFO}
    ${TESTING_SRC_APP_SEARCH}
    ${TESTING_SRC_APP_REPAIR}
    ${TESTING_SRC_APP_UNINSTALL}
    ${TESTING_SRC_APP_ENTER}
    ${TESTING_SRC_APP_RUN}
    ${TESTING_SRC_APP_KILL}
    ${TESTING_SRC_APP_PS}
    ${TESTING_SRC_APP_WHICH}
    ${TESTING_SRC_APP_REMOTE}
    ../ok/engine/KMOABApplication.cpp
    ../ok/engine/KMOABUtils.cpp
    ../ok/engine/KMOABElf.cpp
    ../ok/engine/KMOABExtract.cpp
    ../ok/engine/KMOABMount.cpp
    ../ok/engine/KMOABPrintDataDir.cpp
    ../ok/engine/KMOABPrintMeta.cpp
    ../ok/engine/KMOABRepackage.cpp
    ${CASE_SRC_APP}
    ${CASE_SRC_COMMON}
    ${CASE_SRC_OK}
)

target_compile_options(kaiming-unit-test PRIVATE -D_FILE_OFFSET_BITS=64)

option(ENABLE_COVERAGE "Enable coverage flags" OFF)
if(ENABLE_COVERAGE)
    target_compile_options(kaiming-unit-test PRIVATE -O0 -g -fprofile-arcs -ftest-coverage -Wno-coverage-mismatch)
    target_link_options(kaiming-unit-test PRIVATE --coverage -lgcov)
    message("-- Enable coverage flags")
endif()

target_include_directories(kaiming-unit-test PRIVATE
    ${glib2_INCLUDE_DIRS}
    ${gio2_INCLUDE_DIRS}
    ${giounix2_INCLUDE_DIRS}
    ${ostree1_INCLUDE_DIRS}
    ${dbusglib1_INCLUDE_DIRS}
    ${dbus1_INCLUDE_DIRS}
    ${curl_INCLUDE_DIRS}
    ${uuid_INCLUDE_DIRS}
    ${elf_INCLUDE_DIRS}
    ${lz4_INCLUDE_DIRS}
    ${lzma_INCLUDE_DIRS}
    ${zlib_INCLUDE_DIRS}
    ${zstd_INCLUDE_DIRS}
    ${fuse_INCLUDE_DIRS}
    ${lzo2_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
    # ${GTEST_INCLUDE_DIRS}
    ${GTest_INCLUDE_DIRS}
    ${GTestMain_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

target_link_libraries(kaiming-unit-test PRIVATE
    ${glib2_LIBRARIES}
    ${gio2_LIBRARIES}
    ${giounix2_LIBRARIES}
    ${ostree1_LIBRARIES}
    ${dbusglib1_LIBRARIES}
    ${dbus1_LIBRARIES}
    ${curl_LIBRARIES}
    ${uuid_LIBRARIES}
    ${elf_LIBRARIES}
    ${lz4_LIBRARIES}
    ${lzma_LIBRARIES}
    ${zlib_LIBRARIES}
    ${zstd_LIBRARIES}
    ${fuse_LIBRARIES}
    ${lzo2_LIBRARIES}
    Threads::Threads
    # GTest::gtest
    # GTest::gtest_main
    ${GTest_LIBRARIES}
    ${GTestMain_LIBRARIES}
    -ldl
)

# add_test(NAME KMUnitTest COMMAND kaiming-unit-test)

# 覆盖率目标
if(ENABLE_COVERAGE)
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage-report
        COMMAND lcov --zerocounters --ignore-errors unused,empty --directory . --build-directory .
        COMMAND sh -c "yes | ./kaiming-unit-test" || true
        COMMAND lcov --capture --directory . --build-directory . --ignore-errors mismatch,mismatch --ignore-errors  gcov,gcov --rc geninfo_unexecuted_blocks=1 --output-file coverage.info
        COMMAND lcov --remove coverage.info /usr/include/* ok/engine/squashfuse/* */tests/* *.h *.hpp --ignore-errors unused,unused --ignore-errors empty,empty --output-file filtered.info
        COMMAND genhtml filtered.info --output-directory coverage-report
        COMMAND python3 ${CMAKE_BINARY_DIR}/../result.py --lcov filtered.info --output ${CMAKE_BINARY_DIR}/../../kaiming-coverage.xml
        COMMAND sh -c "if [ $(id -u) -eq 0 ]; then _u=$SUDO_USER; [ \"$_u\" != \"root\" ] && sudo -u \"$_u\" xdg-open coverage-report/index.html; else xdg-open coverage-report/index.html; fi" || true
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report in coverage-report/"
        VERBATIM
    )
endif()

