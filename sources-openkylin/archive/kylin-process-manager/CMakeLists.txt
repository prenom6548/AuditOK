cmake_minimum_required(VERSION 3.14)

project(kylin-process-manager LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DQT_NO_KEYWORDS)

find_package(PkgConfig REQUIRED)

find_package(KF5KIO)

find_package(KF5WindowSystem)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Concurrent DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Concurrent DBus)

# 查找 wayland-scanner 工具
find_program(wayland_scanner wayland-scanner)
if(NOT wayland_scanner)
        message(FATAL_ERROR "wayland-scanner not found.")
endif()

pkg_check_modules(GSETTINGQT REQUIRED gsettings-qt)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(DATACOLLECT kysdk-datacollect)
pkg_check_modules(LIBPROC2 REQUIRED libproc2)
pkg_check_modules(wayland_client REQUIRED wayland-client)

file(GLOB_RECURSE BASE_FILE CMAKE_CONFIGURE_DEPENDS
    utils/misc.h
    utils/misc.cpp
    control/systemddbusinterface.h
    control/systemddbusinterface.cpp)

if (ENABLE_TEST)
    # enable_testing()
    # add_subdirectory(autotests)
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)

add_subdirectory(daemon)
add_subdirectory(daemon1)

include_directories(${GIO_INCLUDE_DIRS})
include_directories(base utils control)
include_directories(core core/monitor core/data core/state core/scene core/policy)

# 定义服务器协议文件列表
set(server_protocols
        ${CMAKE_SOURCE_DIR}/protocols/dpms.xml
        ${CMAKE_SOURCE_DIR}/protocols/wlr-output-management-unstable-v1.xml
)

# 初始化源文件和头文件列表
set(protos_src "")
set(server_protos_headers "")
set(client_protos_headers "")

# 遍历服务器协议文件
foreach(xml ${server_protocols})
        # 获取协议文件名（不包含扩展名）
        get_filename_component(basename ${xml} NAME_WE)

        # 生成 .c 文件
        set(c_file ${basename}.c)
        add_custom_command(
                OUTPUT ${c_file}
                COMMAND ${wayland_scanner} private-code ${xml} ${c_file}
                DEPENDS ${xml}
                COMMENT "Generating c code from xml"
                )
        list(APPEND protos_src ${c_file})

        # 生成客户端头文件
        set(client_header ${basename}_client.h)
        add_custom_command(
                OUTPUT ${client_header}
                COMMAND ${wayland_scanner} client-header ${xml} ${client_header}
                DEPENDS ${xml}
                COMMENT "Generating client header from xml"
                )
        list(APPEND client_protos_headers ${client_header})
endforeach()

# 添加生成头文件的目录到搜索路径
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#message(WARNING ${client_header})
#message(WARNING ${CMAKE_CURRENT_BINARY_DIR}/${protos_src})

# 基础数据结构
set(BASE_SOURCE
    base/appinfo.cpp
    base/profileattribute.cpp
    base/schedpolicy.cpp
    base/timewheel.cpp
)

# 参数调节
set(CONTROL_SOURCE
    control/cgroupcontrolinterface.cpp
    control/daemon1dbusinterface.cpp
    control/daemondbusinterface.cpp
    control/datacollector.cpp
    control/displayeffectdbusinterface.cpp
    control/displayeffectmanager.cpp
    control/efficiencymodeinterface.cpp
    control/eventbus.h
    control/groupmanagementunit.cpp
    control/login1dbusinterface.cpp
    control/multimediacontroller.cpp
    control/notifydbusinterface.cpp
    control/priorityschedulingmanager.cpp
    control/profileaction.cpp
    control/resourcecontroller.cpp
    control/resourcemanagerinterface.cpp
    control/systemddbusinterface.cpp
    control/systemnotifiessender.cpp
)

# 工具
set(UTILS_SOURCE
    utils/appinfohelper.cpp
    utils/devicestatehelper.cpp
    utils/jsonhelper.cpp
    utils/misc.cpp
    utils/processinfohelper.cpp
    utils/systemstatehelper.cpp
)

set (WAYLAND_SOURCE
    ${CMAKE_CURRENT_BINARY_DIR}/${protos_src}
    ${CMAKE_CURRENT_BINARY_DIR}/${client_protos_headers}
)

