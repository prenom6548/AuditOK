set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

pkg_check_modules(UDEV REQUIRED libudev)
find_library(LIBUDEV_LIBRARY NAMES udev)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/control
    ${CMAKE_SOURCE_DIR}/base
    ${CMAKE_SOURCE_DIR}/utils)

set (SRC_Sources
    src/main.cpp
    src/processmanager.cpp
    src/cgroupmanager.cpp
    src/systemdunitmanager.cpp
    src/resourcelimit.cpp
    src/efficiencymode.cpp
    src/cpuraplenergymeter.cpp
    src/cgroupv2releasenotification.cpp
    src/psrstatemanagerinterface.cpp
    src/vrrstatemanagerinterface.cpp
    src/refreshratemanager.cpp
    src/inputeventlistener.cpp
    ${CMAKE_SOURCE_DIR}/control/systemddbusinterface.cpp
    ${CMAKE_SOURCE_DIR}/utils/processinfohelper.cpp
    ${CMAKE_SOURCE_DIR}/utils/misc.cpp
)

qt5_add_dbus_adaptor(SRC_Sources
    configs/com.kylin.ProcessManagerDaemon.xml
    src/processmanager.h
    ProcessManager
    processmanagerservice
    ProcessManagerService)

qt5_add_dbus_adaptor(SRC_Sources
    configs/com.kylin.ProcessManagerDaemon.ResourceLimit.xml
    src/resourcelimit.h
    ResourceLimit
    resourcelimitservice
    ResourceLimitService)

qt5_add_dbus_adaptor(SRC_Sources
    configs/com.kylin.ProcessManagerDaemon.CgroupControl.xml
    src/cgroupmanager.h
    CGroupManager
    cgroupmanagerservice
    CGroupManagerService)


qt5_add_dbus_adaptor(SRC_Sources
    configs/com.kylin.ProcessManagerDaemon.EfficiencyMode.xml
    src/efficiencymode.h
    EfficiencyMode
    efficiencymodeservice
    EfficiencyModeService)

add_executable(
    kylin-process-manager-daemon
    ${BASE_FILE}
    ${SRC_Sources}
    src/resourcewatcher.cpp src/resourcewatcher.h
    src/resource.cpp src/resource.h
    src/pressurewatcher.cpp src/pressurewatcher.h
    src/memorywatcher.cpp src/memorywatcher.h
    src/systemresourcemanager.h src/systemresourcemanager.cpp
)

target_link_libraries(kylin-process-manager-daemon
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    ${LIBPROC2_LIBRARIES}
    cgroup
    jsoncpp
    pthread
    ${LIBUDEV_LIBRARY}
)

install(TARGETS kylin-process-manager-daemon DESTINATION /usr/bin)
install(FILES configs/kylin-process-manager-thawer DESTINATION /usr/bin)
install(FILES configs/com.kylin.ProcessManagerDaemon.yaml DESTINATION /etc/kylin-config/basic)
install(FILES configs/com.kylin.ProcessManagerDaemon.conf DESTINATION /usr/share/dbus-1/system.d)
install(FILES configs/com.kylin.ProcessManagerDaemon.service DESTINATION /usr/share/dbus-1/system-services)
install(FILES configs/kylin-process-manager-daemon.service DESTINATION /lib/systemd/system)
install(FILES configs/kylin-process-manager-thawer.service DESTINATION /lib/systemd/system)
install(FILES configs/kylin-process-manager-cleaner DESTINATION /usr/bin)

set(C_SRC src/cpulimit_fork.c)
add_executable(cpulimit_fork ${C_SRC})

install(TARGETS cpulimit_fork DESTINATION /usr/bin)
