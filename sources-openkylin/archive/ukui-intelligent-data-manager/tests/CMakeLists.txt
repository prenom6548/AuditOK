cmake_minimum_required(VERSION 3.5)
project(tests VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Test DBus Sql REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Test DBus Sql REQUIRED)

find_package(PkgConfig)
pkg_check_modules(PEONY REQUIRED peony)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(UKUI_FILE_METADATA ukui-file-metadata)

include_directories(
    ../
)

remove_definitions(-DCOMPLIE_AS_PROJECT)

set(PKG_CONFIG_LIBS
    ${PEONY_LIBRARIES}
    ${GIO_LIBRARIES}
    ${UKUI_FILE_METADATA_LIBRARIES}
)

set(TESTS_INCLUDE_VAR
    ../file/
    ${PEONY_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${UKUI_FILE_METADATA_INCLUDE_DIRS}
)

message(${TESTS_INCLUDE_VAR})

set(TESTS_LIBS_VAR
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::DBus
    ${PKG_CONFIG_LIBS}
    ukui-idm-fileinfo
)

message(${TESTS_LIBS_VAR})

add_executable(fileinfotests
    fileinfotest.cpp
)
target_include_directories(fileinfotests PRIVATE
    ${TESTS_INCLUDE_VAR}
)
target_link_libraries(fileinfotests PRIVATE
    ${TESTS_LIBS_VAR}
)

add_executable(fileinfomanagertests
    fileinfomanagertests.cpp
)

target_include_directories(fileinfomanagertests PRIVATE
    ${TESTS_INCLUDE_VAR}
)
target_link_libraries(fileinfomanagertests PRIVATE
    ${TESTS_LIBS_VAR}
)

add_executable(databasetests
    databasetest.cpp
    ../database/databasemanager.cpp
)

target_include_directories(databasetests PRIVATE
    ../database/
    ${TESTS_INCLUDE_VAR}
)
target_link_libraries(databasetests PRIVATE
    ${TESTS_LIBS_VAR}
    Qt${QT_VERSION_MAJOR}::Sql
)

add_executable(modeltests
    modeltest.cpp
    ../model/fileitemmodel.cpp
    ../model/fileitemproxysortfiltermodel.cpp
    ../model/modelcontroller.cpp
    ../model/spacesfilesfetcher.cpp
    ../model/spacessettings.cpp
    ../database/databasemanager.cpp
)

target_include_directories(modeltests PRIVATE
    ../model/
    ../database/
    ../file/
    ${TESTS_INCLUDE_VAR}
)

target_link_libraries(modeltests PRIVATE
    ${TESTS_LIBS_VAR}
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Widgets
)

add_executable(thumbnailtests
    thumbnailtest.cpp
    ../thumbnail/thumbnailmanager.cpp
)

target_link_directories(thumbnailtests PRIVATE
    ../thumbnail/
    ${TESTS_INCLUDE_VAR}
)

target_link_libraries(thumbnailtests PRIVATE
    ${TESTS_LIBS_VAR}
)

add_test(NAME fileinfotests COMMAND fileinfotests)
add_test(NAME fileinfomanagertests COMMAND fileinfomanagertests)
add_test(NAME databasetests COMMAND databasetests)
add_test(NAME modeltests COMMAND modeltests)
add_test(NAME thumbnailtests COMMAND thumbnailtests)
