cmake_minimum_required(VERSION 3.10)
project(plat-sec-patrol VERSION 0.1.0 LANGUAGES C)

include(FindPkgConfig)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 编译选项
add_compile_options(-D_GNU_SOURCE -Wall -Wextra -Werror -pedantic)
add_compile_options(-fPIC)

# 线程支持
find_package(Threads REQUIRED)

# 线程库
find_package(Threads REQUIRED)

# ====================
# 公共库（核心工具和插件共用）
# ====================
add_library(plat_common SHARED
    common/logger.c
    common/plugin_env_utils.c
    common/cJSON.c
    src/utils/string_utils.c
    src/utils/file_utils.c
)

target_include_directories(plat_common PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/common/include
)

target_link_libraries(plat_common 
    m
)

# 设置共享库的输出名称和版本
set_target_properties(plat_common PROPERTIES
    OUTPUT_NAME "plat_common"
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

# ====================
# 可执行文件（直接编译所有核心源码）
# ====================
add_executable(plat-sec-patrol 
    src/main.c
    src/core/cli.c
    src/core/base_collector.c
    src/core/capability_parser.c
    src/core/judge.c
    src/core/executor.c
    src/core/extension.c
    src/models/environment.c
    src/models/capability.c
    src/models/result.c
    src/utils/cmd_runner.c
)

target_include_directories(plat-sec-patrol PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common
)

target_link_libraries(plat-sec-patrol
    plat_common
    ${CMAKE_DL_LIBS}
    m
    Threads::Threads
)

# 根据安装前缀动态定义插件目录，避免硬编码
target_compile_definitions(plat-sec-patrol PRIVATE
    DEFAULT_PLUGIN_DIR="${CMAKE_INSTALL_PREFIX}/lib/plat-sec-patrol/plugins"
)

# 设置运行时库搜索路径
set_target_properties(plat-sec-patrol PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# ====================
# 插件
# ====================
# 环境感知插件
add_subdirectory(plugins/environment/cpu_collector)
add_subdirectory(plugins/environment/firmware_collector)
add_subdirectory(plugins/environment/os_collector)

# 能力库插件
add_subdirectory(plugins/capability/hygon)

# ====================
# 安装规则
# ====================
install(TARGETS plat-sec-patrol DESTINATION bin)

# 插件安装规则由各插件子目录的 CMakeLists.txt 自行定义：
#   - plugins/environment/*/  → lib/plat-sec-patrol/plugins/environment/
#   - plugins/capability/*/   → lib/plat-sec-patrol/plugins/capability/

# 安装公共库（仅安装共享库，静态库不安装到运行时包）
# 明确指定 LIBRARY DESTINATION 为 lib，避免在 64 位系统上安装到 lib64
install(TARGETS plat_common
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# 安装公共 API 头文件（仅安装外部需要的头文件）
# 1. 主头文件
install(FILES include/plat_sec_patrol.h 
    DESTINATION include/plat-sec-patrol
)

# 2. 数据模型头文件（供外部使用）
install(DIRECTORY include/models/ 
    DESTINATION include/plat-sec-patrol/models
    FILES_MATCHING PATTERN "*.h"
)

# 3. 插件接口头文件（供插件开发者使用）
install(DIRECTORY include/plugins/ 
    DESTINATION include/plat-sec-patrol/plugins
    FILES_MATCHING PATTERN "*.h"
)

# 注意：不安装以下内部头文件
# - include/core/     # 核心业务逻辑，内部实现
# - include/utils/    # 工具函数，内部实现

# 安装文档和许可证
install(FILES README.md LICENSE DESTINATION share/doc/plat-sec-patrol)

# ====================
# 卸载目标 (make uninstall)
# ====================
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
)

# ====================
# 测试（可选）
# ====================
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# ====================
# 配置摘要
# ====================
message(STATUS "")
message(STATUS "Configuration summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
if(BUILD_TESTING)
    message(STATUS "  Build tests: ON")
    if(CUNIT_FOUND OR (CUNIT_INCLUDE_DIR AND CUNIT_LIBRARY))
        message(STATUS "  CUnit framework: Found")
    else()
        message(STATUS "  CUnit framework: NOT FOUND - tests will fail to build")
    endif()
else()
    message(STATUS "  Build tests: OFF")
endif()
message(STATUS "")
