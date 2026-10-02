project(tool-box)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g")

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets DBus LinguistTools REQUIRED)

set(SRCS
    "${TOP_DIR}/interface/kom_application_interface.cpp"
    "${TOP_DIR}/plugins/common/frame.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/connecter.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/tool_box.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/app_item.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/installer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/blankpage.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/resource.qrc")

set(COMPONENT_SRCS
    "${TOP_DIR}/plugins/fault_diagnosis/component/text_label.cpp"
    "${TOP_DIR}/plugins/fault_diagnosis/component/icon_label.cpp"
    "${TOP_DIR}/plugins/fault_diagnosis/component/utils.cpp"
	"${TOP_DIR}/plugins/fault_diagnosis/component/user_experience_dialog.cpp")

set(TRANSLATIONS
	"${CMAKE_CURRENT_LIST_DIR}/translations/tool-box_zh_CN.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/tool-box_zh_HK.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/tool-box_bo_CN.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/tool-box_mn.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/tool-box_ug.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/tool-box_kk.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/tool-box_ky.ts")
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

include_directories(${TOP_DIR}/interface/)
include_directories(${TOP_DIR}/3rdparty/kyplugin)
include_directories(${TOP_DIR}/kom/)
include_directories(${TOP_DIR}/plugins/common/)
include_directories(${TOP_DIR}/plugins/fault_diagnosis/component/)
include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(${PROJECT_NAME} SHARED ${SRCS} ${COMPONENT_SRCS} ${QM_FILES})
# add_library(${PROJECT_NAME} SHARED ${SRCS} ${DIAGNOSIS_SRCS} ${QM_FILES} ${SRC_DIR}/resource.qrc)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::DBus)

pkg_check_modules(GLIB_2.0 REQUIRED glib-2.0)
include_directories(${GLIB_2.0_INCLUDE_DIRS})
link_directories(${GLIB_2.0_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLIB_2.0_LIBRARIES})

pkg_check_modules(KYSDK_QTWIDGETS kysdk-qtwidgets)
if (${KYSDK_QTWIDGETS_FOUND})
	include_directories(${KYSDK_QTWIDGETS_INCLUDE_DIRS})
	link_directories(${KYSDK_QTWIDGETS_LIBRARY_DIRS})
	target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_QTWIDGETS_LIBRARIES})
else ()
	include_directories(${TOP_DIR}/compatibility/kysdk-qtwidgets/)
	target_link_libraries(${PROJECT_NAME} PRIVATE compatibility)
endif ()

pkg_check_modules(KYSDK_WAYLANDHELPER kysdk-waylandhelper)
if (${KYSDK_WAYLANDHELPER_FOUND})
	include_directories(${KYSDK_WAYLANDHELPER_INCLUDE_DIRS})
	link_directories(${KYSDK_WAYLANDHELPER_LIBRARY_DIRS})
	target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_WAYLANDHELPER_LIBRARIES})
else ()
	include_directories(${TOP_DIR}/compatibility/kysdk-waylandhelper/)
	target_link_libraries(${PROJECT_NAME} PRIVATE compatibility)
endif ()

pkg_check_modules(KYSDK_UKENV kysdk-ukenv)
if (${KYSDK_UKENV_FOUND})
	include_directories(${KYSDK_UKENV_INCLUDE_DIRS})
	link_libraries(${KYSDK_UKENV_LIBRARY_DIRS})
	target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_UKENV_LIBRARIES})
else ()
	include_directories(${TOP_DIR}/compatibility/kysdk-ukenv/)
	target_link_libraries(${PROJECT_NAME} PRIVATE compatibility)
endif ()

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/kylin-os-manager/plugins/)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-os-manager/plugins/tool-box/translations/)

add_subdirectory(app_launcher)
