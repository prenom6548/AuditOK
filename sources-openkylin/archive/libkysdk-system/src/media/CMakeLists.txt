cmake_minimum_required(VERSION 3.16)

# Find required packages
find_package(ALSA REQUIRED)
if(NOT ALSA_FOUND)
    message(FATAL_ERROR "ALSA library not found. Please install libasound2-dev package.")
endif()

aux_source_directory(. SOURCESCODE)
include_directories(.)
include_directories(${ALSA_INCLUDE_DIRS})

add_library(kymedia SHARED ${SOURCESCODE})
set_target_properties(kymedia PROPERTIES VERSION 3.0.0 SOVERSION 1)

add_executable(kymedia-test test/kymedia-test.c)
target_link_libraries(kymedia PRIVATE ${ALSA_LIBRARIES})
target_link_libraries(kymedia-test PRIVATE kymedia ${ALSA_LIBRARIES})
target_include_directories(kymedia-test PRIVATE ${ALSA_INCLUDE_DIRS})

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
install(TARGETS kymedia
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES libkymedia.h
        DESTINATION include/kysdk/kysdk-system) 