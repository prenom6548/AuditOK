
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core DBus REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5DBus_INCLUDE_DIRS})

add_library(kylogrotate SHARED libkylogrotate.c)
set_target_properties(kylogrotate PROPERTIES VERSION 3.0.0 SOVERSION 1)

add_executable(kysdk-logrotate logrotate.cpp main.cpp)
target_link_libraries(kysdk-logrotate Qt5::Core Qt5::DBus)

add_executable(kylogrotate-test test/kylogrotate-test.c libkylogrotate.c)
target_link_libraries(kylogrotate-test kylog)

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
install(TARGETS kylogrotate
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
