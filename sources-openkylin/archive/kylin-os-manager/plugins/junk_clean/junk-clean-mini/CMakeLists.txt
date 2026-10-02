cmake_minimum_required(VERSION 3.16)

project(junk-clean-mini)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../)
set(JUNK__MINI_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SRCS
    "${JUNK__MINI_TOP_DIR}/main.cpp"
    "${JUNK__MINI_TOP_DIR}/junkcleanmini.cpp"
    "${JUNK__MINI_TOP_DIR}/../cleaner/boot_partition_cleaner.cpp"
    "${JUNK__MINI_TOP_DIR}/../cleaner/cleaner.h"
    "${JUNK__MINI_TOP_DIR}/../basefunc.cpp"
)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/kom/)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::DBus)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/kylin-os-manager/bin/)

