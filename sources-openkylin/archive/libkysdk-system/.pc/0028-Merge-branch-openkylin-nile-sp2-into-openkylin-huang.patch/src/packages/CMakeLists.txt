aux_source_directory(. SOURCESCODE)
include_directories(.)
add_library(kypackage SHARED ${SOURCESCODE})
set_target_properties(kypackage PROPERTIES VERSION 3.0.0 SOVERSION 1)
find_library(DBUS_LIB dbus-1)
find_library(GLIBC_LIB glib-2.0)

pkg_check_modules(KYSDKCONF2 kysdk-conf2)
target_include_directories(kypackage PRIVATE ${KYSDKCONF2_INCLUDE_DIRS})
target_link_directories(kypackage PRIVATE ${KYSDKCONF2_LIBRARY_DIRS})
target_link_libraries(kypackage libcjson.so kylog ${DBUS_LIB} ${KYSDKCONF2_LIBRARIES} ${DBUS_GLIB_LIB})
add_executable(kypackage-test test/kypackage-test.c)
target_link_libraries(kypackage-test kypackage)
target_link_options(kypackage PRIVATE "-Wl,-z,now")

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
install(TARGETS kypackage
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
# install(TARGETS kypackage
#         DESTINATION lib/kysdk/kysdk-system)

install(FILES libkypackages.h
        DESTINATION include/kysdk/kysdk-system)
