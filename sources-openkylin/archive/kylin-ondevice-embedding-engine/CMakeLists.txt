cmake_minimum_required(VERSION 3.7)

project(kylin-ondevice-embedding-engine)

set(TARGET kylin-ondevice-embedding-engine)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(UUID REQUIRED IMPORTED_TARGET uuid)


find_package(nlohmann_json REQUIRED)
find_package(OpenCV REQUIRED)
find_package(TritonClient REQUIRED)
find_package(KylinAiEngine CONFIG REQUIRED)

set(SOURCE_LIST
    src/embeddingclient.cpp
    src/utils.cpp
)

# 单独指定头文件目录
set(HEADER_FILES
    src/embeddingclient.h
    src/utils.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${TRITON_CLIENT_PATH}
    ${OpenCV_INCLUDE_DIRS}
)


add_library(${TARGET} SHARED
    ondeviceembeddingengine.h 
    ondeviceembeddingengine.cpp
    ${SOURCE_LIST}
    ${HEADER_FILES}
)

set_target_properties(${TARGET} PROPERTIES VERSION 1.0.0 SOVERSION 1)
set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${TARGET}
    PkgConfig::UUID
    ${OpenCV_LIBS}
    TritonClient::grpcclient
    nlohmann_json::nlohmann_json
)

install(TARGETS ${TARGET}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

set(HEADER_PATH kylin-ai/plugins/ai-engines)
install(FILES ondeviceembeddingengine.h DESTINATION include/${HEADER_PATH})

if (DEFINED ENABLE_TEST)
    unset(ENABLE_TEST CACHE)
endif(DEFINED ENABLE_TEST)
option(ENABLE_TEST "Build Test" OFF)
if (ENABLE_TEST)
    add_subdirectory(tests)
endif (ENABLE_TEST)