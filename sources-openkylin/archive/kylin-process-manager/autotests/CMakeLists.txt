cmake_minimum_required(VERSION 3.5)

project(AutoTests LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")

enable_testing()
add_definitions(-DQT_NO_KEYWORDS)
find_package(PkgConfig REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Concurrent)
find_package(KF5KIO)
find_package(KF5WindowSystem REQUIRED)

pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(LIBPROC2 REQUIRED libproc2)

include_directories(${GIO_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/base)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../utils)
include_directories(${CMAKE_SOURCE_DIR}/core)
include_directories(${CMAKE_SOURCE_DIR}/base)
include_directories(${CMAKE_SOURCE_DIR}/daemon/src)

add_executable(test_appinfomanager
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/appinfomanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/desktopfilemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/selection/selectionchecker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/appinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/timewheel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/systemddbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/appinfohelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/processinfohelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    tst_appinfomanager.cpp
)

add_executable(test_configmanager
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    tst_configmanager.cpp
)

add_executable(test_processinfohelper
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/processinfohelper.cpp
    tst_processinfohelper.cpp
)

add_executable(test_exceptiongroupprocesswatcher
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/exceptiongroupprocesswatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/processinfomanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/appinfomanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/desktopfilemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/selection/selectionchecker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/appinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/timewheel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/systemddbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/appinfohelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/processinfohelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    tst_exceptiongroupprocesswatcher.cpp
)

add_executable(test_timewheel
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/timewheel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/timewheel.cpp
    tst_timewheel.cpp
)

add_executable(test_misc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    tst_misc.cpp
)

add_executable(test_appinfohelper
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/appinfohelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/appinfohelper.cpp
    tst_appinfohelper.cpp
)

add_executable(test_appchooser
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/appchooser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/appchooser.cpp
    tst_appchooser.cpp
)

add_executable(test_schedpolicy
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    tst_schedpolicy.cpp
)

add_test(NAME AutoTests COMMAND test_appinfomanager test_configmanager)

target_link_libraries(
    test_appinfomanager PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    KF5::KIOCore
    KF5::WindowSystem
    ${GSETTINGQT_LIBRARIES}
    ${GIO_LIBRARIES}
    jsoncpp
    pthread
    ${LIBPROC2_LIBRARIES}
    X11
)

target_link_libraries(
    test_configmanager PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    ${GSETTINGQT_LIBRARIES}
    ${GIO_LIBRARIES}
    jsoncpp
)

target_link_libraries(
    test_processinfohelper PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    ${GIO_LIBRARIES}
    ${LIBPROC2_LIBRARIES}
)

target_link_libraries(
    test_exceptiongroupprocesswatcher PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    KF5::KIOCore
    KF5::WindowSystem
    ${GSETTINGQT_LIBRARIES}
    ${GIO_LIBRARIES}
    jsoncpp
    pthread
    ${LIBPROC2_LIBRARIES}
    X11
)

target_link_libraries(
    test_timewheel PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
)

target_link_libraries(
    test_misc PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
)

target_link_libraries(
    test_appinfohelper PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    ${GIO_LIBRARIES}
    KF5::WindowSystem
    KF5::KIOCore
)

target_link_libraries(
    test_appchooser PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    ${GIO_LIBRARIES}
)

target_link_libraries(
    test_schedpolicy PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME test_schedpolicy COMMAND test_schedpolicy)

add_executable(test_appinfo
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/appinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/processinfohelper.cpp
    tst_appinfo.cpp
)

target_include_directories(test_appinfo PRIVATE ${KF5CoreAddons_INCLUDE_DIRS})
target_link_libraries(
    test_appinfo PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    ${GIO_LIBRARIES}
    KF5::CoreAddons
    KF5::KIOCore
    ${LIBPROC2_LIBRARIES}
)

add_executable(test_daemondbusinterface
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/daemondbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    tst_daemondbusinterface.cpp
)

target_link_libraries(
    test_daemondbusinterface PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME test_daemondbusinterface COMMAND test_daemondbusinterface)

add_executable(test_systemddbusinterface
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/systemddbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    tst_systemddbusinterface.cpp
)

target_link_libraries(
    test_systemddbusinterface PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME test_systemddbusinterface COMMAND test_systemddbusinterface)

add_executable(test_applauncher
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/applauncher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/appinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/processinfohelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/appinfohelper.cpp
    tst_applauncher.cpp
)
target_link_libraries(
    test_applauncher PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    ${GIO_LIBRARIES}
    KF5::KIOCore
    KF5::WindowSystem
    pthread
    ${LIBPROC2_LIBRARIES}
)

# add_executable(tst_applaunchmanager tst_applaunchmanager.cpp)
# target_link_libraries(tst_applaunchmanager
#     PRIVATE
#     Qt5::Test
#     Qt${QT_VERSION_MAJOR}::DBus
# )
# add_test(NAME tst_applaunchmanager COMMAND tst_applaunchmanager)

add_executable(tst_appwhitelist tst_appwhitelist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/appwhitelist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp)
target_link_libraries(tst_appwhitelist
    PRIVATE
    Qt5::Test
    ${GSETTINGQT_LIBRARIES}
    jsoncpp
)
add_test(NAME tst_appwhitelist COMMAND tst_appwhitelist)

add_executable(tst_configmanager tst_configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp)
target_link_libraries(tst_configmanager
    PRIVATE
    Qt5::Test
    ${GSETTINGQT_LIBRARIES}
    jsoncpp
)
add_test(NAME tst_configmanager COMMAND tst_appwhitelist)

add_executable(tst_datacollector tst_datacollector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/datacollector.cpp)
target_link_libraries(tst_datacollector
    PRIVATE
    Qt5::Test
    ${DATACOLLECT_LIBRARIES}
    pthread
)
add_test(NAME tst_datacollector COMMAND tst_datacollector)

add_executable(tst_desktopfilemanager tst_desktopfilemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/desktopfilemanager.cpp)
target_link_libraries(tst_desktopfilemanager
    PRIVATE
    Qt5::Test
    Qt${QT_VERSION_MAJOR}::Concurrent
    KF5::KIOCore
    pthread
)
add_test(NAME tst_desktopfilemanager COMMAND tst_desktopfilemanager)

add_executable(tst_powermanager tst_powermanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/devicestate.cpp)
target_link_libraries(tst_powermanager
    PRIVATE
    Qt5::Test
    Qt5::DBus
    ${GSETTINGQT_LIBRARIES}
)
add_test(NAME tst_powermanager COMMAND tst_powermanager)

add_executable(tst_devicemodemanager tst_devicemodemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/devicestate.cpp)
target_link_libraries(tst_devicemodemanager
    PRIVATE
    Qt5::Test
    Qt5::DBus
    ${GSETTINGQT_LIBRARIES}
)
add_test(NAME tst_devicemodemanager COMMAND tst_devicemodemanager)

add_executable(tst_groupmanager tst_groupmanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../core/groupmanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../base/daemondbusinterface.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../base/groupmanagementunit.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../base/systemddbusinterface.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../base/resourcecontroller.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../base/profileattribute.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../base/profileaction.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../core/resourcemanagerinterface.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../utils/processinfohelper.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp)
target_link_libraries(tst_groupmanager
   PRIVATE
   Qt5::Test
   Qt5::Core
   Qt5::DBus
   Qt${QT_VERSION_MAJOR}::Widgets
   ${GIO_LIBRARIES}
   jsoncpp
   ${LIBPROC2_LIBRARIES}
)
add_test(NAME tst_groupmanager COMMAND tst_groupmanager)

add_executable(tst_eventwatcher tst_eventwatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/eventwatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/devicestate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/daemondbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp)
target_link_libraries(tst_eventwatcher
    PRIVATE
    Qt5::Test
    Qt5::DBus
    KF5::WindowSystem
    ${GSETTINGQT_LIBRARIES}
)
add_test(NAME tst_eventwatcher COMMAND tst_eventwatcher)

add_executable(tst_multimediacontroller tst_multimediacontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/multimediacontroller.cpp)
target_link_libraries(tst_multimediacontroller
    PRIVATE
    Qt5::Test
    Qt5::DBus
    Qt5::Core
    Qt5::Widgets
)
add_test(NAME tst_multimediacontroller COMMAND tst_multimediacontroller)

add_executable(tst_processinfomanager tst_processinfomanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/processinfomanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/appinfomanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/appinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/selection/selectionchecker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/desktopfilemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/timewheel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/appinfohelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/processinfohelper.cpp)
target_link_libraries(tst_processinfomanager
    PRIVATE
    Qt5::Test
    Qt5::Core
    Qt5::Widgets
    ${GSETTINGQT_LIBRARIES}
    ${GIO_LIBRARIES}
    KF5::KIOCore
    KF5::WindowSystem
    jsoncpp
    ${LIBPROC2_LIBRARIES}
    X11
    pthread
)
add_test(NAME tst_processinfomanager COMMAND tst_processinfomanager)

add_executable(tst_resourcemanagerinterface tst_resourcemanagerinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/resourcemanagerinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/daemondbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/systemddbusinterface.cpp)
target_link_libraries(tst_resourcemanagerinterface
    PRIVATE
    Qt5::Test
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
)
add_test(NAME tst_resourcemanagerinterface COMMAND tst_resourcemanagerinterface)

add_executable(tst_systemnotifiessender tst_systemnotifiessender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/systemnotifiessender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/notifydbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp)
target_link_libraries(tst_systemnotifiessender
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
    ${GIO_LIBRARIES}
    ${GSETTINGQT_LIBRARIES}
    jsoncpp
)
add_test(NAME tst_systemnotifiessender COMMAND tst_systemnotifiessender)

add_executable(tst_cpuenergymeter tst_cpuenergymeter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/energymeter/cpuenergymeter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/daemondbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp)
target_link_libraries(tst_cpuenergymeter
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt5::Widgets
)
add_test(NAME tst_cpuenergymeter COMMAND tst_cpuenergymeter)

add_executable(tst_processgroupconsumptionmonitor tst_processgroupconsumptionmonitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/consumptionmonitor/processgroupconsumptionmonitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/energymeter/cpuenergymeter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/daemondbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp)
target_link_libraries(tst_processgroupconsumptionmonitor
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
    ${LIBPROC2_LIBRARIES}
)
add_test(NAME tst_processgroupconsumptionmonitor COMMAND tst_processgroupconsumptionmonitor)

add_executable(tst_upowersystemenery tst_upowersystemenery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/systemenery/upowersystemenery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp)
target_link_libraries(tst_upowersystemenery
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME tst_upowersystemenery COMMAND tst_upowersystemenery)

add_executable(test_singleton
    tst_singleton.cpp
)

target_link_libraries(
    test_singleton PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    pthread
)

add_test(NAME test_singleton COMMAND test_singleton)

add_executable(test_groupmanagementunit
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/groupmanagementunit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/resourcecontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/profileattribute.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/profileaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/resourcemanagerinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/schedpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/systemddbusinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/daemondbusinterface.cpp
    tst_groupmanagementunit.cpp
)

target_link_libraries(
    test_groupmanagementunit PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
    jsoncpp
)

add_test(NAME test_groupmanagementunit COMMAND test_groupmanagementunit)

# 添加resource测试
add_executable(test_resource
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/resource.cpp
    tst_resource.cpp
)

target_link_libraries(
    test_resource PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(NAME test_resource COMMAND test_resource)

# 添加cgroupmanager测试 - 使用真实的libcgroup库
add_executable(test_cgroupmanager
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/cgroupmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/cgroupv2releasenotification.cpp
    tst_cgroupmanager.cpp
)

target_link_libraries(
    test_cgroupmanager PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    cgroup
    pthread
)

add_test(NAME test_cgroupmanager COMMAND test_cgroupmanager)

add_executable(test_pressurewatcher
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/pressurewatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/resource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/resourcewatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    tst_pressurewatcher.cpp
)

target_link_libraries(
    test_pressurewatcher PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
)
add_test(NAME test_pressurewatcher COMMAND test_pressurewatcher)

add_executable(test_processmanager
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/processmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/resourcewatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/resource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/systemresourcemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/cpuraplenergymeter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/cgroupmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/systemdunitmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/cgroupv2releasenotification.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/pressurewatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src/memorywatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/misc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/systemddbusinterface.cpp
    tst_processmanager.cpp
)

target_include_directories(test_processmanager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../daemon/src
)

target_link_libraries(
    test_processmanager PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Widgets
    pthread
    jsoncpp
    cgroup
    ${LIBPROC2_LIBRARIES}
)

add_test(NAME test_processmanager COMMAND test_processmanager)