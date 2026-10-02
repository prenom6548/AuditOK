cmake_minimum_required(VERSION 3.5)

project(kyai-assistant LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#######################################################################
# Options
#######################################################################
option(ENABLE_TEST "Build Test" OFF)

# jsoncpp
find_package(jsoncpp REQUIRED)

# pkg-config
find_package(PkgConfig REQUIRED)

# glib
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

# kylin-ai-proto
find_package(KylinAiProto REQUIRED)
kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES assistantservice)

include_directories(include)
include_directories(src)
include_directories(include/kylin-ai/private)

add_library(kyai-assistant SHARED
    include/kylin-ai/private/osassistant/kyaiexport.h
    include/kylin-ai/private/osassistant/osassistant.h
    include/kylin-ai/private/osassistant/error.h
    promptidmanager.h
    promptidmanager.cpp
    error.cpp
    osassistant.cpp
    assistantserviceproxy.cpp
    serverproxy.cpp
    ${COMMON_PROTO_FILES}
)

# 添加库导出宏定义
target_compile_definitions(kyai-assistant PRIVATE KYAI_ASSISTANT_BUILDING_LIB)

# 设置符号可见性
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(kyai-assistant stdc++fs)
    endif ()
endif ()

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

set_target_properties(kyai-assistant PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)

target_link_libraries(
    kyai-assistant
    ${GIO_LIBRARIES}
    jsoncpp
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
install(TARGETS kyai-assistant
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/kylin-ai
    DESTINATION include)

install(DIRECTORY usr/ DESTINATION /usr)
