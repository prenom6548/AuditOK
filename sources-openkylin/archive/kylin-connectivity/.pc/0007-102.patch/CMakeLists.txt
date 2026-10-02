cmake_minimum_required(VERSION 3.14)
project(kylin-connectivity)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 COMPONENTS Widgets Network LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network LinguistTools REQUIRED)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS) # CONFIG += no_keywords qt关键字冲突

set(QRC_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/res/resources.qrc")

add_executable(kylin-connectivity)

set(SUBDIRECTORIES_MAIN
        common
        connectmanage
        discovery
        filesearch
        filetransfer
        filesync
        kylinconnectivitymanager
        plugin
        projection
        ui
        dbus
        pcscreenmanage
        3rdparty/plugins-simple
        3rdparty/dsoftbus
        3rdparty/ftpserver
        )
foreach (SUBDIRECTORY_MAIN ${SUBDIRECTORIES_MAIN})
    add_subdirectory(${SUBDIRECTORY_MAIN})
endforeach ()

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

pkg_check_modules(KYSDK_ALM kysdk-alm)
target_include_directories(kylin-connectivity PUBLIC ${KYSDK_ALM_INCLUDE_DIRS})
target_link_directories(kylin-connectivity PUBLIC ${KYSDK_ALM_LIBRARY_DIRS})
target_link_libraries(kylin-connectivity PUBLIC ${KYSDK_ALM_LIBRARIES})

pkg_check_modules(KYSDK_LOG kysdk-log)
target_include_directories(kylin-connectivity PUBLIC ${KYSDK_LOG_INCLUDE_DIRS})
target_link_directories(kylin-connectivity PUBLIC ${KYSDK_LOG_LIBRARY_DIRS})
target_link_libraries(kylin-connectivity PUBLIC ${KYSDK_LOG_LIBRARIES})

pkg_check_modules(KYSDK_WAYLANDHELPER kysdk-waylandhelper)
target_include_directories(kylin-connectivity PUBLIC ${KYSDK_WAYLANDHELPER_INCLUDE_DIRS})
target_link_directories(kylin-connectivity PUBLIC ${KYSDK_WAYLANDHELPER_LIBRARY_DIRS})
target_link_libraries(kylin-connectivity PUBLIC ${KYSDK_WAYLANDHELPER_LIBRARIES})

target_include_directories(kylin-connectivity PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(kylin-connectivity PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        ui
        dbus
        )

set(TRANSLATIONS
        "${CMAKE_CURRENT_LIST_DIR}/translations/kylin-connectivity_en.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/kylin-connectivity_zh_CN.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/kylin-connectivity_bo_CN.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/kylin-connectivity_mn.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/kylin-connectivity_zh_HK.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/peony-send-to-connectivity_zh_CN.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/peony-send-to-connectivity_bo_CN.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/peony-send-to-connectivity_mn.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/peony-send-to-connectivity_zh_HK.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/peony-vfs-kylin-multiterminal_zh_CN.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/peony-vfs-kylin-multiterminal_bo_CN.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/peony-vfs-kylin-multiterminal_mn.ts"
	"${CMAKE_CURRENT_LIST_DIR}/translations/peony-vfs-kylin-multiterminal_zh_HK.ts")
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

target_sources(kylin-connectivity PRIVATE main.cpp ${QRC_MAIN} ${QM_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/kylin-connectivity.desktop DESTINATION /usr/share/applications/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/3rdparty/scrcpy-server.jar DESTINATION /usr/share/kylin-connectivity/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/config/phoneServerConfig.ini DESTINATION $ENV{HOME}/.config/kylin-connectivity/)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-connectivity/translations/)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data/ DESTINATION /usr/share/kylin-user-guide/data/guide/)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/output/lib/dsoftbus/ DESTINATION /opt/kylin-connectivity/dsoftbus/ USE_SOURCE_PERMISSIONS)
install(PROGRAMS ${CMAKE_CURRENT_LIST_DIR}/3rdparty/sndcpy DESTINATION /usr/share/kylin-connectivity/)
install(PROGRAMS ${CMAKE_CURRENT_LIST_DIR}/3rdparty/plugins-simple/src/lib/libsimple.so DESTINATION /usr/lib/)
install(PROGRAMS ${CMAKE_CURRENT_LIST_DIR}/output/bin/connectivity_softbus_server DESTINATION /opt/kylin-connectivity/)
install(PROGRAMS ${CMAKE_CURRENT_LIST_DIR}/output/bin/connectivity_softbus_client DESTINATION /opt/kylin-connectivity/)

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/3rdparty/tools/arm/ DESTINATION /opt/kylin-connectivity/tools USE_SOURCE_PERMISSIONS)
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/3rdparty/tools/amd/ DESTINATION /opt/kylin-connectivity//tools USE_SOURCE_PERMISSIONS)
endif()
install(FILES ${CMAKE_CURRENT_LIST_DIR}/config/control.conf DESTINATION /usr/share/kylin-connectivity/)
