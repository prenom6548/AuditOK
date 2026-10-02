cmake_minimum_required(VERSION 3.16)

project(kyimageproc)
aux_source_directory(. SOURCESCODE)
include_directories(.)

set(CMAKE_CXX_STANDARD 11)

add_library(kyimageproc SHARED ${SOURCESCODE})
set_target_properties(kyimageproc PROPERTIES VERSION 3.0.0 SOVERSION 1)

find_package(PkgConfig REQUIRED)
pkg_check_modules(KYSDKLOG kysdk-log)
find_package( OpenCV REQUIRED )
target_include_directories(kyimageproc PRIVATE ${KYSDKLOG_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_directories(kyimageproc PRIVATE ${KYSDKLOG_LIBRARY_DIRS} ${OpenCV_LIBRARY_DIRS})
target_link_libraries(kyimageproc PRIVATE ${KYSDKLOG_LIBRARIES} dl ${OpenCV_LIBRARIES})
target_link_options(kyimageproc PRIVATE "-Wl,-z,now")

add_executable(kyimage-test test/kyimage-test.c)
target_link_libraries(kyimage-test kyimageproc)

set(PREFIX "")
set(LIBS_INSTALL_DIR "${PREFIX}/usr/lib/kysdk/kysdk-system")
set(HEADERS_INSTALL_DIR "${PREFIX}/usr/include/kysdk/kysdk-system/")

# install(TARGETS kyimageproc LIBRARY DESTINATION ${LIBS_INSTALL_DIR})

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
install(TARGETS kyimageproc
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES libkyimageproc.h DESTINATION ${HEADERS_INSTALL_DIR})
