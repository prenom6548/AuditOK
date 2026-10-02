project(kylin-os-manager-daemon)

set(SERVICE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(DRIVER_MANAGER_SERVICE_DIR ${SERVICE_DIR}/driver-manager-service/)
set(CMAKE_AUTOMOC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core DBus Network Concurrent REQUIRED)

#GLIB
pkg_check_modules(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})

set(SRCS
        ${SERVICE_DIR}/main.cpp
        ${SERVICE_DIR}/dbus.cpp
        ${SERVICE_DIR}/junk_clean_interface.cpp
        ${SERVICE_DIR}/tool_box_interface.cpp
        ${SERVICE_DIR}/disk_monitor.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/drivermanagerservicedbus.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/drivermanagerclient.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/soundcardinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/networkcardinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/hardwareinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/graphiccardinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/bluetoothinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/driverinfo/driverinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/driverinfo/drivermodeinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/videodevicefinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/soundcardfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/scannerfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/printerfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/otherdevicefinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/networkcardfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/inputdevicefinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/graphiccardfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/devicefinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/bluetoothfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/scannerdriverfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/graphicdriverfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/printerdriverfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/getDriver.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/videoDriverFinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/inputdevicedriverfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/otherdevicedriverfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/pcinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/pcinfogetter.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/cpuinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/cpuinfofinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/memoryinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/memoryinfofinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/baseboardinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/baseboardinfofinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/diskinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/diskinfofinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/monitorinfofinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/monitorinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/keyboardinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/keyboardinfofinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/mouseinfofinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/mouseinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/camerainfofinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/camerainfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/batteryinfofinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/batteryinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/faninfofinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/faninfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/cdinfofinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/cdinfo.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hotplug/usblistenerthread.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/hotplug/usbhotplugserver.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/hotplugdevdriverfinder.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/include/globalsignal.cpp
        ${DRIVER_MANAGER_SERVICE_DIR}/include/hostresolver.cpp
)

set(HEADERS
        ${SERVICE_DIR}/dbus.h
        ${SERVICE_DIR}/dbus_args.h
        ${DRIVER_MANAGER_SERVICE_DIR}/drivermanagerservicedbus.h
        ${DRIVER_MANAGER_SERVICE_DIR}/drivermanagerclient.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/soundcardinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/networkcardinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/hardwareinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/graphiccardinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/bluetoothinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/driverinfo/driverinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/driverinfo/drivermodeinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/videodevicefinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/soundcardfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/scannerfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/printerfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/otherdevicefinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/networkcardfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/inputdevicefinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/graphiccardfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/devicefinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/bluetoothfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/scannerdriverfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/graphicdriverfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/printerdriverfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/getDriver.h
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/videoDriverFinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/inputdevicedriverfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/otherdevicedriverfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/pcinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/pcinfogetter.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/cpuinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/cpuinfofinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/memoryinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/memoryinfofinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/baseboardinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/baseboardinfofinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/diskinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/diskinfofinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/monitorinfofinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/monitorinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/keyboardinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/keyboardinfofinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/mouseinfofinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/mouseinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/camerainfofinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/camerainfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/batteryinfofinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/batteryinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/faninfofinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/faninfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwarefinder/cdinfofinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hardwareinfo/cdinfo.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hotplug/usblistenerthread.h
        ${DRIVER_MANAGER_SERVICE_DIR}/hotplug/usbhotplugserver.h
        ${DRIVER_MANAGER_SERVICE_DIR}/driverfinder/hotplugdevdriverfinder.h
        ${DRIVER_MANAGER_SERVICE_DIR}/include/globalsignal.h
        ${DRIVER_MANAGER_SERVICE_DIR}/include/hostresolver.h
)

include_directories(${SERVICE_DIR})
include_directories(${SERVICE_DIR}/../../kom/)
include_directories(${TOP_DIR}/plugins/service-support/service-support-backend)

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::DBus Qt5::Network Qt5::Concurrent ${GLIB_LIBRARIES} "-ludev" "-lusb-1.0")
target_link_libraries(${PROJECT_NAME} PRIVATE service-support-backend)
target_link_libraries(${PROJECT_NAME} PRIVATE dl)

# 添加SDK模块
function(find_kysdk_package prj_name kysdk_package_name)
    pkg_check_modules(${kysdk_package_name}_PKG ${kysdk_package_name})
    if (${${kysdk_package_name}_PKG_FOUND})
        target_include_directories(${PROJECT_NAME} PRIVATE ${${kysdk_package_name}_PKG_INCLUDE_DIRS})
        target_link_directories(${PROJECT_NAME} PRIVATE ${${kysdk_package_name}_PKG_LIBRARY_DIRS})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${${kysdk_package_name}_PKG_LIBRARIES})
    else()
        set(COMPATIBILITY_NAME compatibility)
        set(COMPATIBILITY_DIR ${TOP_DIR}/${COMPATIBILITY_NAME})
        target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/${kysdk_package_name}/)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_NAME})
    endif()
endfunction()

find_kysdk_package(${PROJECT_NAME} kysdk-log)
find_kysdk_package(${PROJECT_NAME} kysdk-diagnostics)
find_kysdk_package(${PROJECT_NAME} kysdk-system)
find_kysdk_package(${PROJECT_NAME} kysdk-hardware)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.service DESTINATION /lib/systemd/system/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.conf DESTINATION /etc/dbus-1/system.d/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.limit DESTINATION /etc/dbus-1/conf/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.limit.verify DESTINATION /etc/dbus-1/conf/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.limit DESTINATION /usr/share/dbus-1/conf/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.limit.verify DESTINATION /usr/share/dbus-1/conf/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.yaml DESTINATION /etc/kylin-config/basic/)

install(FILES ${SERVICE_DIR}/data/com.kylin.assistant.systemdaemon.limit DESTINATION /etc/dbus-1/conf/)
install(FILES ${SERVICE_DIR}/data/com.kylin.assistant.systemdaemon.limit.verify DESTINATION /etc/dbus-1/conf/)
install(FILES ${SERVICE_DIR}/data/com.kylin.assistant.systemdaemon.limit DESTINATION /usr/share/dbus-1/conf/)
install(FILES ${SERVICE_DIR}/data/com.kylin.assistant.systemdaemon.limit.verify DESTINATION /usr/share/dbus-1/conf/)
install(FILES ${SERVICE_DIR}/data/com.kylin.assistant.systemdaemon.yaml DESTINATION /etc/kylin-config/basic/)

