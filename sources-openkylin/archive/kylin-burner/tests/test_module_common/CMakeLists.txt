# Common模块测试的CMake配置
# Utils类测试
add_executable(utils_test
	${COMMON_TEST_FILE}
	utils_test.cpp
	${PROJECT_ROOT_DIR}/src/common/utils.h
	${PROJECT_ROOT_DIR}/src/common/utils.cpp
        ${PROJECT_ROOT_DIR}/src/model/diroptutils.h
        ${PROJECT_ROOT_DIR}/src/model/diroptutils.cpp
)

target_link_libraries(utils_test
	${COMMON_DEPEND_LIB}
)

# DBCaller类测试
add_executable(dbcaller_test 
	${COMMON_TEST_FILE}
	dbcaller_test.cpp
	${PROJECT_ROOT_DIR}/src/common/dbuscaller.cpp
	${PROJECT_ROOT_DIR}/src/common/dbuscaller.h
)
target_link_libraries(dbcaller_test
	${COMMON_DEPEND_LIB}
)

# TrackingManager类测试
add_executable(trackingmanager_test
	${COMMON_TEST_FILE}
	trackingmanager_test.cpp 
	${PROJECT_ROOT_DIR}/src/common/trackingmanager.cpp
	${PROJECT_ROOT_DIR}/src/common/trackingmanager.h
)
target_link_libraries(trackingmanager_test
	${COMMON_DEPEND_LIB}
)


add_executable(encodingdetector_test
        ${COMMON_TEST_FILE}
        encodingdetector_test.cpp
        ${PROJECT_ROOT_DIR}/src/common/encodingdetector.cpp
        ${PROJECT_ROOT_DIR}/src/common/encodingdetector.h
)
target_link_libraries(encodingdetector_test
        ${COMMON_DEPEND_LIB}
)


# 设置测试覆盖率
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(utils_test PRIVATE -fprofile-arcs -ftest-coverage)
    target_compile_options(dbcaller_test PRIVATE -fprofile-arcs -ftest-coverage)
    target_compile_options(trackingmanager_test PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_libraries(utils_test gcov)
    target_link_libraries(dbcaller_test gcov)
    target_link_libraries(trackingmanager_test gcov)
endif()

# 添加CTest测试
add_test(NAME utils_test COMMAND utils_test)
add_test(NAME dbcaller_test COMMAND dbcaller_test)
add_test(NAME trackingmanager_test COMMAND trackingmanager_test)
