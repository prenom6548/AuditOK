project(kylin-source-update-tool LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g")
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS DBus Widgets LinguistTools REQUIRED)

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp)

set(COMPONENT_SRCS
    ${TOP_DIR}/plugins/fault_diagnosis/component/text_label.cpp
    ${TOP_DIR}/plugins/fault_diagnosis/component/utils.cpp)

set(TRANSLATIONS
    ${CMAKE_CURRENT_LIST_DIR}/translations/kylin-source-update-tool_zh_CN.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/kylin-source-update-tool_bo_CN.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/kylin-source-update-tool_mn.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/kylin-source-update-tool_zh_HK.ts)
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/translations)
qt5_add_translation(QM_FILES ${TRANSLATIONS})

include_directories(${TOP_DIR}/kom/)
include_directories(${TOP_DIR}/plugins/fault_diagnosis/component/)
include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT_NAME} ${SRCS} ${COMPONENT_SRCS} ${QM_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::DBus)

pkg_check_modules(KYSDK_QTWIDGETS kysdk-qtwidgets)
if (${KYSDK_QTWIDGETS_FOUND})
    include_directories(${KYSDK_QTWIDGETS_INCLUDE_DIRS})
    link_directories(${KYSDK_QTWIDGETS_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_QTWIDGETS_LIBRARIES})
else ()
    include_directories(${TOP_DIR}/compatibility/kysdk-qtwidgets/)
    target_link_libraries(${PROJECT_NAME} PRIVATE compatibility)
endif ()

pkg_check_modules(KYSDK_ALM kysdk-alm)
if (${KYSDK_ALM_FOUND})
    include_directories(${KYSDK_ALM_INCLUDE_DIRS})
    link_directories(${KYSDK_ALM_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_ALM_LIBRARIES})
else ()
    include_directories(${TOP_DIR}/compatibility/kysdk-alm)
    target_link_libraries(${PROJECT_NAME} PRIVATE compatibility)
endif ()

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/kylin-os-manager/bin/)
install(FILES ${QM_FILES} DESTINATION /usr/share/kylin-os-manager/treasure-chest/kylin-source-update-tool/translations/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/kylin-source-update-tool.conf DESTINATION /usr/share/kylin-os-manager/plugins/tool-box/plugins/)
