cmake_minimum_required(VERSION 3.1.3)

project(kba)

include_directories(../include/kba)

link_directories(${LIBRARY_OUTPUT_PATH})

set(bin_SRCS
    ${bin_SRCS}
    test.c
    )

add_executable(kba-test ${bin_SRCS})
target_link_libraries(kba-test -lkba -ljson-c)

install(TARGETS kba-test DESTINATION sbin)

