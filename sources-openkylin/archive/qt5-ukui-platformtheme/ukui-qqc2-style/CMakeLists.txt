project(ukui-qqc2-style)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Gui
    Qml
    Quick
    LinguistTools
)

file(GLOB QML_UKUIFILES "org.ukui.style/*.qml" "org.ukui.style/qmldir")
file(GLOB PRIVATE_FILES "org.ukui.style/private/*")
file(GLOB_RECURSE OTHER_FILES qml.qrc)

SOURCE_GROUP("QMLFiles" FILES ${QML_UKUIFILES})
SOURCE_GROUP("QMLPrivateFiles" FILES ${PRIVATE_FILES})
SOURCE_GROUP("other files" FILES ${OTHER_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${QMLFiles} ${QMLPrivateFiles} ${OTHER_FILES})

message("cmake_install_libdir..." , ${CMAKE_INSTALL_LIBDIR})

# ======================================
# 翻译配置（完全贴合项目实际目录）
# ======================================
set(TRANSLATION_FILES
    translations/qt5-ukui-qqc2-style_en_US.ts
    translations/qt5-ukui-qqc2-style_zh_CN.ts
    translations/qt5-ukui-qqc2-style_zh_Hant.ts
    translations/qt5-ukui-qqc2-style_de.ts
    translations/qt5-ukui-qqc2-style_fr.ts
    translations/qt5-ukui-qqc2-style_es.ts
    translations/qt5-ukui-qqc2-style_tr.ts
    translations/qt5-ukui-qqc2-style_vi.ts
)

# 关键：只执行一次翻译提取+编译（避免重复导致路径混乱）
# qt5_create_translation 会自动生成 .ts（提取文本）和 .qm（编译）
qt5_create_translation(QM_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/org.ukui.style  # 扫描 QML 文本
    ${TRANSLATION_FILES}                       # 输入：.ts 模板
    OPTIONS -no-obsolete                       # 忽略过时翻译（可选）
)

# 修正：安装路径和项目其他模块完全一致（无 translations 子目录）
if(UNIX)
    # 项目统一翻译安装目录：/usr/share/qt5-ukui-platformtheme/
    set(TRANSLATION_INSTALL_DIR "/usr/share/qt5-ukui-platformtheme/")
    message("翻译安装路径（和其他模块一致）：" ${TRANSLATION_INSTALL_DIR})

    # 安装 .qm 文件到统一目录（和其他模块的 .qm 放在一起）
    install(FILES ${QM_FILES}
        DESTINATION ${TRANSLATION_INSTALL_DIR}  # 直接安装到目录根，不嵌套
        COMPONENT translations
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ  # 确保读取权限
    )
endif()

# 资源配置（可选，不影响安装）
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/qml.qrc)
    foreach(QM_FILE ${QM_FILES})
        get_filename_component(QM_FILENAME ${QM_FILE} NAME)
        set_source_files_properties(${QM_FILE} PROPERTIES
            QT_RESOURCE_ALIAS "translations/${QM_FILENAME}"
        )
    endforeach()
    qt5_add_resources(QML_RESOURCES ${OTHER_FILES} ${QM_FILES})
endif()

# 翻译目标（确保构建时自动生成 .qm）
add_custom_target(ukui_qqc2_style_translations ALL DEPENDS ${QM_FILES})

# QML 文件安装不变
if(UNIX)
    message("CMAKE_LIBRARY_ARCHITECTURE123....." "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/QtQuick/Controls.2/org.ukui.style/")
    set(TARGET_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/QtQuick/Controls.2/org.ukui.style/")
    message("qqc2 targetpath", ${TARGET_PATH})
    set(TARGET_UKUIFILES ${QML_UKUIFILES})
    install(FILES ${TARGET_UKUIFILES} DESTINATION ${TARGET_PATH})
    install(FILES ${PRIVATE_FILES} DESTINATION "${TARGET_PATH}/private")
endif()
