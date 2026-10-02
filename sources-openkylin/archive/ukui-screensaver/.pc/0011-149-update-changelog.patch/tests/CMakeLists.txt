add_subdirectory(unit_test_biometric_helper)
add_subdirectory(unit_test_account_helper)
add_subdirectory(unit_test_freedesktop_helper)
add_subdirectory(unit_test_kglobalaccel_helper)
add_subdirectory(unit_test_login1_helper)
add_subdirectory(unit_test_session_helper)
add_subdirectory(unit_test_systemupgrade_helper)
add_subdirectory(unit_test_uniauth_service)
add_subdirectory(unit_test_usd_helper)
add_subdirectory(unit_test_gsettings_helper)
add_subdirectory(unit_test_engine_device)
add_subdirectory(unit_test_inputswitch_event)
add_subdirectory(unit_test_auth_pam)
add_subdirectory(unit_test_common)
add_subdirectory(unit_test_switch_user)
add_subdirectory(unit_test_session_watcher)
add_subdirectory(unit_test_display_service)
add_subdirectory(unit_test_dbus_interface)
add_subdirectory(unit_test_backend_dbus)
add_subdirectory(unit_test_lock_dialog_model)
add_subdirectory(unit_test_personal_data)
add_subdirectory(unit_test_screensaver)
add_subdirectory(unit_test_screenlock)

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

