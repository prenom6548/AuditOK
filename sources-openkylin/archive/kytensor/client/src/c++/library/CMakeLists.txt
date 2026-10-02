# Copyright (c) 2020-2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

cmake_minimum_required (VERSION 3.16)

execute_process(
  COMMAND lsb_release -d
  OUTPUT_VARIABLE LINUX_DISTRO
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "System: ${LINUX_DISTRO}")

find_package(Threads REQUIRED)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

#
# common
#
add_library(
    client-common-library INTERFACE
)

target_include_directories(
  client-common-library
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

#
# json_utils
#
if(TRITON_ENABLE_CC_HTTP OR TRITON_ENABLE_EXAMPLES)
  find_package(RapidJSON CONFIG REQUIRED)
  add_library(
      json-utils-library EXCLUDE_FROM_ALL OBJECT
      json_utils.h json_utils.cc
  )
  target_include_directories(
    json-utils-library
    PUBLIC ${RapidJSON_INCLUDE_DIRS}
  )
  target_link_libraries(
    json-utils-library
    PRIVATE
      client-common-library
  )

  add_library(
    json_utils_static STATIC
    $<TARGET_OBJECTS:json-utils-library>
  )

  add_library(
      TritonClient::json_utils_static ALIAS json_utils_static
  )

  foreach(_json_target json-utils-library json_utils_static)
    target_compile_features(${_json_target} PRIVATE cxx_std_${TRITON_MIN_CXX_STANDARD})
    target_compile_options(
      ${_json_target} PRIVATE
      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wextra -Werror>
      $<$<CXX_COMPILER_ID:MSVC>:/W0 /D_WIN32_WINNT=0x0A00 /EHsc>
    )

    set_target_properties(
      ${_json_target}
      PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )

    target_include_directories(
      ${_json_target}
      PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    install(
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/json_utils.h
        DESTINATION include
    )
  endforeach()

  install(
    TARGETS
      json_utils_static
    EXPORT
      triton-client-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

endif() # TRITON_ENABLE_CC_HTTP OR TRITON_ENABLE_EXAMPLES

#
# shm_utils
#
add_library(
    shm-utils-library EXCLUDE_FROM_ALL OBJECT
    shm_utils.h shm_utils.cc
)
target_link_libraries(
  shm-utils-library
  PRIVATE
    client-common-library
)

if(NOT WIN32)
  add_library(
    shm_utils_static STATIC
    $<TARGET_OBJECTS:shm-utils-library>
  )

  add_library(
      TritonClient::shm_utils_static ALIAS shm_utils_static
  )

  foreach(_shm_target shm-utils-library shm_utils_static)
    target_compile_features(${_shm_target} PRIVATE cxx_std_${TRITON_MIN_CXX_STANDARD})
    target_compile_options(
      ${_shm_target} PRIVATE
      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wextra -Werror>
      $<$<CXX_COMPILER_ID:MSVC>:/W0 /D_WIN32_WINNT=0x0A00 /EHsc>
    )

    set_target_properties(
      ${_shm_target}
      PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )

    target_include_directories(
      ${_shm_target}
      PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    install(
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/shm_utils.h
        DESTINATION include
    )
  endforeach()

  install(
    TARGETS
      shm_utils_static
    EXPORT
      triton-client-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endif(NOT WIN32)

if(TRITON_ENABLE_CC_GRPC)
  #
  # libgrpcclient.so and libgrpcclient_static.a
  #
  configure_file(libgrpcclient.ldscript libgrpcclient.ldscript COPYONLY)

  # libgrpcclient object build
  set(
      REQUEST_SRCS
      grpc_client.cc common.cc
  )

  set(
      REQUEST_HDRS
      grpc_client.h common.h ipc.h
  )

  add_library(
      grpc-client-library EXCLUDE_FROM_ALL OBJECT
      ${REQUEST_SRCS} ${REQUEST_HDRS}
  )
  add_dependencies(
      grpc-client-library
      grpc-service-library proto-library
  )

  # libgrpcclient_static.a
  add_library(
      grpcclient_static STATIC
      $<TARGET_OBJECTS:grpc-client-library>
      $<TARGET_OBJECTS:grpc-service-library>
      $<TARGET_OBJECTS:proto-library>
  )
  add_library(
      TritonClient::grpcclient_static ALIAS grpcclient_static
  )

  target_include_directories(
      grpcclient_static
      PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  )

  execute_process(
    COMMAND lsb_release -d
    OUTPUT_VARIABLE LINUX_DISTRO
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  message(STATUS "System: ${LINUX_DISTRO}")

  if (LINUX_DISTRO MATCHES "Kylin V10 SP1|Ubuntu 20\\.04.*LTS")

    target_link_libraries(
        grpcclient_static
        #PRIVATE gRPC::grpc++
        #PRIVATE gRPC::grpc
        #PUBLIC protobuf::libprotobuf
        #PUBLIC Threads::Threads
        libgrpc++.a
        libgrpc.a
        libcares_static.a
        libaddress_sorting.a
        libupb.a
        libabsl_statusor.a
        libgpr.a
        libabsl_status.a
        libabsl_strerror.a
        #libabsl_flags.a
        libabsl_flags_internal.a
        libabsl_flags_reflection.a
        libabsl_hash.a
        libabsl_city.a
        libabsl_bad_variant_access.a
        libabsl_low_level_hash.a
        libabsl_raw_hash_set.a
        libabsl_hashtablez_sampler.a
        libabsl_flags_config.a
        libabsl_flags_program_name.a
        libabsl_flags_private_handle_accessor.a
        libabsl_flags_commandlineflag.a
        libabsl_flags_commandlineflag_internal.a
        libabsl_flags_marshalling.a
        libabsl_random_distributions.a
        libabsl_random_seed_sequences.a
        libabsl_random_internal_pool_urbg.a
        libabsl_random_internal_randen.a
        libabsl_random_internal_randen_hwaes.a
        libabsl_random_internal_randen_hwaes_impl.a
        libabsl_random_internal_randen_slow.a
        libabsl_random_internal_platform.a
        libabsl_random_internal_seed_material.a
        libabsl_random_seed_gen_exception.a
        libabsl_cord.a
        libabsl_bad_optional_access.a
        libabsl_cordz_info.a
        libabsl_cord_internal.a
        libabsl_cordz_functions.a
        libabsl_exponential_biased.a
        libabsl_cordz_handle.a
        #libabsl_crc_cord_state.a
        #libabsl_crc32c.a
        #libabsl_crc_internal.a
        #libabsl_crc_cpu_detect.a
        libabsl_str_format_internal.a
        libabsl_synchronization.a
        libabsl_stacktrace.a
        libabsl_symbolize.a
        libabsl_debugging_internal.a
        libabsl_demangle_internal.a
        libabsl_graphcycles_internal.a
        libabsl_malloc_internal.a
        libabsl_time.a
        libabsl_strings.a
        libabsl_throw_delegate.a
        libabsl_int128.a
        libabsl_strings_internal.a
        libabsl_base.a
        libabsl_spinlock_wait.a
        libabsl_raw_logging_internal.a
        libabsl_log_severity.a
        libabsl_civil_time.a
        libabsl_time_zone.a
        -lssl
        -lcrypto
        -lz
        -lpthread
        -lrt
        libprotobuf.a
        libre2.a
    )
  else()
    target_link_libraries(
        grpcclient_static
        PRIVATE gRPC::grpc++
        PRIVATE gRPC::grpc
        PUBLIC protobuf::libprotobuf
        PUBLIC Threads::Threads
    )
  endif()
  # libgrpcclient.so
  add_library(
      grpcclient SHARED
      $<TARGET_OBJECTS:grpc-service-library>
      $<TARGET_OBJECTS:proto-library>
      $<TARGET_OBJECTS:grpc-client-library>
  )
  add_library(
      TritonClient::grpcclient ALIAS grpcclient
  )

  if (NOT WIN32 AND NOT TRITON_KEEP_TYPEINFO)
     set_target_properties(
       grpcclient
       PROPERTIES
         LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libgrpcclient.ldscript
         LINK_FLAGS "-Wl,--version-script=libgrpcclient.ldscript"
     )
  endif() # NOT WIN32 AND NOT TRITON_KEEP_TYPEINFO

  if (LINUX_DISTRO MATCHES "Kylin V10 SP1|Ubuntu 20\\.04.*LTS")
    target_link_libraries(
        grpcclient
        #PRIVATE gRPC::grpc++
        #PRIVATE gRPC::grpc
        #PUBLIC protobuf::libprotobuf
        #PUBLIC Threads::Threads
        libgrpc++.a
        libgrpc.a
        libcares_static.a
        libaddress_sorting.a
        libupb.a
        libabsl_statusor.a
        libgpr.a
        libabsl_status.a
        libabsl_strerror.a
        #libabsl_flags.a
        libabsl_flags_internal.a
        libabsl_flags_reflection.a
        libabsl_hash.a
        libabsl_city.a
        libabsl_bad_variant_access.a
        libabsl_low_level_hash.a
        libabsl_raw_hash_set.a
        libabsl_hashtablez_sampler.a
        libabsl_flags_config.a
        libabsl_flags_program_name.a
        libabsl_flags_private_handle_accessor.a
        libabsl_flags_commandlineflag.a
        libabsl_flags_commandlineflag_internal.a
        libabsl_flags_marshalling.a
        libabsl_random_distributions.a
        libabsl_random_seed_sequences.a
        libabsl_random_internal_pool_urbg.a
        libabsl_random_internal_randen.a
        libabsl_random_internal_randen_hwaes.a
        libabsl_random_internal_randen_hwaes_impl.a
        libabsl_random_internal_randen_slow.a
        libabsl_random_internal_platform.a
        libabsl_random_internal_seed_material.a
        libabsl_random_seed_gen_exception.a
        libabsl_cord.a
        libabsl_bad_optional_access.a
        libabsl_cordz_info.a
        libabsl_cord_internal.a
        libabsl_cordz_functions.a
        libabsl_exponential_biased.a
        libabsl_cordz_handle.a
        #libabsl_crc_cord_state.a
        #libabsl_crc32c.a
        #libabsl_crc_internal.a
        #libabsl_crc_cpu_detect.a
        libabsl_str_format_internal.a
        libabsl_synchronization.a
        libabsl_stacktrace.a
        libabsl_symbolize.a
        libabsl_debugging_internal.a
        libabsl_demangle_internal.a
        libabsl_graphcycles_internal.a
        libabsl_malloc_internal.a
        libabsl_time.a
        libabsl_strings.a
        libabsl_throw_delegate.a
        libabsl_int128.a
        libabsl_strings_internal.a
        libabsl_base.a
        libabsl_spinlock_wait.a
        libabsl_raw_logging_internal.a
        libabsl_log_severity.a
        libabsl_civil_time.a
        libabsl_time_zone.a
        -lssl
        -lcrypto
        libprotobuf.a
        libre2.a
    )
  else()
    target_link_libraries(
        grpcclient
        PRIVATE gRPC::grpc++
        PRIVATE gRPC::grpc
        PUBLIC protobuf::libprotobuf
        PUBLIC Threads::Threads
    )
  endif()

  foreach(_client_target grpc-client-library grpcclient_static grpcclient)
    target_compile_features(${_client_target} PRIVATE cxx_std_${TRITON_MIN_CXX_STANDARD})
    target_compile_options(
      ${_client_target} PRIVATE
      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wextra -Werror>
      $<$<CXX_COMPILER_ID:MSVC>:/W0 /D_WIN32_WINNT=0x0A00 /EHsc>
    )

    set_target_properties(
      ${_client_target}
      PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )

    target_include_directories(
      ${_client_target}
      PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<TARGET_PROPERTY:proto-library,INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:grpc-service-library,INCLUDE_DIRECTORIES>
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    if(TRITON_ENABLE_GPU)
      target_compile_definitions(
        ${_client_target}
          PUBLIC TRITON_ENABLE_GPU=1
      )
    endif() # TRITON_ENABLE_GPU

    if(TRITON_ENABLE_GPU)
      target_link_libraries(
        ${_client_target}
        PUBLIC CUDA::cudart
      )
    endif() # TRITON_ENABLE_GPU
  endforeach()

  if (TRITON_USE_THIRD_PARTY)
    if (NOT WIN32)
      install(
        DIRECTORY
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/curl/lib/
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/grpc/lib/
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/protobuf/lib/
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/c-ares/lib/
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/absl/lib/
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/re2/lib/
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FILES_MATCHING
          PATTERN "*\.a"
          PATTERN "CMakeFiles" EXCLUDE
          PATTERN "cmake" EXCLUDE
          PATTERN "gens" EXCLUDE
          PATTERN "libs" EXCLUDE
          PATTERN "third_party" EXCLUDE
      )
    else()
      install(
        DIRECTORY
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/curl/lib/
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/grpc/lib/
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/protobuf/lib/
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/c-ares/lib/
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/absl/lib/
          ${CMAKE_CURRENT_BINARY_DIR}/../../third-party/re2/lib/
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FILES_MATCHING
          PATTERN "*\.lib"
          PATTERN "CMakeFiles" EXCLUDE
          PATTERN "cmake" EXCLUDE
          PATTERN "gens" EXCLUDE
          PATTERN "libs" EXCLUDE
          PATTERN "third_party" EXCLUDE
      )
    endif() # NOT WIN32

    install(
      DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/../../../third-party/grpc/include/
        ${CMAKE_CURRENT_BINARY_DIR}/../../../third-party/absl/include/
        ${CMAKE_CURRENT_BINARY_DIR}/../../../third-party/protobuf/include/
        ${CMAKE_CURRENT_BINARY_DIR}/../../../third-party/re2/include/
        DESTINATION include
    )
  endif()

  install(
      FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/grpc_client.h
      DESTINATION include
  )
endif() # TRITON_ENABLE_CC_GRPC

if(TRITON_ENABLE_CC_HTTP)
  if(${TRITON_ENABLE_ZLIB})
    find_package(ZLIB REQUIRED)
  endif() # TRITON_ENABLE_ZLIB
  #
  # libhttpclient.so and libhttpclient_static.a
  #
  configure_file(libhttpclient.ldscript libhttpclient.ldscript COPYONLY)

  # libhttpclient object build
  set(
      REQUEST_SRCS
      http_client.cc common.cc cencode.c
  )

  set(
      REQUEST_HDRS
      http_client.h common.h ipc.h cencode.h
  )

  add_library(
      http-client-library EXCLUDE_FROM_ALL OBJECT
      ${REQUEST_SRCS} ${REQUEST_HDRS}
  )

  if (NOT WIN32)
    set_property(
      SOURCE cencode.c
      PROPERTY COMPILE_FLAGS -Wno-implicit-fallthrough
    )
  endif() # NOT WIN32

  target_link_libraries(
    http-client-library
    PUBLIC
      triton-common-json        # from repo-common
  )

  # libhttpclient_static.a
  add_library(
      httpclient_static STATIC
      $<TARGET_OBJECTS:http-client-library>
  )
  add_library(
      TritonClient::httpclient_static ALIAS httpclient_static
  )

  target_link_libraries(
      httpclient_static
      PRIVATE triton-common-json
      PUBLIC CURL::libcurl
      PUBLIC Threads::Threads
  )

  if(${TRITON_ENABLE_ZLIB})
    target_link_libraries(
      httpclient_static
      PRIVATE ZLIB::ZLIB
    )
  endif() # TRITON_ENABLE_ZLIB

  # libhttpclient.so
  add_library(
      httpclient SHARED
      $<TARGET_OBJECTS:http-client-library>
  )
  add_library(
      TritonClient::httpclient ALIAS httpclient
  )

  if (NOT WIN32)
     set_target_properties(
       httpclient
       PROPERTIES
         LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libhttpclient.ldscript
         LINK_FLAGS "-Wl,--version-script=libhttpclient.ldscript"
     )
  endif() # NOT WIN32

  target_link_libraries(
      httpclient
      PRIVATE triton-common-json
      PUBLIC CURL::libcurl
      PUBLIC Threads::Threads
  )

  foreach(_client_target http-client-library httpclient_static httpclient)
    target_compile_features(${_client_target} PRIVATE cxx_std_${TRITON_MIN_CXX_STANDARD})
    target_compile_options(
      ${_client_target} PRIVATE
      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wextra -Werror>
      $<$<CXX_COMPILER_ID:MSVC>:/W0 /D_WIN32_WINNT=0x0A00 /EHsc>
    )

    set_target_properties(
      ${_client_target}
      PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )

    target_include_directories(
      ${_client_target}
      PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<TARGET_PROPERTY:CURL::libcurl,INTERFACE_INCLUDE_DIRECTORIES>
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_compile_definitions(
      ${_client_target}
        PRIVATE CURL_STATICLIB=1
    )
    if(TRITON_ENABLE_GPU)
      target_compile_definitions(
        ${_client_target}
          PUBLIC TRITON_ENABLE_GPU=1
      )
    endif() # TRITON_ENABLE_GPU

    if(TRITON_ENABLE_GPU)
      target_link_libraries(
        ${_client_target}
        PUBLIC CUDA::cudart
      )
    endif() # TRITON_ENABLE_GPU

    if(${TRITON_ENABLE_ZLIB})
      target_compile_definitions(
        ${_client_target}
        PUBLIC TRITON_ENABLE_ZLIB=1
      )
    endif() # TRITON_ENABLE_ZLIB
  endforeach()

  install(
      FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/http_client.h
      DESTINATION include
  )
endif() # TRITON_ENABLE_CC_HTTP

if(TRITON_ENABLE_CC_HTTP OR TRITON_ENABLE_CC_GRPC)
  install(
      FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/common.h
      ${CMAKE_CURRENT_SOURCE_DIR}/ipc.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cencode.h
      DESTINATION include
  )

  include(GNUInstallDirs)
  set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/TritonClient)

  if(TRITON_ENABLE_CC_GRPC)
    install(
      TARGETS
        #grpcclient
        grpcclient_static
      EXPORT
        triton-client-targets
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  endif() # TRITON_ENABLE_CC_GRPC

  if(TRITON_ENABLE_CC_HTTP)
    install(
      TARGETS
        httpclient
        httpclient_static
      EXPORT
        triton-client-targets
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  endif() # TRITON_ENABLE_CC_HTTP

  install(
    EXPORT
      triton-client-targets
    FILE
      TritonClientTargets.cmake
    NAMESPACE
      TritonClient::
    DESTINATION
      ${INSTALL_CONFIGDIR}
  )

  include(CMakePackageConfigHelpers)
  configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/TritonClientConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/TritonClientConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
  )

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/TritonClientConfig.cmake
    DESTINATION
      ${INSTALL_CONFIGDIR}
  )

  #
  # Export from build tree
  #
  export(
    EXPORT
      triton-client-targets
    FILE
      ${CMAKE_CURRENT_BINARY_DIR}/TritonClientTargets.cmake
    NAMESPACE
      TritonClient::
  )

  export(PACKAGE TritonClient)

endif() # TRITON_ENABLE_CC_HTTP OR TRITON_ENABLE_CC_GRPC
