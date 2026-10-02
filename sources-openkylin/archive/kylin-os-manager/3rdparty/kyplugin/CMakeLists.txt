cmake_minimum_required(VERSION 3.16)

project(kyplugin LANGUAGES CXX)

set(KYPLUGIN_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

add_library(${PROJECT_NAME} SHARED)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME} PRIVATE dl)

set(SRCS
	"${KYPLUGIN_TOP_DIR}/dir.cpp"
	"${KYPLUGIN_TOP_DIR}/dlibrary.cpp"
	"${KYPLUGIN_TOP_DIR}/host.cpp"
	"${KYPLUGIN_TOP_DIR}/kyplugin.cpp"
	"${KYPLUGIN_TOP_DIR}/plugin_manager.cpp"
	"${KYPLUGIN_TOP_DIR}/provider.cpp")

set(HEADERS
	"${KYPLUGIN_TOP_DIR}/host.h"
	"${KYPLUGIN_TOP_DIR}/kyplugin.h"
	"${KYPLUGIN_TOP_DIR}/plugin_manager.h"
	"${KYPLUGIN_TOP_DIR}/provider.h")

target_include_directories(${PROJECT_NAME} PRIVATE ${KYPLUGIN_TOP_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/)
install(FILES ${HEADERS} DESTINATION /usr/include/kylin-os-manager-plugin/)
