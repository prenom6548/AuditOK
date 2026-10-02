cmake_minimum_required(VERSION 3.10)
project(os_collector C)

set(CMAKE_C_STANDARD 11)

add_library(os_collector SHARED os_collector.c)

target_include_directories(os_collector PRIVATE 
    ${CMAKE_SOURCE_DIR}/../../../include
    ${CMAKE_SOURCE_DIR}/../../../common
    ${CMAKE_SOURCE_DIR}/../../../common/include
)

# 链接共享库 plat_common
target_link_libraries(os_collector
    plat_common
    ${CMAKE_DL_LIBS}
)

# 设置运行时库搜索路径
# 插件位于 lib/plat-sec-patrol/plugins/environment/，需向上 3 级到 lib/
set_target_properties(os_collector PROPERTIES
    INSTALL_RPATH "$ORIGIN/../../../lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# 安装规则（仅安装共享库到插件子目录）
install(TARGETS os_collector
    LIBRARY DESTINATION lib/plat-sec-patrol/plugins/environment
)