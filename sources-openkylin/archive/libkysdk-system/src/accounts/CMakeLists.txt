aux_source_directory(. SOURCESCODE)
include_directories(.)
find_library(GLIBC_LIB glib-2.0)
find_library(DBUS_LIB dbus-1)
find_library(DBUS_GLIB_LIB dbus-glib-1)
find_package(PkgConfig)
add_library(kysdk-accounts SHARED ${SOURCESCODE})
set_target_properties(kysdk-accounts PROPERTIES VERSION 3.0.0 SOVERSION 1)
add_executable(kyaccounts-test test/kyaccounts_test.c)
pkg_check_modules(PKGS REQUIRED polkit-gobject-1)
target_link_libraries(kysdk-accounts kylog polkit-gobject-1 libcjson.so ${GLIBC_LIB} ${DBUS_LIB} ${DBUS_GLIB_LIB})
target_link_libraries(kyaccounts-test kysdk-accounts)
target_link_options(kysdk-accounts PRIVATE "-Wl,-z,now")

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
install(TARGETS kysdk-accounts
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES libkyaccounts.h
        DESTINATION include/kysdk/kysdk-system)

install(FILES org.kylin.kysdk.accounts.policy
    DESTINATION share/polkit-1/actions/)
