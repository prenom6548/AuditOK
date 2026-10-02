# Configuration类测试
add_executable(configuration_test configuration_test.cpp ${COMMON_TEST_FILE})
target_link_libraries(configuration_test
    Qt5::Test
    Qt5::Widgets
    ${KYSDKBASE_PKG_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

# Logger类测试
add_executable(logger_test logger_test.cpp ${COMMON_TEST_FILE})
target_link_libraries(logger_test
    Qt5::Test
    Qt5::Widgets
    ${KYSDKBASE_PKG_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

# Signals类测试
add_executable(signals_test
    ${COMMON_TEST_FILE}
    signals_test.cpp
    ${PROJECT_ROOT_DIR}/src/frame/signals.cpp
    ${PROJECT_ROOT_DIR}/src/frame/signals.h
    )
target_link_libraries(signals_test
    Qt5::Test
    Qt5::Widgets
    ${KYSDKBASE_PKG_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

# Signals类测试
add_executable(auditlog_test
     auditlog_test.cpp
     ${COMMON_TEST_FILE}
     ${PROJECT_ROOT_DIR}/src/frame/auditlog.cpp
     ${PROJECT_ROOT_DIR}/src/frame/auditlog.h
    )
target_link_libraries(auditlog_test
    Qt5::Test
    Qt5::Widgets
    ${KYSDKBASE_PKG_LIBRARIES}
    ${GLIB2_LIBRARIES}
)

# 设置测试覆盖率
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(configuration_test PRIVATE -fprofile-arcs -ftest-coverage)
    target_compile_options(logger_test PRIVATE -fprofile-arcs -ftest-coverage)
    target_compile_options(signals_test PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_libraries(configuration_test gcov)
    target_link_libraries(logger_test gcov)
    target_link_libraries(signals_test gcov)
endif()

# 添加CTest测试
add_test(NAME configuration_test COMMAND configuration_test)
add_test(NAME logger_test COMMAND logger_test)
add_test(NAME signals_test COMMAND signals_test)
