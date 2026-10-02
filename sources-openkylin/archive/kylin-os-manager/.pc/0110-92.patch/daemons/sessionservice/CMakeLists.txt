cmake_minimum_required(VERSION 3.16)

project(kylin-os-manager-session-service)

set(SESSION_SERVICE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)

add_executable(${PROJECT_NAME})

find_package(Qt5 COMPONENTS Core DBus LinguistTools DBus Sql REQUIRED)

set(SRCS
        ${SESSION_SERVICE_DIR}/main.cpp
        ${SESSION_SERVICE_DIR}/sessionclient.cpp
        ${SESSION_SERVICE_DIR}/driverinstall/aptinstaller.cpp
        ${SESSION_SERVICE_DIR}/database/drivermanagerdatabase.cpp
)


set(HEADERS
        ${SESSION_SERVICE_DIR}/sessionclient.h
        ${SESSION_SERVICE_DIR}/driverinstall/aptinstaller.h
        ${SESSION_SERVICE_DIR}/database/drivermanagerdatabase.h
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

include_directories(${SESSION_SERVICE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Sql)
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${HEADERS} ${QM_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/share/kylin-os-manager/kylin-os-manager-session-service/)
install(FILES ${SESSION_SERVICE_DIR}/sessionservice-config.desktop DESTINATION /etc/xdg/autostart)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-os-manager/translations/)
