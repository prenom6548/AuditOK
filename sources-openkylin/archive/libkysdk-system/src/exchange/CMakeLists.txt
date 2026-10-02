cmake_minimum_required(VERSION 3.16)

# Find required packages
find_package(X11 REQUIRED)
if(NOT X11_FOUND)
    message(FATAL_ERROR "X11 library not found. Please install libx11-dev package.")
endif()

aux_source_directory(. SOURCESCODE)
include_directories(.)
include_directories(${X11_INCLUDE_DIR})

add_library(kydataexchange SHARED ${SOURCESCODE})
set_target_properties(kydataexchange PROPERTIES VERSION 3.0.0 SOVERSION 1)

add_executable(kydataexchange-test test/kyclipboard-test.c)
target_link_libraries(kydataexchange kylog ${X11_LIBRARIES})
target_link_libraries(kydataexchange-test kydataexchange ${X11_LIBRARIES})
target_include_directories(kydataexchange-test PRIVATE ${X11_INCLUDE_DIR})

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
install(TARGETS kydataexchange
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES libkydataexchange.h
        DESTINATION include/kysdk/kysdk-system)
