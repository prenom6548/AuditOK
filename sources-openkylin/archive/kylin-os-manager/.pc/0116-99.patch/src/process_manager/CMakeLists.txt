project(kylin-os-manager-process-manager)

set(PROCESS_MANAGER_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS REQUIRED Core DBus)

pkg_check_modules(GLIB-2.0 glib-2.0)
include_directories(${GLIB-2.0_INCLUDE_DIRS})
link_directories(${GLIB-2.0_LIBRARY_DIRS})

set(SRC ${PROCESS_MANAGER_DIR}/main.cpp
        ${PROCESS_MANAGER_DIR}/daemon.cpp
        ${PROCESS_MANAGER_DIR}/conf_parser.cpp
        ${PROCESS_MANAGER_DIR}/process.cpp
        ${PROCESS_MANAGER_DIR}/conf.cpp
        ${PROCESS_MANAGER_DIR}/service_dbus.cpp
        ${PROCESS_MANAGER_DIR}/client_dbus.cpp)

include_directories(${PROCESS_MANAGER_DIR})
add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::DBus)
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLIB-2.0_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
install(FILES ${PROCESS_MANAGER_DIR}/data/process-manager.ini DESTINATION /etc/kylin-os-manager/)
install(FILES ${PROCESS_MANAGER_DIR}/data/kylin-os-manager-process-manager.desktop DESTINATION /etc/xdg/autostart/)
