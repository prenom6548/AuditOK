cmake_minimum_required(VERSION 3.5)

project(kylin-burner)

#import burn engine lib
add_subdirectory(lib/ky-burn-engine)
add_subdirectory(lib/ky-disc-info)
add_subdirectory(lib/ky-udf-image)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(KAIMING_APP_ID "$ENV{APP_ID}")
set(XDG_SESSION_TYPE "$ENV{XDG_SESSION_TYPE}")
set(DESKTOP_SESSION "$ENV{DESKTOP_SESSION}")
set(UkuiSessionType 0)

message(STATUS "DESKTOP_SESSION is [${DESKTOP_SESSION}]")
message(STATUS "XDG_SESSION_TYPE is [${XDG_SESSION_TYPE}]")

# 判断是否为开明构建，创建编译期间全局宏定义: KAIMING_BUILD
if(NOT KAIMING_APP_ID STREQUAL "")
    set(KaiMingBuild true)
    message(STATUS "KAIMING_APP_ID is : [${KAIMING_APP_ID}], set KaiMingBuild = true")
else()
    set(KaiMingBuild false)
    message(STATUS "KAIMING_APP_ID is null, set KaiMingBuild = false")
endif()
add_compile_definitions(KAIMING_BUILD=${KaiMingBuild})


# 判断显示协议，创建全局宏定义: UKUI_SESSION_TYPE
# 显示协议：x11,                                           UKUI_SESSION_TYPE = 0
# 显示协议：wayland，kylin-wlcom, 使用ukuiwaylandhelper.h,  UKUI_SESSION_TYPE = 1
#                               使用ukuistylehelper.h,    UKUI_SESSION_TYPE = 2
function(set_ukuiSessionType)
    if(KaiMingBuild)
        set(UkuiSessionType 2 PARENT_SCOPE)
    else()
        set(UkuiSessionType 2 PARENT_SCOPE)
    endif()
endfunction()

# 在开明编译中获取环境变量 XDG_SESSION_TYPE 的值为空
if(XDG_SESSION_TYPE STREQUAL "")
    if(DESKTOP_SESSION STREQUAL "")
        message(STATUS "XDG_SESSION_TYPE 与 DESKTOP_SESSION 均为空，平台编包")
        set(UkuiSessionType 2)
    elseif(DESKTOP_SESSION STREQUAL "kylin-wlcom")
        set_ukuiSessionType()
    else()
        set(UkuiSessionType 0)
    endif()
elseif(XDG_SESSION_TYPE STREQUAL "wayland")
    set_ukuiSessionType()
else()
    set(UkuiSessionType 0)
endif()
message(STATUS "XDG_SESSION_TYPE is [${XDG_SESSION_TYPE}], DESKTOP_SESSION is [${DESKTOP_SESSION}], KaiMingBuild is : [${KaiMingBuild}], set UkuiSessionType is : ${UkuiSessionType}")
add_compile_definitions(UKUI_SESSION_TYPE=${UkuiSessionType})

if(KaiMingBuild)
    set(CMAKE_C_FLAGS "$ENV{CFLAGS} -O0 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIC -fPIE -pie -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack")
    set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O0 -std=c++17 -frtti -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIC -fPIE -pie -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie")
else()
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

