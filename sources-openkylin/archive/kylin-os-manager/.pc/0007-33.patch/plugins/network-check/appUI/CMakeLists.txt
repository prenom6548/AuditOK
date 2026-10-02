cmake_minimum_required(VERSION 3.5)
project(network-check LANGUAGES CXX)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../)
set(NETWORK_CHECK_DIR ${CMAKE_CURRENT_LIST_DIR}/../)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

find_package(PkgConfig REQUIRED)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools X11Extras Network DBus Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools X11Extras Network DBus Concurrent REQUIRED)
find_package(KF5WindowSystem REQUIRED)

pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)
#pkg_check_modules(KYSDKKABASE_PKG kysdk-kabase)
#pkg_check_modules(KYSDKQTWIDGETS_PKG kysdk-qtwidgets)
#pkg_check_modules(KYSDK_WAYLANDHELPER kysdk-waylandhelper)

link_directories(${PROJECT_SOURCE_DIR}/../lib)

include_directories(
${PROJECT_SOURCE_DIR}/include/
${PROJECT_SOURCE_DIR}/../libBase/include/
${PROJECT_SOURCE_DIR}/../customWidget/
${PROJECT_SOURCE_DIR}/../Observer_Notifier/
${PROJECT_SOURCE_DIR}/../libNWDBus/include
${PROJECT_SOURCE_DIR}/../HWCheck/
${PROJECT_SOURCE_DIR}/../IPCheck/
${PROJECT_SOURCE_DIR}/../DHCPCheck/
${PROJECT_SOURCE_DIR}/../DNSCheck/
${PROJECT_SOURCE_DIR}/../HostCheck/
#${PROJECT_SOURCE_DIR}/../ProxyCheck/
${PROJECT_SOURCE_DIR}/../NetCheck/
${PROJECT_SOURCE_DIR}/../../../kom/
${TOP_DIR}/plugins/common/
)

file(GLOB source_SRC
    "src/*.cpp"
    "include/*.h"
    "src/*.ui"
    "../customWidget/*.cpp"
    "../customWidget/*.h"
    "../customWidget/*.ui"
    "../Observer_Notifier/*.cpp"
    "../Observer_Notifier/*.h"
    "${TOP_DIR}/plugins/common/frame.cpp"
    "${TOP_DIR}/interface/kom_application_interface.cpp")

SET(ui_RESOURCE ${PROJECT_SOURCE_DIR}/pictures.qrc)

add_library(${PROJECT_NAME} SHARED)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,/usr/lib/kylin-os-manager/fault-detection/")

find_package(PkgConfig)

pkg_check_modules(KYSDK_SOUND kysdk-soundeffects)
if (${KYSDK_SOUND_FOUND})
    add_definitions(-DHAS_SOUNDEFFECTS)
    target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_SOUND_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_SOUND_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_SOUND_LIBRARIES})
endif()

pkg_check_modules(KYSDK_MODULE kysdk-kabase kysdk-ukenv kysdk-waylandhelper kysdk-qtwidgets)

target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARIES})

target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/interface)
target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/3rdparty/kyplugin)

link_directories(${PROJECT_SOURCE_DIR}/../plugins)
link_directories(${PROJECT_SOURCE_DIR}/../lib)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::X11Extras Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus Qt${QT_VERSION_MAJOR}::Concurrent)
target_link_libraries(${PROJECT_NAME} PRIVATE KF5::WindowSystem)

target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::QGSETTINGS)
target_link_libraries(${PROJECT_NAME} PRIVATE  -lX11)
target_link_libraries(${PROJECT_NAME} PRIVATE  ${UKUI_LOG4QT})

target_link_libraries(${PROJECT_NAME} PRIVATE libBase)
target_link_libraries(${PROJECT_NAME} PRIVATE HWCheck)
target_link_libraries(${PROJECT_NAME} PRIVATE IPCheck)
target_link_libraries(${PROJECT_NAME} PRIVATE DHCPCheck)
target_link_libraries(${PROJECT_NAME} PRIVATE DNSCheck)
target_link_libraries(${PROJECT_NAME} PRIVATE HostCheck)
#target_link_libraries(${PROJECT_NAME} PRIVATE ProxyCheck)
target_link_libraries(${PROJECT_NAME} PRIVATE NetCheck)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)

set(TRANSLATIONS
	"${NETWORK_CHECK_DIR}/translations/kylin-netcheck-tools_zh_CN.ts"
	"${NETWORK_CHECK_DIR}/translations/kylin-netcheck-tools_bo_CN.ts"
	"${NETWORK_CHECK_DIR}/translations/kylin-netcheck-tools_mn.ts"
	"${NETWORK_CHECK_DIR}/translations/kylin-netcheck-tools_ug.ts"
	"${NETWORK_CHECK_DIR}/translations/kylin-netcheck-tools_kk.ts"
	"${NETWORK_CHECK_DIR}/translations/kylin-netcheck-tools_ky.ts"
	"${NETWORK_CHECK_DIR}/translations/kylin-netcheck-tools_zh_HK.ts"
	"${NETWORK_CHECK_DIR}/translations/kylin-netcheck-tools_de.ts"
	"${NETWORK_CHECK_DIR}/translations/kylin-netcheck-tools_es.ts"
	"${NETWORK_CHECK_DIR}/translations/kylin-netcheck-tools_fr.ts")
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

target_sources(${PROJECT_NAME} PRIVATE ${source_SRC} ${ui_RESOURCE} ${QM_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/kylin-os-manager/plugins/)
install(FILES ${PROJECT_SOURCE_DIR}/data/org.kylin.nw.check.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
install(FILES ${PROJECT_SOURCE_DIR}/config/kylin-netcheck-tools.conf DESTINATION /etc/kylin-os-manager/net-check/)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-os-manager/network-check/translations/)
