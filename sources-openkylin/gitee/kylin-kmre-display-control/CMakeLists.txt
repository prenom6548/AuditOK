# Don't treat any warnings as error as we take the source directly from
# upstream and just compile it.
cmake_minimum_required(VERSION 2.8.2)

set(CMAKE_C_FLAGS "-Wall -DIOSTREAM_NAMESPACE='\"kmre::DisplayControl::\"'")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -DIOSTREAM_NAMESPACE='\"kmre::DisplayControl::\"'")

# Ensure -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/shared
    ${CMAKE_SOURCE_DIR}/shared/emugl/common
    ${CMAKE_SOURCE_DIR}/host/include
    ${CMAKE_SOURCE_DIR}/host/include/libDisplayControl
    ${CMAKE_SOURCE_DIR}/host/include/DisplayControl
    ${CMAKE_SOURCE_DIR}/host/libs/libDisplayControl
    ${CMAKE_SOURCE_DIR}/host/libs/displayControl_dec
    ${MIRCLIENT_INCLUDE_DIRS})

add_subdirectory(host)
add_subdirectory(shared)
add_subdirectory(system)
