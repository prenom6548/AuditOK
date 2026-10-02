cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(kaiming
        DESCRIPTION "kaiming packages"
        HOMEPAGE_URL ""
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 默认构建类型为Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

function(get_linker_version LINKER_PATH OUTPUT_VAR)
    if (NOT LINKER_PATH)
        set(LINKER_PATH "ld")
    endif()

    execute_process(
        COMMAND ${LINKER_PATH} --version
        OUTPUT_VARIABLE LD_VERSION_OUTPUT
        ERROR_VARIABLE LD_VERSION_ERROR
        RESULT_VARIABLE LD_VERSION_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (LD_VERSION_RESULT EQUAL 0)
        string(REGEX MATCH "([0-9]+\\.[0-9]+(\\.[0-9]+)?)" VERSION_NUM "${LD_VERSION_OUTPUT}")
        set(${OUTPUT_VAR} ${VERSION_NUM} PARENT_SCOPE)
    else()
        set(${OUTPUT_VAR} "unknown" PARENT_SCOPE)
    endif()
endfunction()

# 安全编译选项
set(SECURITY_COMPILE_FLAGS "-fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIC -fPIE")
if (NOT CMAKE_CXX_COMPILER_LINKER_VERSION)
    get_linker_version("${CMAKE_LINKER}" CMAKE_CXX_COMPILER_LINKER_VERSION)
endif()
message(STATUS "LINKER:${CMAKE_LINKER}, VERSION:${CMAKE_CXX_COMPILER_LINKER_VERSION}")
# v10sp1中部分静态库构建时没有加参数-fPIE,链接的时候不能加-pie选项; openkylin2.0中ld版本为2.41
if(CMAKE_CXX_COMPILER_LINKER_VERSION VERSION_LESS "2.41")
    set(SECURITY_LINK_FLAGS "-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack")
else()
    set(SECURITY_LINK_FLAGS "-pie -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -ggdb ${SECURITY_COMPILE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -ggdb -frtti ${SECURITY_COMPILE_FLAGS}")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 ${SECURITY_COMPILE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -frtti ${SECURITY_COMPILE_FLAGS}")
endif()

# 设置链接选项
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SECURITY_LINK_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SECURITY_LINK_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SECURITY_LINK_FLAGS}")

# 单元测试编译选项（仅在 Debug 下可用）
option(ENABLE_COVERAGE "Enable coverage flags" OFF)
if(ENABLE_COVERAGE)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Coverage enabled in Debug mode")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
        add_definitions(-DCPPUNIT_TEST)
    else()
        message(WARNING "Coverage can only be enabled in Debug build type")
        set(ENABLE_COVERAGE OFF CACHE BOOL "Enable coverage flags" FORCE)
    endif()
endif()
# 不再默认同时维护单元测试用例
# include(CTest)

# 处理环境变量
if(DEFINED ENV{CFLAGS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} $ENV{CFLAGS}")
endif()

if(DEFINED ENV{CXXFLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS}")
endif()

find_package(PkgConfig REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# 使用 REQUIRED, 如果找不到模块，命令将失败并报错，并停止运行
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO2 REQUIRED gio-2.0)
pkg_check_modules(GIO_UNIX2 REQUIRED gio-unix-2.0)
pkg_check_modules(OSTREE REQUIRED ostree-1)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(DBUSGLIB REQUIRED dbus-glib-1)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(UUID REQUIRED uuid)  #  libuuid
pkg_check_modules(LIBELF REQUIRED libelf)

# 输出 libcurl 的库信息（可选）
message("Libcurl include directories: ${CURL_INCLUDE_DIRS}")
message("Libcurl libraries: ${CURL_LIBRARIES}")

set(LOCALEDIR "/opt/system/resource/locale")
configure_file(config.h.in config.h)

add_compile_options(-Wno-unused-result)

aux_source_directory(app SRC_APP)
aux_source_directory(app/deltas SRC_APP_DELTAS)
aux_source_directory(app/builder SRC_APP_BUILDER)
aux_source_directory(app/install SRC_APP_INSTALL)
aux_source_directory(app/update SRC_APP_UPDATE)
aux_source_directory(app/upgrade SRC_APP_UPGRADE)
aux_source_directory(app/list SRC_APP_LIST)
aux_source_directory(app/info SRC_APP_INFO)
aux_source_directory(app/search SRC_APP_SEARCH)
aux_source_directory(app/repair SRC_APP_REPAIR)
aux_source_directory(app/uninstall SRC_APP_UNINSTALL)
aux_source_directory(app/enter SRC_APP_ENTER)
aux_source_directory(app/run SRC_APP_RUN)
aux_source_directory(app/ps SRC_APP_PS)
aux_source_directory(app/print-meta SRC_APP_PRINTMETA)
aux_source_directory(app/kill SRC_APP_KILL)
aux_source_directory(app/which SRC_APP_WHICH)
aux_source_directory(app/remoteModify SRC_APP_REMOTE)
aux_source_directory(dbus SRC_APP_DBUS)
aux_source_directory(common SRC_COMMON)
aux_source_directory(app/changelog SRC_APP_CHANGELOG)
aux_source_directory(app/compare-versions SRC_APP_COMPAREVERSIONS)

add_library(kaiming-common
    OBJECT
        ${SRC_APP_DELTAS}
        ${SRC_APP_INSTALL}
        ${SRC_APP_ENTER}
        ${SRC_APP_UPDATE}
        ${SRC_APP_UPGRADE}
        ${SRC_APP_LIST}
        ${SRC_APP_SEARCH}
        ${SRC_APP_INFO}
        ${SRC_APP_REPAIR}
        ${SRC_APP_UNINSTALL}
        ${SRC_APP_RUN}
        ${SRC_APP_PS}
        ${SRC_APP_KILL}
        ${SRC_APP_WHICH}
        ${SRC_APP_REMOTE}
        ${SRC_COMMON}
)
set_target_properties(kaiming-common
    PROPERTIES
        POSITION_INDEPENDENT_CODE 1
)

add_executable(km-cli
    ${SRC_APP}
    ${SRC_APP_BUILDER}
    ${SRC_APP_PRINTMETA}
    ${SRC_APP_CHANGELOG}
    ${SRC_APP_COMPAREVERSIONS}
    $<TARGET_OBJECTS:kaiming-common>
)

add_executable(km-run
    common/KMLogger.cpp
    common/KMSubCommand.cpp
    common/KMDynamicCreator.cpp
    common/KMObject.cpp
    common/KMVersion.cpp
    common/KMPermissionConfig
    common/KMOverlay.cpp
    common/KMInfoJson.cpp
    common/KMBuildinOptions.cpp
    common/KMContainer.cpp
    common/KMBrowserUnits.cpp
    common/KMCrun.cpp
    common/KMInstalledAppQuery.cpp
    common/KMJsonHelper.cpp
    common/KMProcessUtils.cpp
    common/KMException.cpp
    common/KMFileUtils.cpp
    common/KMDirUtils.cpp
    common/KMStringUtils.cpp
    common/KMProcessUtils.cpp
    common/KMUnixSocketServer.cpp
    common/KMStorageDir.cpp
    app/main.cpp
    app/KMApplication.cpp
    common/KMCommonUtils.cpp
    ${SRC_APP_ENTER}
    ${SRC_APP_RUN}
    ${SRC_APP_PS}
    ${SRC_APP_KILL}
)

# 增加系统dbus
add_executable(kaiming-system-dbus
    ./dbus/gdbus/KMSystemServerAdaptor.cpp
    $<TARGET_OBJECTS:kaiming-common>
)

# 增加Session dbus
add_executable(kaiming-sessionproxy-dbus
    dbus/gdbus/KMSessionProxy.cpp
    common/KMLogger.cpp
    common/KMCrun.cpp
    common/KMStringUtils.cpp
    common/KMProcessUtils.cpp
    common/KMJsonHelper.cpp
    common/KMException.cpp
    common/KMCommonUtils.cpp
    common/KMBrowserUnits.cpp
    common/KMStorageDir.cpp
    common/KMInfoJson.cpp
)

# 增加SystemAsync dbus
add_executable(kaiming-systemproxy-dbus
    dbus/gdbus/KMSystemProxy.cpp
    common/KMLogger.cpp
    common/KMCrun.cpp
    common/KMStringUtils.cpp
    common/KMProcessUtils.cpp
    common/KMJsonHelper.cpp
    common/KMException.cpp
    common/KMCommonUtils.cpp
    common/KMBrowserUnits.cpp
    common/KMStorageDir.cpp
    common/KMInfoJson.cpp
)

add_dependencies(kaiming-system-dbus build_kmsysteminterface)
add_dependencies(kaiming-sessionproxy-dbus build_kmsessionproxy)
add_dependencies(kaiming-systemproxy-dbus build_kmsystemproxy)

target_include_directories(kaiming-common PUBLIC
    ${GLIB2_INCLUDE_DIRS}
    ${GIO2_INCLUDE_DIRS}
    ${GIO_UNIX2_INCLUDE_DIRS}
    ${OSTREE_INCLUDE_DIRS}
    ${DBUSGLIB_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${UUID_INCLUDE_DIRS}
    ${LIBELF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${nlohmann_json_INCLUDE_DIRS})

target_include_directories(km-cli PUBLIC
    ${GLIB2_INCLUDE_DIRS}
    ${GIO2_INCLUDE_DIRS}
    ${GIO_UNIX2_INCLUDE_DIRS}
    ${OSTREE_INCLUDE_DIRS}
    ${DBUSGLIB_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${LIBELF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(km-run PUBLIC
    ${GLIB2_INCLUDE_DIRS}
    ${GIO2_INCLUDE_DIRS}
    ${GIO_UNIX2_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(kaiming-system-dbus PUBLIC
    ${GLIB2_INCLUDE_DIRS}
    ${GIO2_INCLUDE_DIRS}
    ${GIO_UNIX2_INCLUDE_DIRS}
    ${OSTREE_INCLUDE_DIRS}
    ${DBUSGLIB_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${LIBELF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLIB_INCLUDE_DIRS}
)

target_include_directories(kaiming-sessionproxy-dbus PUBLIC
    ${GIO_UNIX2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLIB_INCLUDE_DIRS}
)

target_include_directories(kaiming-systemproxy-dbus PUBLIC
    ${GIO_UNIX2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLIB_INCLUDE_DIRS}
)

target_link_libraries(kaiming-common PUBLIC
    ${GLIB2_LIBRARIES}
    ${GIO2_LIBRARIES}
    ${GIO_UNIX2_LIBRARIES}
    ${OSTREE_LIBRARIES}
    ${DBUSGLIB_LIBRARIES}
    ${CURL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${UUID_LIBRARIES}  # 链接 libuuid
    Threads::Threads
    stdc++fs
    ${LIBELF_LIBRARIES}
    elf
    dl
    atomic
)

target_link_libraries(km-cli PRIVATE
    ${GLIB2_LIBRARIES}
    ${GIO2_LIBRARIES}
    ${GIO_UNIX2_LIBRARIES}
    ${OSTREE_LIBRARIES}
    ${DBUSGLIB_LIBRARIES}
    ${CURL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${UUID_LIBRARIES}
    Threads::Threads
    -lstdc++fs 
    ${LIBELF_LIBRARIES}
    -lelf
    -ldl
    -latomic
)

target_link_libraries(km-run PRIVATE
    ${GIO2_LIBRARIES}
    ${UUID_LIBRARIES}
    Threads::Threads
    -lstdc++fs 
    -ldl
    -latomic
)

target_link_libraries(kaiming-system-dbus PRIVATE
    ${GLIB2_LIBRARIES}
    ${GIO2_LIBRARIES}
    ${GIO_UNIX2_LIBRARIES}
    ${OSTREE_LIBRARIES}
    ${DBUSGLIB_LIBRARIES}
    ${CURL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${UUID_LIBRARIES}
    Threads::Threads
    -lstdc++fs
    ${LIBELF_LIBRARIES}
    -lelf
    -ldl
    -latomic
    ${CMAKE_BINARY_DIR}/dbus/kmsysteminterface.a
    ${GLIB_LIBRARIES}
)

target_link_libraries(kaiming-sessionproxy-dbus PRIVATE
    ${GIO_UNIX2_LIBRARIES}
    ${CMAKE_BINARY_DIR}/dbus/kmsessionproxy.a
    ${GLIB_LIBRARIES}
    ${UUID_LIBRARIES}
)

target_link_libraries(kaiming-systemproxy-dbus PRIVATE
    ${GIO_UNIX2_LIBRARIES}
    ${CMAKE_BINARY_DIR}/dbus/kmsystemproxy.a
    ${GLIB_LIBRARIES}
    ${UUID_LIBRARIES}
)

# set(ETC_PATH "/opt/system/conf")
set(ETC_PATH "/etc")
set(LIB_PATH "/opt/system/lib")
set(USR_SHARE_PATH "/opt/system/resource")

# 安装二进制到 /opt/kaiming-tools/bin
install( TARGETS km-cli
    RUNTIME DESTINATION  /opt/kaiming-tools/bin
)

install( TARGETS km-run
    RUNTIME DESTINATION  /opt/kaiming-tools/bin
)

# 安装二进制到 /opt/kaiming-tools/bin
install( TARGETS kaiming-system-dbus
    RUNTIME DESTINATION  /opt/kaiming-tools/bin
)

# 安装二进制到 /opt/kaiming-tools/bin
install( TARGETS kaiming-sessionproxy-dbus
    RUNTIME DESTINATION  /opt/kaiming-tools/bin
)

install( TARGETS kaiming-systemproxy-dbus
    RUNTIME DESTINATION  /opt/kaiming-tools/bin
)

# 安装 run 运行所需文件，没有则会开启 dbus 失败
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/
        USE_SOURCE_PERMISSIONS
        DESTINATION ${ETC_PATH}/systemd/system-environment-generators
        FILES_MATCHING PATTERN "60-kaiming"
)

# 安装 system dbus 运行所需文件，没有则会开启 dbus 失败
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/org.kylin.kaiming.conf
        DESTINATION ${ETC_PATH}/dbus-1/system.d
)

# 安装 system dbus 运行所需文件，没有则会开启 dbus 失败
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/org.kylin.kaiming.service
        DESTINATION ${LIB_PATH}/systemd/system
)

# 安装 session dbus 运行所需文件，没有则会开启 dbus 失败
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/km-ses-dbusproxy.service
        DESTINATION ${LIB_PATH}/systemd/user
)

# 安装 session dbus 运行所需文件，没有则会开启 dbus 失败
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/org.kaiming.proxy.session.conf
        DESTINATION ${ETC_PATH}/dbus-1/session.d
        RENAME org.kaiming.proxy.conf
)

# 安装 session dbus 运行所需文件，没有则会开启 dbus 失败
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/org.kaiming.proxy.session.service
        DESTINATION ${USR_SHARE_PATH}/dbus-1/services
        RENAME org.kaiming.proxy.service
)

# 安装 systme dbus 运行所需文件，没有则会开启 dbus 失败
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/org.kaiming.proxy.system.conf
        DESTINATION ${ETC_PATH}/dbus-1/system.d
        RENAME org.kaiming.proxy.conf
)

# 安装 system dbus 运行所需文件，没有则会开启 dbus 失败
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/org.kaiming.proxy.system.service
        DESTINATION ${USR_SHARE_PATH}/dbus-1/system-services
        RENAME org.kaiming.proxy.service
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/org.kylin.kaiming.policy
    DESTINATION ${USR_SHARE_PATH}/polkit-1/actions
)

#安装kaiming-manifests.json
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/kaiming-manifests.json
    DESTINATION /opt/system/resource/update-manifests
)

# 安装 XDG_DATA_DIRS 环境变量设置脚本
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/20kaiming
        DESTINATION ${ETC_PATH}/X11/Xsession.d
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/kaiming.sh
        DESTINATION ${ETC_PATH}/profile.d
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/60-kaimingenv.conf
        DESTINATION ${ETC_PATH}/ostree/ostree.conf.d
)

# 安装 kaiming 命令补全脚本
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/shells/kaiming_completion.sh
        DESTINATION ${ETC_PATH}/bash_completion.d
)

# 安装 kaiming-custom.xml
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/mime/packages/kaiming-custom.xml
        DESTINATION ${USR_SHARE_PATH}/mime/packages/
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/hicolor/scalable/mimetypes/application-x-kaiming.svg
        DESTINATION ${USR_SHARE_PATH}/icons/hicolor/scalable/mimetypes/
)

# 安装 kaiming 版本检查脚本
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/shells/kaiming_version.sh
        DESTINATION /opt/kaiming-tools/bin
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/shells/install_files.sh
        DESTINATION /opt/kaiming-tools/bin
)

#  安装wayland兼容x11脚本
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/shells/x11-ext.sh
        PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
        DESTINATION /opt/kaiming-tools/bin
)

#  安装pulse兼容配置
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/shells/pulse-config
        DESTINATION /opt/kaiming-tools/config
)

