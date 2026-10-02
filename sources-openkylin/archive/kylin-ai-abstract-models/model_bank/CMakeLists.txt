cmake_minimum_required(VERSION 3.14)

project(tutorialbackend LANGUAGES C CXX)

option(TRITON_ENABLE_GPU "Enable GPU support in backend" OFF)
option(TRITON_ENABLE_STATS "Include statistics collections in backend" ON)
option(ENABLE_KYML "Enable kyml hardware device detection" OFF)

FIND_PACKAGE(TritonCore REQUIRED)
FIND_PACKAGE(TritonCommon REQUIRED)
FIND_PACKAGE(TritonBackend REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)
FIND_PACKAGE(nlohmann_json REQUIRED)
pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)

set(TRITON_MIN_CXX_STANDARD 17 CACHE STRING "The minimum C++ standard whose features are requested to build this target.")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

configure_file(src/libtriton_model_bank.ldscript libtriton_model_bank.ldscript COPYONLY)

add_library(
  triton-model_bank-backend SHARED
  src/modelbank.cpp
  src/utils.cpp
)

add_library(
  TutorialModelBankBackend::triton-model_bank-backend ALIAS triton-model_bank-backend
)

target_include_directories(
  triton-model_bank-backend
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

target_compile_features(triton-model_bank-backend PRIVATE cxx_std_${TRITON_MIN_CXX_STANDARD})

target_link_libraries(
  triton-model_bank-backend
  PRIVATE
    ${TRITONCORE_LIBRARIES}
    ${TRITONCOMMON_LIBRARIES}
    ${TRITONBACKEND_LIBRARIES}
    ${YAML_CPP_LIBRARIES}
)

if(ENABLE_KYML)
  target_link_libraries(
    triton-model_bank-backend
    PRIVATE
      kyml
  )
  target_compile_definitions(triton-model_bank-backend PRIVATE KYML_ENABLED)
endif()

# 构建仓库目录

add_custom_command(TARGET triton-model_bank-backend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:triton-model_bank-backend>/model_repos/model_bank/1/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.pbtxt
    $<TARGET_FILE_DIR:triton-model_bank-backend>/model_repos/model_bank/)

add_custom_command(TARGET triton-model_bank-backend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/src/default_model.yaml
    $<TARGET_FILE_DIR:triton-model_bank-backend>/model_repos/model_bank/1)

add_custom_command(TARGET triton-model_bank-backend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modeldevicebinding.json
    $<TARGET_FILE_DIR:triton-model_bank-backend>/model_repos/model_bank/1)

add_custom_command(TARGET triton-model_bank-backend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:triton-model_bank-backend>
    $<TARGET_FILE_DIR:triton-model_bank-backend>/model_repos/model_bank/1)

if(WIN32)
  set_target_properties(
    triton-model_bank-backend PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME triton_model_bank
  )
else()
  set_target_properties(
    triton-model_bank-backend PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME triton_model_bank
    LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libtriton_model_bank.ldscript
    LINK_FLAGS "-Wl,--version-script libtriton_model_bank.ldscript"
  )
endif()
