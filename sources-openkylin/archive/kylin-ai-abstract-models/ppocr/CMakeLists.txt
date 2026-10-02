# Copyright 2021, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.14)

project(tutorialbackend LANGUAGES C CXX)

#
# Options
#
# Must include options required for this project as well as any
# projects included in this one by FetchContent.
#
# GPU support is disabled by default because ocr backend doesn't
# use GPUs.
#
option(TRITON_ENABLE_GPU "Enable GPU support in backend" OFF)
option(TRITON_ENABLE_STATS "Include statistics collections in backend" ON)

FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(TritonCore REQUIRED)
FIND_PACKAGE(TritonCommon REQUIRED)
FIND_PACKAGE(TritonBackend REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)
FIND_PACKAGE(RapidJSON REQUIRED)
FIND_PACKAGE(nlohmann_json REQUIRED)
pkg_check_modules(ONNXRUNTIME REQUIRED IMPORTED_TARGET libonnxruntime)

#
# Setting C++ min standard
#
set(TRITON_MIN_CXX_STANDARD 17 CACHE STRING "The minimum C++ standard whose features are requested to build this target.")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#
# Dependencies
#
# FetchContent requires us to include the transitive closure of all
# repos that we depend on so that we can override the tags.
#
include(FetchContent)

configure_file(src/libtriton_ocr.ldscript libtriton_ocr.ldscript COPYONLY)

add_library(
  triton-ocr-backend SHARED
  src/ocr.cc
  src/ocr_det.cpp
  src/text_det.cpp
  src/text_rec.cpp
)

add_library(
  TutorialOcrBackend::triton-ocr-backend ALIAS triton-ocr-backend
)

target_include_directories(
  triton-ocr-backend
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    ${OpenCV_INCLUDE_DIRS}
    ${ONNXRUNTIME_INCLUDE_DIRS}
)

target_compile_features(triton-ocr-backend PRIVATE cxx_std_${TRITON_MIN_CXX_STANDARD})
target_compile_options(
  triton-ocr-backend PRIVATE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wno-unused-parameter -Wno-type-limits -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/Wall /D_WIN32_WINNT=0x0A00 /EHsc /Zc:preprocessor>
)

target_link_libraries(
  triton-ocr-backend
  PRIVATE
    ${TRITONCORE_LIBRARIES}
    ${TRITONCOMMON_LIBRARIES}
    ${TRITONBACKEND_LIBRARIES}
    ${OpenCV_LIBS}
    ${ONNXRUNTIME_LIBRARIES}
)

# 构建仓库目录

add_custom_command(TARGET triton-ocr-backend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:triton-ocr-backend>/model_repos/ocr_ppocr/1/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.pbtxt
    $<TARGET_FILE_DIR:triton-ocr-backend>/model_repos/ocr_ppocr/)

add_custom_command(TARGET triton-ocr-backend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:triton-ocr-backend>
    $<TARGET_FILE_DIR:triton-ocr-backend>/model_repos/ocr_ppocr/1)

if(WIN32)
  set_target_properties(
    triton-ocr-backend PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME triton_ocr
  )
else()
  set_target_properties(
    triton-ocr-backend PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME triton_ocr
    LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libtriton_ocr.ldscript
    LINK_FLAGS "-Wl,--version-script libtriton_ocr.ldscript"
  )
endif()

#
# Install
#
include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/TutorialOcrBackend)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/TutorialOcrBackendConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/TutorialOcrBackendConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

export(PACKAGE TutorialOcrBackend)
