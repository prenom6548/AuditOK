add_subdirectory(unit_test_giodbus)
add_subdirectory(unit_test_biodeviceinfo)
add_subdirectory(unit_test_kalabel)
add_subdirectory(unit_test_rsac)
add_subdirectory(unit_test_pam_tally)
add_subdirectory(unit_test_common)
add_subdirectory(unit_test_loginoptionswidget)
add_subdirectory(unit_test_keywatcher)
add_subdirectory(unit_test_personalizeddata)
add_subdirectory(unit_test_modebutton)
add_subdirectory(unit_test_users)
add_subdirectory(unit_test_biodevices)
add_subdirectory(unit_test_usdblockshortcut)
add_subdirectory(unit_test_sessionmanager)
add_subdirectory(unit_test_servicemanager)

# 处理打桩工具相关配置
set(TEST_UTILS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/kt-test-utils")

# 收集打桩工具的头文件，递归查找符合条件的头文件
file(GLOB_RECURSE UTILS_HEADERS
    "${TEST_UTILS_PATH}/cpp-stub/*.h"
    "${TEST_UTILS_PATH}/cpp-stub/*.hpp"
    "${TEST_UTILS_PATH}/cpp-stub-ext/*.h"
)

# 收集打桩工具的源文件，递归查找符合条件的源文件
file(GLOB_RECURSE UTILS_SOURCES
    "${TEST_UTILS_PATH}/cpp-stub/*.cpp"
    "${TEST_UTILS_PATH}/cpp-stub-ext/*.cpp"
)

# 创建一个库（这里以静态库为例，可以根据需求改为共享库等）来存放打桩工具相关代码
add_library(utils_lib STATIC ${UTILS_SOURCES} ${UTILS_HEADERS})

# 将打桩工具库的头文件目录添加到包含路径，方便其他目标使用
target_include_directories(utils_lib
    PUBLIC
        "${TEST_UTILS_PATH}/cpp-stub"
        "${TEST_UTILS_PATH}/cpp-stub-ext"
)

