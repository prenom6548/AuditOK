cmake_minimum_required(VERSION 3.5)

project(tests)

option(UNIT_TEST "option for unit test" ON)

if (UNIT_TEST)
    add_definitions(-DUNIT_TEST)
endif(UNIT_TEST)

add_subdirectory(kt-test-utils)
add_subdirectory(lib-kylin-printer)
add_subdirectory(unit_test_kylin-printer-exe)
add_subdirectory(unit_test_kylin-printer)
add_subdirectory(unit_test_kylin-printer1)
add_subdirectory(unit_test_kylin-printer2)
add_subdirectory(unit_test_kylin-printer3)
add_subdirectory(unit_test_kylin-printer-backend)
add_subdirectory(unit_test_kylin-printer-backend-exe)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
