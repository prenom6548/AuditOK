cmake_minimum_required(VERSION 3.10) 
project(DbusProxy)  

# 设置C++标准
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

# 查找依赖包 
find_package(PkgConfig REQUIRED) 

# 查找glib和gio库
pkg_check_modules(GLIB REQUIRED glib-2.0) 
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GIO_UNIX REQUIRED gio-unix-2.0)

# 包含头文件目录
include_directories(${GLIB_INCLUDE_DIRS}) 
include_directories(${GIO_INCLUDE_DIRS})  
include_directories(${GIO_UNIX_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# 链接库目录
link_directories(${GLIB_LIBRARY_DIRS})  
link_directories(${GIO_LIBRARY_DIRS})   
link_directories(${GIO_UNIX_LIBRARY_DIRS})

# 添加编译选项
add_compile_options(${GLIB_CFLAGS_OTHER})   
add_compile_options(${GIO_CFLAGS_OTHER})   
add_compile_options(${GIO_UNIX_CFLAGS_OTHER})

# 创建可执行文件
add_executable(km-ses-dbusproxy KMDbusProxy.cpp KMDbusProxySide.cpp ../common/KMLogger.cpp)

# 链接所需的库
target_link_libraries(km-ses-dbusproxy 
    ${GLIB_LIBRARIES}  
    ${GIO_LIBRARIES}
    ${GIO_UNIX_LIBRARIES}
)

# 设置输出目录（可选）
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

install( TARGETS km-ses-dbusproxy
    RUNTIME DESTINATION  /opt/kaiming-tools/bin
)