# 测试配置
cmake_minimum_required(VERSION 3.10)

# 查找 CUnit 测试框架
find_package(PkgConfig REQUIRED)
pkg_check_modules(CUNIT cunit)

if(NOT CUNIT_FOUND)
    find_path(CUNIT_INCLUDE_DIR NAMES CUnit/CUnit.h)
    find_library(CUNIT_LIBRARY NAMES cunit)
    
    if(CUNIT_INCLUDE_DIR AND CUNIT_LIBRARY)
        set(CUNIT_FOUND TRUE)
        set(CUNIT_INCLUDE_DIRS ${CUNIT_INCLUDE_DIR})
        set(CUNIT_LIBRARIES ${CUNIT_LIBRARY})
        message(STATUS "Found CUnit: ${CUNIT_LIBRARY}")
    else()
        message(FATAL_ERROR "CUnit not found. Please install CUnit to build tests.")
    endif()
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/common ${CUNIT_INCLUDE_DIRS})

# ====================
# 工具函数测试
# ====================
add_executable(test_string_utils test_string_utils.c)
target_link_libraries(test_string_utils plat_common ${CUNIT_LIBRARIES} m)
add_test(NAME test_string_utils COMMAND test_string_utils)

add_executable(test_file_utils test_file_utils.c)
target_link_libraries(test_file_utils plat_common ${CUNIT_LIBRARIES} m)
add_test(NAME test_file_utils COMMAND test_file_utils)

# ====================
# 核心模块测试
# ====================

# 能力解析器测试 - 直接编译相关源文件
add_executable(test_capability_parser
    test_capability_parser.c
    ../src/core/capability_parser.c
    ../src/models/capability.c
    ../src/models/environment.c
    ../src/models/result.c
)
target_link_libraries(test_capability_parser plat_common ${CUNIT_LIBRARIES} m)
add_test(NAME test_capability_parser COMMAND test_capability_parser)

# 判定引擎测试 - 直接编译相关源文件
add_executable(test_judge
    test_judge.c
    ../src/core/judge.c
    ../src/core/capability_parser.c
    ../src/models/capability.c
    ../src/models/environment.c
    ../src/models/result.c
)
target_link_libraries(test_judge plat_common ${CUNIT_LIBRARIES} m)
add_test(NAME test_judge COMMAND test_judge)

# Extension 模块测试（仅公共接口）
add_executable(test_extension
    test_extension.c
    ../src/core/extension.c
    ../src/core/capability_parser.c
    ../src/core/judge.c
    ../src/models/capability.c
    ../src/models/environment.c
    ../src/models/result.c
)
target_link_libraries(test_extension plat_common ${CUNIT_LIBRARIES} m dl)
add_test(NAME test_extension COMMAND test_extension)

# 设置测试环境变量
set_tests_properties(test_capability_parser PROPERTIES ENVIRONMENT "PLUGIN_DIR=${CMAKE_BINARY_DIR}/plugins")
set_tests_properties(test_judge PROPERTIES ENVIRONMENT "PLUGIN_DIR=${CMAKE_BINARY_DIR}/plugins")
set_tests_properties(test_extension PROPERTIES ENVIRONMENT "PLUGIN_DIR=${CMAKE_BINARY_DIR}/plugins")

message(STATUS "Tests configured successfully")
