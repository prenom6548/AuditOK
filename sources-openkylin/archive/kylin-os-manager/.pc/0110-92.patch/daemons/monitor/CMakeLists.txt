project(kylin-os-manager-monitor)

set(MONITOR_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)

add_executable(${PROJECT_NAME})

find_package(PkgConfig REQUIRED)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

find_package(Qt5 REQUIRED COMPONENTS Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)

find_package(Qt5 REQUIRED COMPONENTS DBus)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)

find_package(Qt5 REQUIRED COMPONENTS Sql)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Sql)

find_package(Qt5 REQUIRED COMPONENTS Network)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Network)

find_package(PkgConfig)
pkg_check_modules(KYSDK_MODULE kysdk-log kysdk-diagnostics)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARIES})

pkg_check_modules(QGSETTINGS gsettings-qt)
target_include_directories(${PROJECT_NAME} PRIVATE ${QGSETTINGS_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${QGSETTINGS_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${QGSETTINGS_LIBRARIES})

pkg_check_modules(LIBGTOP libgtop-2.0)
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBGTOP_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBGTOP_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBGTOP_LIBRARIES})

include_directories(${MONITOR_DIR}/../../kom/)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_definitions(-DDEBUG_MODE)
endif()

set(SRCS
	"${MONITOR_DIR}/main.cpp"
	"${MONITOR_DIR}/crash-collect.cpp"
	"${MONITOR_DIR}/crash-info.cpp"
	"${MONITOR_DIR}/crash-monitor.cpp"
	"${MONITOR_DIR}/crash-database.cpp"
	"${MONITOR_DIR}/systemmonitor.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE ${MONITOR_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
install(FILES ${MONITOR_DIR}/data/komd.crash.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
install(FILES ${MONITOR_DIR}/data/com.kylin-os-manager-monitor.service DESTINATION /lib/systemd/system/)
