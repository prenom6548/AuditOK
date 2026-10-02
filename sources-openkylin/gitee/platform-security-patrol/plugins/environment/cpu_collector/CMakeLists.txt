cmake_minimum_required(VERSION 3.10)
project(cpu_collector C)

set(CMAKE_C_STANDARD 11)

# 编译为共享库，目标名为 cpu_hygon（不含lib前缀）
add_library(cpu_hygon_collector SHARED cpu_collector.c)

# 包含项目头文件目录
target_include_directories(cpu_hygon_collector PRIVATE 
    ${CMAKE_SOURCE_DIR}/../../../include
    ${CMAKE_SOURCE_DIR}/../../../common
    ${CMAKE_SOURCE_DIR}/../../../common/include
)

# 链接共享库 plat_common
target_link_libraries(cpu_hygon_collector
    plat_common
    ${CMAKE_DL_LIBS}
)

# 设置运行时库搜索路径，确保能找到 libplat_common.so
# 插件位于 lib/plat-sec-patrol/plugins/environment/，需向上 3 级到 lib/
set_target_properties(cpu_hygon_collector PROPERTIES
    INSTALL_RPATH "$ORIGIN/../../../lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# 安装规则（仅安装共享库到插件子目录）
install(TARGETS cpu_hygon_collector
    LIBRARY DESTINATION lib/plat-sec-patrol/plugins/environment
)
