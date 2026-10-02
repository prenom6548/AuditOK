cmake_minimum_required(VERSION 3.10)
project(hygon_capability C)

set(CMAKE_C_STANDARD 11)

# 编译为共享库
add_library(hygongenuine_capability SHARED hygon_capability.c)

# 包含项目头文件目录
target_include_directories(hygongenuine_capability PRIVATE 
    ${CMAKE_SOURCE_DIR}/../../../include
    ${CMAKE_SOURCE_DIR}/../../../common
    ${CMAKE_SOURCE_DIR}/../../../common/include
)

# 链接依赖库（需要 cJSON）和公共库
find_package(PkgConfig REQUIRED)
target_link_libraries(hygongenuine_capability
    ${CJSON_LIBRARIES}
    plat_common
    ${CMAKE_DL_LIBS}
)

# 根据安装前缀动态定义能力定义文件目录，避免硬编码
target_compile_definitions(hygongenuine_capability PRIVATE
    CAPABILITY_DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/plat-sec-patrol/capabilities"
)

# 设置运行时库搜索路径
# 插件位于 lib/plat-sec-patrol/plugins/capability/，需向上 3 级到 lib/
set_target_properties(hygongenuine_capability PROPERTIES
    INSTALL_RPATH "$ORIGIN/../../../lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# 安装规则（仅安装共享库到插件子目录）
install(TARGETS hygongenuine_capability
    LIBRARY DESTINATION lib/plat-sec-patrol/plugins/capability
)

# 安装能力定义 JSON 文件
install(FILES hygon_capabilities.json 
    DESTINATION share/plat-sec-patrol/capabilities
)
