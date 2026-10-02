include(FindPkgConfig)
pkg_check_modules(ZIP REQUIRED libzip)
include_directories(${ZIP_INCLUDE_DIRS})

# pkg_check_modules(GZIP REQUIRED glib-2.0 libnl-3.0)
# include_directories(${GLIB_INCLUDE_DIRS})

pkg_check_modules(BZIP2 REQUIRED zlib)
include_directories(${BZIP2_INCLUDE_DIRS})

pkg_check_modules(XZ REQUIRED liblzma)
include_directories(${XZ_INCLUDE_DIRS})

pkg_check_modules(LZ4 REQUIRED liblz4)
include_directories(${LZ4_INCLUDE_DIRS})

pkg_check_modules(ZSTD REQUIRED libzstd)
include_directories(${ZSTD_INCLUDE_DIRS})

pkg_check_modules(Z7 REQUIRED libarchive)
include_directories(${Z7_INCLUDE_DIRS})

add_library(kystorage SHARED libkystorage.c)
set_target_properties(kystorage PROPERTIES VERSION 3.0.0 SOVERSION 1)
target_link_libraries(kystorage kylog kypackage magic ${ZIP_LIBRARIES} ${BZIP2_LIBRARIES} ${XZ_LIBRARIES} ${LZ4_LIBRARIES} ${ZSTD_LIBRARIES} ${Z7_LIBRARIES})
target_link_options(kystorage PRIVATE "-Wl,-z,now")

add_executable(kystorage-test test/kystorage-test.c)
target_link_libraries(kystorage-test kystorage)

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
install(TARGETS kystorage
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES libkystorage.h
        DESTINATION include/kysdk/kysdk-system)