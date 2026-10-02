cmake_minimum_required(VERSION 3.16)

project(service-support-backend LANGUAGES CXX)

set(SERVICE_SUPPORT_BACKEND_DIR ${CMAKE_CURRENT_LIST_DIR})
add_library(${PROJECT_NAME} STATIC)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_definitions(-DDEBUG_MODE)
endif()

pkg_check_modules(OPENSSL openssl)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)

find_package(Qt5 COMPONENTS Core Network REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Network)

find_package(Qt5 REQUIRED COMPONENTS DBus)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)

include_directories(${SERVICE_SUPPORT_BACKEND_DIR}/../../../kom/)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
include_directories(${TOP_DIR}/daemons/service/)

set(SRCS
${SERVICE_SUPPORT_BACKEND_DIR}/src/feedbackglobal.h
${SERVICE_SUPPORT_BACKEND_DIR}/src/informationclassitem.h
${SERVICE_SUPPORT_BACKEND_DIR}/src/informationclassitem.cpp
${SERVICE_SUPPORT_BACKEND_DIR}/src/informationitem.h
${SERVICE_SUPPORT_BACKEND_DIR}/src/informationitem.cpp
${SERVICE_SUPPORT_BACKEND_DIR}/src/settings.h
${SERVICE_SUPPORT_BACKEND_DIR}/src/settings.cpp
${SERVICE_SUPPORT_BACKEND_DIR}/feedbackmanager.h
${SERVICE_SUPPORT_BACKEND_DIR}/src/feedbackmanager.cpp
${SERVICE_SUPPORT_BACKEND_DIR}/src/feedbackmanagerlogic.h
${SERVICE_SUPPORT_BACKEND_DIR}/src/feedbackmanagerlogic.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${SERVICE_SUPPORT_BACKEND_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS})
