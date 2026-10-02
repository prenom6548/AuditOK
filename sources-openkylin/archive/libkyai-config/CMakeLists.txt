cmake_minimum_required(VERSION 3.5)

project(ky-ai-config LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#######################################################################
# Options
#######################################################################
option(ENABLE_TEST "Build Test" OFF)

find_package(jsoncpp REQUIRED)

find_package(PkgConfig REQUIRED)

find_package(KylinAiProto REQUIRED)
kylin_ai_generate_gdbus_proto_code(COMMON_PROTO_FILES configservice)

pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

include_directories(include)
include_directories(src)
include_directories(include/kylin-ai/private/aiconfig)

set(SDK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/modelconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/configserviceproxy.cpp
    ${COMMON_PROTO_FILES}
)

add_library(kyai-config SHARED ${SDK_SOURCES})

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(tests)
endif ()

set_target_properties(kyai-config PROPERTIES VERSION 1.0.0 SOVERSION 1)

target_link_libraries(
    kyai-config
    ${GIO_LIBRARIES}
    jsoncpp
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
install(TARGETS kyai-config
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/kylin-ai
    DESTINATION include)

install(DIRECTORY usr/ DESTINATION /usr)
