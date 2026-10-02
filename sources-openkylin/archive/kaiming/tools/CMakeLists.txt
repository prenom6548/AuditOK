cmake_minimum_required(VERSION 3.5)
project(vfspreload C)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 用于拦截和重定向容器的文件系统调用，实现文件隔离和路径重定向
add_library(vfspreload SHARED ${CMAKE_CURRENT_SOURCE_DIR}/vfsredirect.c) 

# 添加头文件搜索路径
target_include_directories(vfspreload PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}  
)

# 链接必要的系统库
target_link_libraries(vfspreload PRIVATE 
    dl      
    Threads::Threads 
)

# 安装配置
install(
    TARGETS vfspreload
    LIBRARY DESTINATION /opt/kaiming-tools/lib  
)
