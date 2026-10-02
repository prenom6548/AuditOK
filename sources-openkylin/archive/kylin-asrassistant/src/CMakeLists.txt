cmake_minimum_required(VERSION 3.5)

project(kylin-asrassistant LANGUAGES CXX)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

find_package(PkgConfig REQUIRED)

find_package(Qt5 COMPONENTS Widgets Network LinguistTools REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Network)

find_package(Qt5DBus COMPONENTS Dbus REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Qt5DBus_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PRIVATE
		      speechrecognitionclient
		      speechinput
		      )

pkg_check_modules(KYSDKQTWIDGETS_PKG kysdk-qtwidgets)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDKQTWIDGETS_PKG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDKQTWIDGETS_PKG_LIBRARIES})

pkg_check_modules(KYSDKKABASE_PKG kysdk-kabase)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDKKABASE_PKG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDKKABASE_PKG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDKKABASE_PKG_LIBRARIES})

pkg_check_modules(KYSDK_UKENV kysdk-ukenv)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_UKENV_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_UKENV_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_UKENV_LIBRARIES})

pkg_check_modules(KYSDK_ALM kysdk-alm)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_ALM_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_ALM_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_ALM_LIBRARIES})

find_package(KF5WindowSystem)
pkg_check_modules(KYSDK_WAYLANDHELPER kysdk-waylandhelper)
target_include_directories(${PROJECT_NAME} PUBLIC ${KYSDK_WAYLANDHELPER_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${KYSDK_WAYLANDHELPER_LIBRARY_DIRS} KF5::WindowSystem)
target_link_libraries(${PROJECT_NAME} PUBLIC ${KYSDK_WAYLANDHELPER_LIBRARIES})

pkg_check_modules(GSETTINGS gsettings-qt)
target_include_directories(${PROJECT_NAME} PUBLIC ${GSETTINGS_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${GSETTINGS_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${GSETTINGS_LIBRARIES})

pkg_check_modules(DATACOLLECT kysdk-datacollect)
target_include_directories(${PROJECT_NAME} PUBLIC ${DATACOLLECT_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${DATACOLLECT_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DATACOLLECT_LIBRARIES})

set(SRCS
	"${SRC_DIR}/main.cpp"
	"${SRC_DIR}/mainwindow.cpp"
	"${SRC_DIR}/mainwindow.h"
    "${SRC_DIR}/basewin/loadanimation.cpp"
    "${SRC_DIR}/basewin/loadanimation.h"
    "${SRC_DIR}/basewin/shortcutwin.cpp"
    "${SRC_DIR}/basewin/shortcutwin.h"
    "${SRC_DIR}/basewin/publicattributes.hpp"
	"${SRC_DIR}/startup/startuptools.cpp"
	"${SRC_DIR}/startup/startuptools.h"
	"${SRC_DIR}/dbusservice/asrdbusservice.cpp"
	"${SRC_DIR}/dbusservice/asrdbusservice.h"
	"${SRC_DIR}/common/tools.cpp"
	"${SRC_DIR}/common/tools.h"
	"res.qrc")

set(TRANSLATIONS
	"${TOP_DIR}/translations/kylin-asrassistant_zh_CN.ts"
	"${TOP_DIR}/translations/kylin-asrassistant_bo_CN.ts")
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

target_include_directories(${PROJECT_NAME} PUBLIC ${TOP_DIR}/speechinput)
target_include_directories(${PROJECT_NAME} PUBLIC ${TOP_DIR}/speechrecognitionclient)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/basewin)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/startup)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dbusservice)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/common)
target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE /usr/include/KF5/KWindowSystem/)
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${QM_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-asrassistant/translations/)
