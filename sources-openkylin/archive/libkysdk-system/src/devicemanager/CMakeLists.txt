find_package(PkgConfig REQUIRED)
pkg_check_modules(UDEV REQUIRED libudev)
include_directories(${UDEV_INCLUDE_DIRS})

add_library(kydevicemanager SHARED libkydevicemanager.c)
set_target_properties(kydevicemanager PROPERTIES VERSION 3.0.0 SOVERSION 1)
target_link_libraries(kydevicemanager kylog ${UDEV_LIBRARIES})

add_executable(kydevicemanager-test test/kydevicemanager-test.c)
message(${LIBRARY_OUTPUT_PATH})
target_link_directories(kydevicemanager-test PRIVATE ${LIBRARY_OUTPUT_PATH})
target_link_libraries(kydevicemanager-test kydevicemanager)

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
install(TARGETS kydevicemanager
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES libkydevicemanager.h
        DESTINATION include/kysdk/kysdk-system)