# 核心功能
set(CORE_MONITOR core/monitor)  # 监控
set(CORE_DATA core/data)        # 数据
set(CORE_STATE core/state)      # 状态
set(CORE_SCENE core/scene)      # 场景
set(CORE_POLICY core/policy)    # 策略

set(CORE_SOURCE
    # 监控
    ${CORE_MONITOR}/batteryremainingtimeestimator/batteryremainingtimeestimator.cpp
    ${CORE_MONITOR}/consumptionmonitor/processgroupconsumptionmonitor.cpp
    ${CORE_MONITOR}/energymeter/cpuenergymeter.cpp
    ${CORE_MONITOR}/systemenery/upowersystemenery.cpp
    ${CORE_MONITOR}/eventwatcher.cpp
    ${CORE_MONITOR}/exceptiongroupprocesswatcher.cpp
    ${CORE_MONITOR}/waylanddisplay.cpp

    # 数据
    ${CORE_DATA}/selection/selectionchecker.cpp
    ${CORE_DATA}/appinfomanager.cpp
    ${CORE_DATA}/configmanager.cpp
    ${CORE_DATA}/datacontext.cpp
    ${CORE_DATA}/desktopfilemanager.cpp
    ${CORE_DATA}/devicestate.cpp
    ${CORE_DATA}/groupmanager.cpp
    ${CORE_DATA}/processinfomanager.cpp
    ${CORE_DATA}/systemstatemanager.cpp

    # 状态
    ${CORE_STATE}/statemanager.cpp

    # 场景
    ${CORE_SCENE}/scenemanager.cpp
    ${CORE_SCENE}/sceneruleparser.cpp

    # 策略
    ${CORE_POLICY}/policyruleparser.cpp
    ${CORE_POLICY}/schedpolicymanager.cpp

    core/appwhitelist.cpp
    core/applauncher.cpp
    core/appchooser.cpp
    core/applaunchmanager.cpp
)

qt5_add_dbus_adaptor(DBUS_SOURCE
    configs/com.kylin.ProcessManager.xml
    processmanager.h
    ProcessManager
    processmanagerservice
    ProcessManagerService)

qt5_add_dbus_adaptor(DBUS_SOURCE
    configs/com.kylin.ProcessManager.AppWhitelist.xml
    appwhitelist.h
    AppWhitelist
    appwhitelistservice
    AppWhitelistService)

qt5_add_dbus_adaptor(DBUS_SOURCE
    configs/com.kylin.ProcessManager.AppLauncher.xml
    applaunchmanager.h
    AppLaunchManager
    applaunchmanagerservice
    AppLaunchManagerService)

set(TS_FILES
    ./translations/zh_CN.ts)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${REQUIRED_LIBS} LinguistTools REQUIRED)
qt5_create_translation(QM_FILES ${TS_FILES} ${CORE_SOURCE} ${UTILS_SOURCE} ${CONTROL_SOURCE} ${BASE_SOURCE} ${DBUS_SOURCE})

add_custom_command(
    OUTPUT ukui-startup-v1.c
    COMMAND wayland-scanner client-header ${CMAKE_CURRENT_SOURCE_DIR}/protocols/ukui-startup-v1.xml ${CMAKE_CURRENT_BINARY_DIR}/ukui-startup-v1.h
    COMMAND wayland-scanner private-code ${CMAKE_CURRENT_SOURCE_DIR}/protocols/ukui-startup-v1.xml ${CMAKE_CURRENT_BINARY_DIR}/ukui-startup-v1.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/protocols/ukui-startup-v1.xml
)

add_custom_command(
    OUTPUT ukui-startup-v2.c
    COMMAND wayland-scanner client-header ${CMAKE_CURRENT_SOURCE_DIR}/protocols/ukui-startup-v2.xml ${CMAKE_CURRENT_BINARY_DIR}/ukui-startup-v2.h
    COMMAND wayland-scanner private-code ${CMAKE_CURRENT_SOURCE_DIR}/protocols/ukui-startup-v2.xml ${CMAKE_CURRENT_BINARY_DIR}/ukui-startup-v2.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/protocols/ukui-startup-v2.xml
)

