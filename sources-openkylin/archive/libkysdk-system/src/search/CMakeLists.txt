find_package(PkgConfig REQUIRED)
pkg_check_modules(MAGICKWAND REQUIRED MagickWand)
include_directories(${MAGICKWAND_INCLUDE_DIRS})

add_library(kysearch SHARED libkysearch.c)
set_target_properties(kysearch PROPERTIES VERSION 3.0.0 SOVERSION 1)
target_link_libraries(kysearch kylog magic sqlite3 ${MAGICKWAND_LIBRARIES})
target_link_options(kysearch PRIVATE "-Wl,-z,now")

add_executable(kysearch-test test/kysearch-test.c)
target_link_libraries(kysearch-test kysearch)

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
install(TARGETS kysearch
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES libkysearch.h
        DESTINATION include/kysdk/kysdk-system)