#  安装 crun 分离检查脚本
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/shells/start.sh
        PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
        DESTINATION /opt/kaiming-tools/bin
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/shells/kaiming
        PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
        DESTINATION /opt/kaiming-tools/bin
)

# 安装dbus客户端
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/shells/dbus-cli.sh
        PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
        DESTINATION /opt/kaiming-tools/bin
)

# 安装 trigger 脚本，并保留权限
# References: https://cmake.org/cmake/help/v2.8.10/cmake.html#command%3ainstall
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shells
        USE_SOURCE_PERMISSIONS
        DESTINATION /opt/kaiming-tools/share/kaiming
        FILES_MATCHING PATTERN "*.trigger"
)

# 设置 PO 文件和目标 MO 文件的路径
# set(POT_FILE ${CMAKE_SOURCE_DIR}/po/kaiming.pot)
set(PO_FILE ${CMAKE_SOURCE_DIR}/po/kaiming.po)
set(MO_FILE ${CMAKE_SOURCE_DIR}/po/kaiming.mo)  # 在构建目录下生成 .mo 文件

# 添加自定义命令，将 .po 文件转换为 .mo 文件
add_custom_command(
    OUTPUT ${MO_FILE}  # 输出的 .mo 文件
    COMMAND msgfmt --output-file ${MO_FILE} ${PO_FILE}  # msgfmt 转换命令
    DEPENDS ${PO_FILE}  # 声明 .mo 文件依赖于 .po 文件
    COMMENT "Converting PO to MO file"  # 转换注释
    VERBATIM  # 保证命令中的空格问题
)

# 创建自定义目标，确保在构建时生成 .mo 文件
add_custom_target(
    po_to_mo ALL  # 将此目标设置为构建时自动执行
    DEPENDS ${MO_FILE}  # 依赖于生成的 .mo 文件
)

# 安装 .mo 文件到指定的系统目录
install(
    FILES ${MO_FILE}  # 安装生成的 .mo 文件
    DESTINATION ${USR_SHARE_PATH}/locale/zh_CN/LC_MESSAGES  # 安装路径

)

# 本地安装时
# install(
#     CODE "execute_process(COMMAND sh ${CMAKE_SOURCE_DIR}/shells/install_files.sh)"
# )
#删除生成的mo文件
install(
    CODE "file(REMOVE \"${MO_FILE}\")"
)

add_subdirectory(dbus)
add_subdirectory(ok)
add_subdirectory(dbusproxy)
add_subdirectory(tools)
#add_subdirectory(test/cpptest)
# 根据 BUILD_TESTING 选项决定是否包含测试目录
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
