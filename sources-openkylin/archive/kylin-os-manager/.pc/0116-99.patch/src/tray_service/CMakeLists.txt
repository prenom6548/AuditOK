project(kylin-os-manager-tray)

set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(TRAY_SERVICE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets DBus REQUIRED)

set(SRCS
        ${ROOT_DIR}/interface/kom_tray_interface.cpp
        ${TRAY_SERVICE_DIR}/main.cpp
        ${TRAY_SERVICE_DIR}/daemon.cpp
        ${TRAY_SERVICE_DIR}/tray.cpp
        ${TRAY_SERVICE_DIR}/action.cpp
        ${TRAY_SERVICE_DIR}/service_dbus.cpp
        ${TRAY_SERVICE_DIR}/client_dbus.cpp)

include_directories(${TRAY_SERVICE_DIR})
include_directories(${ROOT_DIR}/3rdparty/kyplugin/)
include_directories(${ROOT_DIR}/interface/)

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE kyplugin)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
