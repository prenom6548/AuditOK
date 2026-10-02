# Status-Fix模块测试的CMake配置
# 包含目录
include_directories(
    ${PROJECT_ROOT_DIR}/src
    ${PROJECT_ROOT_DIR}/src/common
    ${PROJECT_ROOT_DIR}/src/data
    ${PROJECT_ROOT_DIR}/src/frame
    ${PROJECT_ROOT_DIR}/src/model
    ${PROJECT_ROOT_DIR}/src/process
    ${PROJECT_ROOT_DIR}/src/status-fix
    ${PROJECT_ROOT_DIR}/lib
    ${Qt5Test_INCLUDE_DIRS}
    ${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${KYSDKSYSINFO_PKG_INCLUDE_DIRS}
    ${KYSDKBASE_PKG_INCLUDE_DIRS}
)

# 链接目录
link_directories(
    ${PROJECT_ROOT_DIR}/build/src
    ${PROJECT_ROOT_DIR}/build/lib/ky-burn-engine
    ${PROJECT_ROOT_DIR}/build/lib/ky-disc-info
    ${PROJECT_ROOT_DIR}/build/lib/ky-udf-image
    ${KYSDKQTWIDGETS_PKG_LIBRARY_DIRS}
    ${KYSDKSYSINFO_PKG_LIBRARY_DIRS}
    ${KYSDKBASE_PKG_LIBRARY_DIRS}
)


# Common头文件测试
add_executable(isofs_test
    isofs_test.cpp
    ${PROJECT_ROOT_DIR}/src/libisofs/isofs.h
    ${PROJECT_ROOT_DIR}/src/libisofs/isofs.cpp
    )
target_link_libraries(isofs_test
    Qt5::Test
    Qt5::Widgets
    ${KYSDKBASE_PKG_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

# DiscMonitor类测试
add_executable(discmonitor_test
    ${COMMON_TEST_FILE}
    discmonitor_test.cpp
    ${PROJECT_ROOT_DIR}/src/status-fix/discmonitor.cpp
    ${PROJECT_ROOT_DIR}/src/status-fix/discmonitor.h
    )
target_link_libraries(discmonitor_test
    ${COMMON_DEPEND_LIB}
)

# 添加CTest测试
add_test(NAME discmonitor_test COMMAND discmonitor_test)
add_test(NAME isofs_test COMMAND isofs_test)
