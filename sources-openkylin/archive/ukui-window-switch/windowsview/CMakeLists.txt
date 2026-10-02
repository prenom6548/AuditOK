cmake_minimum_required(VERSION 3.16)

find_package(QT NAMES Qt6 Qt5 COMPONENTS LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools REQUIRED)

file(GLOB TS_FILES translations/*.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/translations)
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/ukui-window-switch-wlcom ${CMAKE_CURRENT_SOURCE_DIR}/ukui-window-switch-kwineffect ${CMAKE_CURRENT_SOURCE_DIR}/qml ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)

add_custom_target(generate_qm ALL DEPENDS ${QM_FILES})

set(TRANSLATION_DIR "/usr/share/ukui-window-switch/translations/")
add_compile_definitions(TRANSLATION_DIR="${TRANSLATION_DIR}")

add_subdirectory(ukui-window-switch-kwineffect)
add_subdirectory(ukui-window-switch-wlcom)
install(FILES ukui-window-switch DESTINATION /usr/bin/)
install(FILES ukui-window-switch.desktop DESTINATION /etc/xdg/autostart/)
install(FILES ${QM_FILES} DESTINATION ${TRANSLATION_DIR})
