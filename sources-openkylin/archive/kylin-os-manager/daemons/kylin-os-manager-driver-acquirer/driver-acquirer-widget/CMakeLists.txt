project(driver-acquirer-widget LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools)  # 注意要加 LinguistTools，否则 qt5_add_translation 用不了

add_library(driver-acquirer-widget SHARED
        webdriverinstallwidget.cpp
)

target_include_directories(driver-acquirer-widget PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(driver-acquirer-widget PRIVATE Qt5::Widgets)

set(TRANSLATIONS
        "${CMAKE_CURRENT_SOURCE_DIR}/translations/driver-acquirer-widget_zh_CN.ts"
        "${CMAKE_CURRENT_SOURCE_DIR}/translations/driver-acquirer-widget_zh_HK.ts"
        "${CMAKE_CURRENT_SOURCE_DIR}/translations/driver-acquirer-widget_bo_CN.ts"
        "${CMAKE_CURRENT_SOURCE_DIR}/translations/driver-acquirer-widget_mn.ts"
        "${CMAKE_CURRENT_SOURCE_DIR}/translations/driver-acquirer-widget_kk.ts"
        "${CMAKE_CURRENT_SOURCE_DIR}/translations/driver-acquirer-widget_ky.ts"
)

qt5_add_translation(QM_FILES ${TRANSLATIONS})

add_custom_target(translations ALL DEPENDS ${QM_FILES})

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

find_kysdk_package(${PROJECT_NAME} kysdk-qtwidgets)

install(TARGETS driver-acquirer-widget LIBRARY DESTINATION /usr/lib/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/webdriverinstallwidget.h DESTINATION /usr/include/driver-acquirer-widget/)
install(FILES ${QM_FILES} DESTINATION /usr/share/driver-acquirer-widget/translations/)