add_custom_command(
    OUTPUT ukui-window-management.c
    COMMAND wayland-scanner client-header ${CMAKE_CURRENT_SOURCE_DIR}/protocols/ukui-window-management.xml ${CMAKE_CURRENT_BINARY_DIR}/ukui-window-management.h
    COMMAND wayland-scanner private-code ${CMAKE_CURRENT_SOURCE_DIR}/protocols/ukui-window-management.xml ${CMAKE_CURRENT_BINARY_DIR}/ukui-window-management.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/protocols/ukui-window-management.xml
)

add_custom_command(
    OUTPUT plasma-window-management.c
    COMMAND wayland-scanner client-header ${CMAKE_CURRENT_SOURCE_DIR}/protocols/plasma-window-management.xml ${CMAKE_CURRENT_BINARY_DIR}/plasma-window-management.h
    COMMAND wayland-scanner private-code ${CMAKE_CURRENT_SOURCE_DIR}/protocols/plasma-window-management.xml ${CMAKE_CURRENT_BINARY_DIR}/plasma-window-management.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/protocols/plasma-window-management.xml
)

set(WAYLAND_PROTOCOLS_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/ukui-startup-v1.c
    ${CMAKE_CURRENT_BINARY_DIR}/ukui-startup-v2.c
    ${CMAKE_CURRENT_BINARY_DIR}/ukui-window-management.c
    ${CMAKE_CURRENT_BINARY_DIR}/plasma-window-management.c
)

add_executable(
    kylin-process-manager
    processmanager.h
    processmanager.cpp
    log-utils.h
    log-utils.cpp
    signalhandler.h
    signalhandler.cpp
    main.cpp
    ${DBUS_SOURCE}
    ${BASE_SOURCE}
    ${UTILS_SOURCE}
    ${CONTROL_SOURCE}
    ${CORE_SOURCE}
    ${TS_FILES}
    ${QM_FILES}
    ${WAYLAND_PROTOCOLS_SRCS}
    ${WAYLAND_SOURCE}
)

target_include_directories(kylin-process-manager PRIVATE ${LIBPROC2_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${DATACOLLECT_INCLUDE_DIRS})
target_include_directories(kylin-process-manager PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_directories(${PROJECT_NAME} PRIVATE ${DATACOLLECT_LIBRARY_DIRS})

target_link_libraries(
    kylin-process-manager
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::DBus
    KF5::KIOCore
    KF5::WindowSystem
    ${GSETTINGQT_LIBRARIES}
    ${DATACOLLECT_LIBRARIES}
    ${GIO_LIBRARIES}
    jsoncpp
    pthread
    ${LIBPROC2_LIBRARIES}
    X11
    Xext
    wayland-client
    ${wayland_client_LIBRARIES}
)

install(TARGETS
    kylin-process-manager
    DESTINATION /usr/bin)
install(FILES
    configs/org.ukui.ProcessManager.gschema.xml
    configs/org.ukui.ProcessManager.DefaultGroupApps.gschema.xml
    configs/org.ukui.ProcessManager.SoftFreezeMode.gschema.xml
    DESTINATION /usr/share/glib-2.0/schemas)

install(FILES
    configs/40-kylin-process-manager.conf
    DESTINATION /etc/rsyslog.d/)

install(FILES
    configs/kylin-process-manager
    DESTINATION /etc/logrotate.d/)

install(FILES
    configs/task-profiles-v1.json
    configs/task-profiles-v2.json
    configs/kylin-process-manager.json
    configs/scene-rules.json
    configs/policy-rules.json
    configs/resource-sched-configs.json
    configs/efficiency-mode-configs.json
    DESTINATION /usr/share/kylin-process-manager/)
install(FILES
    configs/com.kylin.ProcessManager.service
    DESTINATION /usr/share/dbus-1/services)
install(FILES ${QM_FILES}
    DESTINATION /usr/share/kylin-process-manager/)
install(FILES
    configs/kylin-process-manager.service
    DESTINATION /usr/lib/systemd/user)
file(GLOB POLICY_RULESD_JSON_FILES "configs/policy_rules.d/*.json")
install(FILES ${POLICY_RULESD_JSON_FILES}
    DESTINATION /usr/share/kylin-process-manager/policy_rules.d/)
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git/hooks/pre-commit)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pre-commit
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/.git/hooks
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
endif()