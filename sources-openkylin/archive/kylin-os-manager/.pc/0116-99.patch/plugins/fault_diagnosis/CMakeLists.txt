project(komp-fault-diagnosis LANGUAGES CXX)

set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g")

find_package(PkgConfig)
find_package(Qt5 COMPONENTS Widgets DBus LinguistTools REQUIRED)

add_definitions(-DSQLITE_HAS_CODEC)
add_definitions(-DSQLITE_CORE)

set(SRCS
    ${TOP_DIR}/plugins/common/frame.cpp
    ${TOP_DIR}/interface/kom_application_interface.cpp
    ${SRC_DIR}/component/text_label.cpp
    ${SRC_DIR}/component/icon_label.cpp
    ${SRC_DIR}/component/utils.cpp
    ${SRC_DIR}/component/user_experience_dialog.cpp
    ${SRC_DIR}/connecter.cpp
    ${SRC_DIR}/fault_diagnosis.cpp
    ${SRC_DIR}/diagnosis_manage.cpp
    ${SRC_DIR}/diagnosis_core.cpp
    ${SRC_DIR}/database.cpp
    ${SRC_DIR}/home_page_widget.cpp
    ${SRC_DIR}/mainwindow.cpp
    ${SRC_DIR}/repair_specific_error_widget.cpp
    ${SRC_DIR}/diagnosis_group_widget.cpp
    ${SRC_DIR}/diagnosis_entry_widget.cpp
    ${SRC_DIR}/diagnosis_subentry_widget.cpp
    ${SRC_DIR}/prompt.cpp)

set(DIAGNOSIS_SRCS
    ${SRC_DIR}/diagnoses/diagnosis.h
    ${SRC_DIR}/diagnoses/complete_machine.cpp
    ${SRC_DIR}/diagnoses/specific_error.cpp)

set(3RDPARTY_SRCS
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/common_ui.cpp
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/toolutils.cpp
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/custom_label.cpp
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/logo_icon_title_bar.cpp
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/daemonipcdbus.cpp
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/menumodule.cpp
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/quad_btns_title_bar.cpp
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/ipweb_widget.cpp
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/ipweb_widget.ui
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/increase_widget.cpp
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/increase_widget.ui
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/config_win.cpp
    ${SRC_DIR}/3rdparty/internal_network_diagnosis/config_win.ui)

set(TRANSLATIONS
    ${SRC_DIR}/translations/fault-diagnosis_zh_CN.ts
    ${SRC_DIR}/translations/fault-diagnosis_zh_HK.ts
    ${SRC_DIR}/translations/fault-diagnosis_bo_CN.ts
    ${SRC_DIR}/translations/fault-diagnosis_mn.ts)
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/translations)
qt5_add_translation(QM_FILES ${TRANSLATIONS})

include_directories(${TOP_DIR}/plugins/common/)
include_directories(${TOP_DIR}/3rdparty/kyplugin/)
include_directories(${TOP_DIR}/interface/)
include_directories(${TOP_DIR}/kom)
include_directories(${SRC_DIR}/)
include_directories(${SRC_DIR}/component/)
include_directories(${SRC_DIR}/diagnoses/)
include_directories(${SRC_DIR}/3rdparty/internal_network_diagnosis/)

add_library(${PROJECT_NAME} SHARED ${SRCS} ${DIAGNOSIS_SRCS} ${QM_FILES} ${SRC_DIR}/resource.qrc ${3RDPARTY_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)

pkg_check_modules(QGSETTINGS REQUIRED gsettings-qt)
include_directories(${QGSETTINGS_INCLUDE_DIRS})
link_directories(${QGSETTINGS_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${QGSETTINGS_LIBRARIES})

pkg_check_modules(SQLCIPHER REQUIRED sqlcipher)
include_directories(${SQLCIPHER_INCLUDE_DIRS})
link_directories(${SQLCIPHER_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SQLCIPHER_LIBRARIES})

pkg_check_modules(GLIB-2.0 REQUIRED glib-2.0)
include_directories(${GLIB-2.0_INCLUDE_DIRS})
link_directories(${GLIB-2.0_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLIB-2.0_LIBRARIES})

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

pkg_check_modules(KYSDK_SOUNDEFFECTS kysdk-soundeffects)
if (${KYSDK_SOUNDEFFECTS_FOUND})
    include_directories(${KYSDK_SOUNDEFFECTS_INCLUDE_DIRS})
    link_directories(${KYSDK_SOUNDEFFECTS_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_SOUNDEFFECTS_LIBRARIES})
else ()
    include_directories(${TOP_DIR}/compatibility/kysdk-soundeffects/)
    target_link_libraries(${PROJECT_NAME} PRIVATE compatibility)
endif ()

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/kylin-os-manager/plugins/)
install(FILES ${SRC_DIR}/data/i18n.conf DESTINATION /usr/share/kylin-os-manager/fault-diagnosis/)
install(FILES ${QM_FILES} DESTINATION /usr/share/kylin-os-manager/fault-diagnosis/translations/)

add_subdirectory(test)
