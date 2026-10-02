# 插件安装目录
set(PLUGIN_INSTALL_DIRS "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/ukui-shortcut-plugins")
add_compile_definitions(PLUGIN_INSTALL_DIRS="${PLUGIN_INSTALL_DIRS}")
# 插件数据文件安装目录
set(SHORTCUT_DATA_INSTALL_DIR "/usr/share/ukui-sidebar/shortcuts")
add_compile_definitions(SHORTCUT_DATA_INSTALL_DIR="${SHORTCUT_DATA_INSTALL_DIR}")
# 插件翻译文件安装目录
set(SHORTCUT_TRANSLATION_FILE_DIR "${SHORTCUT_DATA_INSTALL_DIR}/translations")
add_compile_definitions(SHORTCUT_TRANSLATION_FILE_DIR="${SHORTCUT_TRANSLATION_FILE_DIR}")