SET(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -g -O0 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O0 -Wall")

# 从changelog中提取版本号
execute_process(
    COMMAND dpkg-parsechangelog -S Version
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 显示提取的版本号
message(STATUS "项目版本号: ${PROJECT_VERSION}")

# 配置版本头文件
configure_file(
    "${CMAKE_SOURCE_DIR}/src/version.h.in"
    "${CMAKE_SOURCE_DIR}/src/version.h"
)


find_package(Qt5 COMPONENTS Widgets DBus X11Extras Network REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5WindowSystem)

pkg_check_modules(KYSDKQTWIDGETS_PKG kysdk-qtwidgets)
pkg_check_modules(GLIB2 glib-2.0)
pkg_check_modules(KYSDKSYSINFO_PKG kysdk-sysinfo)
pkg_check_modules(KYSDKALM_PKG kysdk-alm)
pkg_check_modules(KYSDKAPPLICATION_PKG kysdk-application)
pkg_check_modules(KYSDKBASE_PKG kysdk-base)
pkg_check_modules(KYSDKSOUNDEFFECTS_PKG kysdk-soundeffects)
pkg_check_modules(KYSDKWAYLANDHELPER_PKG kysdk-waylandhelper)
pkg_check_modules(DATACOLLECT kysdk-datacollect)


include(${CMAKE_SOURCE_DIR}/ProjectGroup.cmake)


#安装二进制程序时,保留编译时的runtime path
set_target_properties(kylin-burner PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

#自定义对象并跟随all_target一起生成
add_custom_target(MO-FILE
    ALL
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/zh_CN.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/zh_CN.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/bo_CN.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/bo_CN.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/kk.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/kk.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/de_DE.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/de_DE.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/es_ES.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/es_ES.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/fr_FR.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/fr_FR.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/ky.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/ky.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/ug.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/ug.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/mn_MN.po"
	    "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/mn_MN.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/zh_HK.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/zh_HK.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/vi.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/vi.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/th.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/th.mo"
    COMMAND "msgfmt" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/ms.po"
            "-o" ${CMAKE_CURRENT_SOURCE_DIR}"/src/po/ms.mo"

)

target_include_directories(kylin-burner PRIVATE
    ${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${KYSDKSYSINFO_PKG_INCLUDE_DIRS}
    ${KYSDKAPPLICATION_PKG_INCLUDE_DIRS}
    ${KYSDKBASE_PKG_INCLUDE_DIRS}
    ${KYSDKALM_PKG_INCLUDE_DIRS}
    ${KYSDKSOUNDEFFECTS_PKG_INCLUDE_DIRS}
    ${DATACOLLECT_INCLUDE_DIRS}
    ${KYSDKWAYLANDHELPER_PKG_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/lib/burner-utils-lib/include
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/src
)

target_link_directories(kylin-burner PRIVATE
    ${KYSDKQTWIDGETS_PKG_LIBRARY_DIRS}
    ${KYSDKSYSINFO_PKG_LIBRARY_DIRS}
    ${KYSDKAPPLICATION_PKG_LIBRARY_DIRS}
    ${KYSDKBASE_PKG_LIBRARY_DIRS}
    ${KYSDKALM_PKG_LIBRARY_DIRS}
    ${KYSDKSOUNDEFFECTS_PKG_LIBRARY_DIRS}
    ${DATACOLLECT_LIBRARY_DIRS}
    ${KYSDKWAYLANDHELPER_PKG_LIBRARY_DIRS}
)

target_link_libraries(kylin-burner PRIVATE
    Qt5::Widgets
    Qt5::DBus
    Qt5::X11Extras
    Qt5::Network
    KF5::WindowSystem
    X11
    pthread
    gsettings-qt
    ukui-log4qt
    ${GLIB2_LIBRARIES}
    ${KYSDKQTWIDGETS_PKG_LIBRARIES}
    ${KYSDKAPPLICATION_PKG_LIBRARIES}
    ${KYSDKBASE_PKG_LIBRARIES}
    ${KYSDKALM_PKG_LIBRARIES}
    ${KYSDKSYSINFO_PKG_LIBRARIES}
    ${KYSDKSOUNDEFFECTS_PKG_LIBRARIES}
    ${DATACOLLECT_LIBRARIES}
    ${KYSDKWAYLANDHELPER_PKG_LIBRARIES}
    ky-burn-engine
    ky-disc-info
    ky-udf-tools
)

if(UkuiSessionType EQUAL 2)
    message(STATUS "UkuiSesionType is : [${UkuiSesionType}], add lib: kysdk-ukuiwindowhelper")
    find_package(kysdk-ukuiwindowhelper)
    pkg_check_modules(KYSDKUKUIWINDOWHLPER_PKG kysdk-ukuiwindowhelper)
    target_include_directories(kylin-burner PRIVATE
        ${KYSDKUKUIWINDOWHLPER_PKG_INCLUDE_DIRS}
    )
    target_link_directories(kylin-burner PRIVATE
        ${KYSDKUKUIWINDOWHLPER_PKG_LIBRARY_DIRS}
    )

    target_link_libraries(kylin-burner PRIVATE
        ${KYSDKUKUIWINDOWHLPER_PKG_LIBRARIES}
    )
endif()

if(KaiMingBuild)
    install(TARGETS kylin-burner DESTINATION /opt/apps/top.openkylin.kylin-burner/files/bin/)
    install(DIRECTORY data/kylin-burner DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/kylin-user-guide/data/guide/)
    install(PROGRAMS kylin-burner-kaiming.desktop RENAME kylin-burner.desktop DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/applications)
    install(FILES src/po/zh_CN.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/zh_CN/LC_MESSAGES/)
    install(FILES src/po/bo_CN.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/bo_CN/LC_MESSAGES/)
    install(FILES src/po/kk.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/kk/LC_MESSAGES/)
    install(FILES src/po/de_DE.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/de_DE/LC_MESSAGES/)
    install(FILES src/po/es_ES.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/es_ES/LC_MESSAGES/)
    install(FILES src/po/fr_FR.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/fr_FR/LC_MESSAGES/)
    install(FILES src/po/ky.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/ky/LC_MESSAGES/)
    install(FILES src/po/ug.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/ug/LC_MESSAGES/)
    install(FILES src/po/mn_MN.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/mn_MN/LC_MESSAGES/)
    install(FILES src/po/zh_HK.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/zh_HK/LC_MESSAGES/)
    install(FILES src/po/vi.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/vi/LC_MESSAGES/)
    install(FILES src/po/th.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/th/LC_MESSAGES/)
    install(FILES src/po/ms.mo RENAME kylin-burner.mo DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/locale/ms/LC_MESSAGES/)
    install(FILES gsettings/org.ukui.kylinburner.gschema.xml DESTINATION /opt/apps/top.openkylin.kylin-burner/files/share/glib-2.0/schemas/)
else()		
    install(TARGETS kylin-burner ${INSTALL_TARGETS_DEFAULT_ARGS})
    install(DIRECTORY data/kylin-burner DESTINATION /usr/share/kylin-user-guide/data/guide/)
    install(PROGRAMS kylin-burner.desktop DESTINATION /usr/share/applications)
    install(FILES src/po/zh_CN.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/zh_CN/LC_MESSAGES/)
    install(FILES src/po/bo_CN.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/bo_CN/LC_MESSAGES/)
    install(FILES src/po/kk.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/kk/LC_MESSAGES/)
    install(FILES src/po/de_DE.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/de_DE/LC_MESSAGES/)
    install(FILES src/po/es_ES.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/es_ES/LC_MESSAGES/)
    install(FILES src/po/fr_FR.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/fr_FR/LC_MESSAGES/)
    install(FILES src/po/ky.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/ky/LC_MESSAGES/)
    install(FILES src/po/ug.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/ug/LC_MESSAGES/)
    install(FILES src/po/mn_MN.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/mn_MN/LC_MESSAGES/)
    install(FILES src/po/zh_HK.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/zh_HK/LC_MESSAGES/)
    install(FILES src/po/vi.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/vi/LC_MESSAGES/)
    install(FILES src/po/th.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/th/LC_MESSAGES/)
    install(FILES src/po/ms.mo RENAME kylin-burner.mo DESTINATION /usr/share/locale/ms/LC_MESSAGES/)
    install(FILES gsettings/org.ukui.kylinburner.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
endif()
