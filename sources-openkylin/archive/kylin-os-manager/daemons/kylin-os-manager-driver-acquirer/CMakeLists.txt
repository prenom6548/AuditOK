project(kylin-os-manager-driver-acquirer)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig)
find_package(Qt5 COMPONENTS Core Sql Network DBus)

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/deviceinfodatabase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/include/common.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/deviceinfodbusservice.cpp
)

set(HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/src/deviceinfodatabase.h
    ${CMAKE_CURRENT_LIST_DIR}/src/common.h
    ${CMAKE_CURRENT_LIST_DIR}/src/deviceinfodbusservice.h
)

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Sql Qt5::Network Qt5::DBus)

pkg_check_modules(SQLCIPHER REQUIRED sqlcipher)
include_directories(${SQLCIPHER_INCLUDE_DIRS})
link_directories(${SQLCIPHER_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SQLCIPHER_LIBRARIES})

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

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/data/com.kylin-os-manager-driver-acquirer.service DESTINATION /etc/systemd/system/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/data/com.kylin-os-manager-driver-acquirer.conf DESTINATION /etc/dbus-1/system.d/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/data/kylin-os-manager-driver-acquirer.desktop DESTINATION /etc/xdg/autostart/)


add_subdirectory(driver-acquirer-widget)