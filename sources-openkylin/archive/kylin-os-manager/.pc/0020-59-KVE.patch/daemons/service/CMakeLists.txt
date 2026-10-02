project(kylin-os-manager-daemon)

set(SERVICE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core DBus REQUIRED)

set(SRCS
        ${SERVICE_DIR}/main.cpp
        ${SERVICE_DIR}/dbus.cpp)

include_directories(${SERVICE_DIR})
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.service DESTINATION /lib/systemd/system/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.conf DESTINATION /etc/dbus-1/system.d/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.limit DESTINATION /etc/dbus-1/conf/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.limit DESTINATION /usr/share/dbus-1/conf/)
install(FILES ${SERVICE_DIR}/data/com.kylin-os-manager.limit.verify DESTINATION /etc/dbus-1/conf/)
