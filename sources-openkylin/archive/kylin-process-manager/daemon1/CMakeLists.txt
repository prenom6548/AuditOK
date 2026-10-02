cmake_minimum_required(VERSION 3.14)

project(kylin-process-resource-manager-daemon LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)

include_directories(
    ${CMAKE_SOURCE_DIR}/utils)

set (SRC_Sources
    src/main.cpp
    src/priorityschedulinghelper.cpp
    src/cpriorityschedulingmanager.cpp
    src/resourcemanager.cpp
)

set (SRC_Headers
    src/priorityschedulinghelper.h
    src/cpriorityschedulingmanager.h
    src/resourcemanager.h
)

qt5_add_dbus_adaptor(SRC_Sources 
    configs/com.kylin.ProcessResourceManagerDaemon.xml
    src/resourcemanager.h
    ResourceManager
    resourcemanagerservice
    ResourceManagerService)

add_executable(kylin-process-resource-manager-daemon
    ${SRC_Sources}
)

target_link_libraries(kylin-process-resource-manager-daemon
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
)

install(TARGETS kylin-process-resource-manager-daemon DESTINATION /usr/bin)
install(FILES configs/com.kylin.ProcessResourceManagerDaemon.yaml DESTINATION /etc/kylin-config/basic)
install(FILES configs/com.kylin.ProcessResourceManagerDaemon.conf DESTINATION /usr/share/dbus-1/system.d)
install(FILES configs/com.kylin.ProcessResourceManagerDaemon.service DESTINATION /usr/share/dbus-1/system-services)
install(FILES configs/kylin-process-resource-manager-daemon.service DESTINATION /lib/systemd/system)