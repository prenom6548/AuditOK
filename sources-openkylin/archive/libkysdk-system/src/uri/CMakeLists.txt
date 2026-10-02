include(FindPkgConfig)
pkg_check_modules(CURL REQUIRED libcurl)
include_directories(${CURL_INCLUDE_DIRS})

pkg_check_modules(SOUP REQUIRED libsoup-2.4)
include_directories(${SOUP_INCLUDE_DIRS})

add_library(kyuri SHARED libkyuri.c)
set_target_properties(kyuri PROPERTIES VERSION 3.0.0 SOVERSION 1)
add_executable(kyuri-test test/kyuri-test.c)
target_link_libraries(kyuri kylog ${CURL_LIBRARIES} ${SOUP_LIBRARIES})
target_link_libraries(kyuri-test kyuri)

# 包含 GNU 安装目录设置  
include(GNUInstallDirs)

# 设置 CMAKE_INSTALL_LIBDIR 变量  
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv8l")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/arm-linux-gnueabihf")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/mips64el-linux-gnuabi64")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/i386-linux-gnu")
else()
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu")
endif()

# 安装库文件  
install(TARGETS kyuri
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES libkyuri.h
        DESTINATION include/kysdk/kysdk-system)