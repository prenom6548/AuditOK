project(kylin-tcpdump LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)

set(SRCS
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp")

set(TRANSLATIONS
    "${CMAKE_CURRENT_LIST_DIR}/translations/komt-packet-capture_zh_CN.ts"
    "${CMAKE_CURRENT_LIST_DIR}/translations/komt-packet-capture_bo_CN.ts"
    "${CMAKE_CURRENT_LIST_DIR}/translations/komt-packet-capture_mn.ts"
    "${CMAKE_CURRENT_LIST_DIR}/translations/komt-packet-capture_zh_HK.ts")
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT_NAME} ${SRCS} ${QM_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

pkg_check_modules(KYSDK_QTWIDGETS kysdk-qtwidgets)
if (${KYSDK_QTWIDGETS_FOUND})
    include_directories(${KYSDK_QTWIDGETS_INCLUDE_DIRS})
    link_directories(${KYSDK_QTWIDGETS_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_QTWIDGETS_LIBRARIES})
else ()
    include_directories(${CMAKE_CURRENT_LIST_DIR}/../../compatibility/kysdk-qtwidgets/)
    target_link_libraries(${PROJECT_NAME} PRIVATE compatibility)
endif ()

pkg_check_modules(KYSDK_ALM kysdk-alm)
if (${KYSDK_ALM_FOUND})
    include_directories(${KYSDK_ALM_INCLUDE_DIRS})
    link_directories(${KYSDK_ALM_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_ALM_LIBRARIES})
else ()
    include_directories(${CMAKE_CURRENT_LIST_DIR}/../../compatibility/kysdk-alm/)
    target_link_libraries(${PROJECT_NAME} PRIVATE compatibility)
endif ()

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/kylin-os-manager/bin/)
install(FILES ${QM_FILES} DESTINATION /usr/share/kylin-os-manager/treasure-chest/kylin-tcpdump/translations/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/kylin-tcpdump.conf DESTINATION /usr/share/kylin-os-manager/plugins/tool-box/plugins/)
