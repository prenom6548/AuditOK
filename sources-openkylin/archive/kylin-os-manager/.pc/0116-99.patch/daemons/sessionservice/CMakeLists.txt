cmake_minimum_required(VERSION 3.16)

project(kylin-os-manager-session-service)

set(SESSION_SERVICE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME})

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets DBus LinguistTools Sql REQUIRED)

set(SRCS
        ${SESSION_SERVICE_DIR}/main.cpp
        ${SESSION_SERVICE_DIR}/sessionclient.cpp
        ${SESSION_SERVICE_DIR}/driverinstall/aptinstaller.cpp
        ${SESSION_SERVICE_DIR}/database/drivermanagerdatabase.cpp
        ${SESSION_SERVICE_DIR}/include/monitorinfoinitializer.cpp
        ${SESSION_SERVICE_DIR}/service_support_popup.cpp
)

set(COMPONENT
        ${SESSION_SERVICE_DIR}/../../plugins/fault_diagnosis/component/text_label.cpp
        ${SESSION_SERVICE_DIR}/../../plugins/fault_diagnosis/component/utils.cpp
)

set(HEADERS
        ${SESSION_SERVICE_DIR}/sessionclient.h
        ${SESSION_SERVICE_DIR}/driverinstall/aptinstaller.h
        ${SESSION_SERVICE_DIR}/database/drivermanagerdatabase.h
        ${SESSION_SERVICE_DIR}/include/monitorinfoinitializer.h
        ${SESSION_SERVICE_DIR}/service_support_popup.h
)

set(TRANSLATIONS
        "${SESSION_SERVICE_DIR}/translations/kylin-os-manager-session-service_zh_CN.ts"
        "${SESSION_SERVICE_DIR}/translations/kylin-os-manager-session-service_zh_HK.ts"
        "${SESSION_SERVICE_DIR}/translations/kylin-os-manager-session-service_bo_CN.ts"
        "${SESSION_SERVICE_DIR}/translations/kylin-os-manager-session-service_mn.ts"
        "${SESSION_SERVICE_DIR}/translations/kylin-os-manager-session-service_ug.ts"
        "${SESSION_SERVICE_DIR}/translations/kylin-os-manager-session-service_kk.ts"
        "${SESSION_SERVICE_DIR}/translations/kylin-os-manager-session-service_ky.ts"
)
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

# 头文件
include_directories(${SESSION_SERVICE_DIR})
include_directories(${SESSION_SERVICE_DIR}/../../kom/)
include_directories(${SESSION_SERVICE_DIR}/../../plugins/fault_diagnosis/component/)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Sql)
target_link_libraries(${PROJECT_NAME} PRIVATE dl)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)

target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${COMPONENT} ${HEADERS} ${SESSION_SERVICE_DIR}/resource.qrc ${QM_FILES})

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
find_kysdk_package(${PROJECT_NAME} kysdk-qtwidgets)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/share/kylin-os-manager/kylin-os-manager-session-service/)
install(FILES ${SESSION_SERVICE_DIR}/sessionservice-config.desktop DESTINATION /etc/xdg/autostart)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-os-manager/translations/)
