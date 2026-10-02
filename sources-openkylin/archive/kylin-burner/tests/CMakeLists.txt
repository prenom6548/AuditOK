cmake_minimum_required(VERSION 3.5)

project(kylin-burner-tests)

set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR}/..)

#import burn engine lib
add_subdirectory(${PROJECT_ROOT_DIR}/lib/ky-burn-engine  ${PROJECT_ROOT_DIR}/tests/build/ky-burn-engine_build)
add_subdirectory(${PROJECT_ROOT_DIR}/lib/ky-disc-info ${PROJECT_ROOT_DIR}/tests/build/ky-disc-info_build)
add_subdirectory(${PROJECT_ROOT_DIR}/lib/ky-udf-image ${PROJECT_ROOT_DIR}/tests/build/ky-udf-image_build)

# 自动处理信号和槽机制所需的MOC文件
# 解决undefined reference to vtable问题
set(CMAKE_AUTOMOC ON)

# 自动处理从.ui文件生成相应的UI类
set(CMAKE_AUTOUIC ON)

# 自动处理Qt资源文件
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")

# 设置 C++ 编译器标志
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O0 -Wall")


# 启用测试
enable_testing()

# 定义单元测试宏，用于标记所有单元测试目标
# 在C++代码中可以使用 #ifdef UNIT_TEST 或 #if defined(UNIT_TEST) 来检查
add_definitions(-DUNIT_TEST)

# 查找Qt5
find_package(Qt5 COMPONENTS Widgets Test DBus X11Extras REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5WindowSystem)

# 查找项目依赖
pkg_check_modules(KYSDKQTWIDGETS_PKG kysdk-qtwidgets)
pkg_check_modules(GLIB2 glib-2.0)
pkg_check_modules(KYSDKSYSINFO_PKG kysdk-sysinfo)
pkg_check_modules(KYSDKALM_PKG kysdk-alm)
pkg_check_modules(KYSDKAPPLICATION_PKG kysdk-application)
pkg_check_modules(KYSDKBASE_PKG kysdk-base)
pkg_check_modules(KYSDKSOUNDEFFECTS_PKG kysdk-soundeffects)
pkg_check_modules(KYSDKWAYLANDHELPER_PKG kysdk-waylandhelper)
pkg_check_modules(DATACOLLECT kysdk-datacollect)


set(COMMON_TEST_FILE
    ${PROJECT_ROOT_DIR}/tests/test_common/test_helpers.h
    ${PROJECT_ROOT_DIR}/tests/test_common/test_helpers.cpp
    ${PROJECT_ROOT_DIR}/src/frame/logger.h
    ${PROJECT_ROOT_DIR}/src/frame/logger.cpp
    ${PROJECT_ROOT_DIR}/src/frame/configuration.h
    ${PROJECT_ROOT_DIR}/src/frame/configuration.cpp
)

set(COMMON_DEPEND_LIB
    Qt5::Test
    Qt5::Widgets
    Qt5::DBus
    Qt5::X11Extras
    KF5::WindowSystem
    X11
    pthread
    gsettings-qt
    ukui-log4qt
    gcov
    ${GLIB2_LIBRARIES}
    ${KYSDKQTWIDGETS_PKG_LIBRARIES}
    ${KYSDKAPPLICATION_PKG_LIBRARIES}
    ${KYSDKBASE_PKG_LIBRARIES}
    ${KYSDKALM_PKG_LIBRARIES}
    ${KYSDKSYSINFO_PKG_LIBRARIES}
    ${KYSDKSOUNDEFFECTS_PKG_LIBRARIES}
    ${DATACOLLECT_LIBRARIES}
    ${KYSDKWAYLANDHELPER_PKG_LIBRARIES}
    )

include_directories(
    ${PROJECT_ROOT_DIR}
    ${PROJECT_ROOT_DIR}/src
    ${PROJECT_ROOT_DIR}/src/common
    ${PROJECT_ROOT_DIR}/src/data
    ${PROJECT_ROOT_DIR}/src/frame
    ${PROJECT_ROOT_DIR}/src/model
    ${PROJECT_ROOT_DIR}/src/process
    ${PROJECT_ROOT_DIR}/src/status-fix
    ${PROJECT_ROOT_DIR}/lib
    ${PROJECT_ROOT_DIR}/tests
    ${PROJECT_ROOT_DIR}/lib/burner-utils-lib/include
    ${Qt5Test_INCLUDE_DIRS}
    ${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${KYSDKSYSINFO_PKG_INCLUDE_DIRS}
    ${KYSDKBASE_PKG_INCLUDE_DIRS}
    ${KYSDKSOUNDEFFECTS_PKG_INCLUDE_DIRS}
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
    ${KYSDKSOUNDEFFECTS_PKG_INCLUDE_DIRS}
)


# 添加测试目录
add_subdirectory(test_common)
add_subdirectory(test_module_common)
add_subdirectory(test_module_data)
add_subdirectory(test_module_model)
add_subdirectory(test_module_frame)
add_subdirectory(test_module_process)
add_subdirectory(test_module_status_fix)
add_subdirectory(test_module_view)
