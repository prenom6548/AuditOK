find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0 gio-2.0 gio-unix-2.0)
include_directories(${GLIB_INCLUDE_DIRS})

pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)
include_directories(${YAML_CPP_INCLUDE_DIRS})

set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Core Sql REQUIRED)

# api
set(CONF2LIB  kyconf2)
set(CONF2LIBSRC api/libkysettings.c api/ksettingsschema.c)

add_library(${CONF2LIB} SHARED ${CONF2LIBSRC})
set_target_properties(${CONF2LIB} PROPERTIES VERSION 3.0.1 SOVERSION 1)
target_link_libraries(${CONF2LIB} ${GLIB_LIBRARIES} ${GIO_LIBRARIES} -lsqlite3)
target_link_options(${CONF2LIB} PRIVATE "-Wl,-z,now")

# sync-config
set(SYNCSYSTEM conf2-sync-system)
set(SYNCSYSTEMSRC sync-config/conf2-sync-system.c sync-config/sync-dbus-server.c sync-config/gdbus/libsyncconfig.c)
add_executable(${SYNCSYSTEM} ${SYNCSYSTEMSRC})
target_link_libraries(${SYNCSYSTEM} ${GLIB_LIBRARIES} ${GIO_LIBRARIES} ${CONF2LIB} kylog)

set(SYNCSESSION conf2-sync-session)
set(SYNCSESSIONSRC sync-config/conf2-sync-session.c sync-config/sync-dbus-server.c sync-config/gdbus/libsyncconfig.c)
add_executable(${SYNCSESSION} ${SYNCSESSIONSRC})
target_link_libraries(${SYNCSESSION} ${GLIB_LIBRARIES} ${GIO_LIBRARIES} ${CONF2LIB} kylog)

# tool
add_executable(kconf2 tools/kconf2-tool.c )
target_link_libraries(kconf2 ${CONF2LIB})

#test
add_executable(kyconf2-test test/kyconf2-test.c)
target_link_libraries(kyconf2-test ${CONF2LIB})

#conf2-compile
set(CONF2COMPILE conf2-compile)
set(CONF2COMPILESRC conf2-compile/main.cpp conf2-compile/sqlitehelper.cpp conf2-compile/utils.cpp conf2-compile/yamlhelper.cpp)
add_executable(${CONF2COMPILE} ${CONF2COMPILESRC})
target_link_libraries(${CONF2COMPILE} Qt5::Core Qt5::Sql ${YAML_CPP_LIBRARIES})

# 包含 GNU 安装目录设置  
include(GNUInstallDirs)

# 设置 CMAKE_INSTALL_LIBDIR 变量  
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv8l")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/arm-linux-gnueabihf/")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/mips64el-linux-gnuabi64/")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/i386-linux-gnu/")
else()
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu/")
endif()

# 安装库文件  
install(TARGETS ${CONF2LIB}
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS ${SYNCSYSTEM}
        DESTINATION /usr/bin)

install(TARGETS ${SYNCSESSION}
        DESTINATION /usr/bin)

install(TARGETS kconf2
        DESTINATION /usr/bin)

install(TARGETS ${CONF2COMPILE}
        DESTINATION /usr/bin)
