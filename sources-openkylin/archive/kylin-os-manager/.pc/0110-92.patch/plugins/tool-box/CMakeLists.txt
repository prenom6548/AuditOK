cmake_minimum_required(VERSION 3.5)

project(tool-box)

set(TOOL_BOX_DIR ${CMAKE_CURRENT_LIST_DIR})
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(${PROJECT_NAME} SHARED)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g -rdynamic)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/interface)
target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/3rdparty/kyplugin)

find_package(Qt5 COMPONENTS REQUIRED Widgets DBus Svg LinguistTools)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::DBus Qt5::Svg)

find_package(PkgConfig REQUIRED)

pkg_check_modules(KYSDK_MODULE kysdk-kabase kysdk-ukenv)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARIES})

target_include_directories(${PROJECT_NAME} PRIVATE ${TOOL_BOX_DIR})

include_directories(${TOOL_BOX_DIR}/../../kom/)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)

set(SRCS "${TOOL_BOX_DIR}/connecter.cpp"
         "${TOOL_BOX_DIR}/plugininterface.cpp"
         "${TOOL_BOX_DIR}/toolboxwidget.cpp"
         "${TOOL_BOX_DIR}/appitem.cpp"
		 "${TOOL_BOX_DIR}/utils.cpp"
         "${TOP_DIR}/interface/kom_application_interface.cpp"
         "${TOOL_BOX_DIR}/res.qrc")

set(TRANSLATIONS
	"${TOOL_BOX_DIR}/translations/kylin-os-manager-tool-box_zh_CN.ts"
	"${TOOL_BOX_DIR}/translations/kylin-os-manager-tool-box_bo_CN.ts"
	"${TOOL_BOX_DIR}/translations/kylin-os-manager-tool-box_mn.ts"
	"${TOOL_BOX_DIR}/translations/kylin-os-manager-tool-box_ug.ts"
	"${TOOL_BOX_DIR}/translations/kylin-os-manager-tool-box_kk.ts"
	"${TOOL_BOX_DIR}/translations/kylin-os-manager-tool-box_ky.ts"
	"${TOOL_BOX_DIR}/translations/kylin-os-manager-tool-box_zh_HK.ts"
	"${TOOL_BOX_DIR}/translations/kylin-os-manager-tool-box_de.ts"
	"${TOOL_BOX_DIR}/translations/kylin-os-manager-tool-box_es.ts"
	"${TOOL_BOX_DIR}/translations/kylin-os-manager-tool-box_fr.ts")
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${QM_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/kylin-os-manager/plugins/)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-os-manager/translations/)
