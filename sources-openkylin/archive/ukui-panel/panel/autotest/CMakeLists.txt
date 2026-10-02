cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(ukui-panel-autotest LANGUAGES CXX)
    enable_testing()

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Test Gui RemoteObjects REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED COMPONENTS Core Test Gui RemoteObjects)
find_package(PkgConfig REQUIRED)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

if (QT_VERSION_MAJOR EQUAL "5")
    set(AUTOTEST_GSETTINGS_QT gsettings-qt)
elseif (QT_VERSION_MAJOR EQUAL "6")
    set(AUTOTEST_GSETTINGS_QT gsettings-qt6)
endif()

set(PANEL_AUTOTEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PANEL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")

include_directories(${PANEL_AUTOTEST_SOURCE_DIR} ${PANEL_SOURCE_DIR})

add_executable(screenManagerTest
        screen-manager-test.cpp
        ../src/screens-manager.cpp)

target_link_libraries(screenManagerTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Gui
        )
add_test(NAME screenManagerTest COMMAND screenManagerTest)

pkg_check_modules(KYSDK_DATACOLLECT REQUIRED IMPORTED_TARGET kysdk-datacollect)
find_program(QT_MOC_EXECUTABLE NAMES moc-qt6 moc-qt5 moc REQUIRED)
find_program(REPC_EXECUTABLE NAMES repc REQUIRED)

set(DATA_COLLECTOR_MOC ${CMAKE_CURRENT_BINARY_DIR}/data-collector.moc)
add_custom_command(
        OUTPUT ${DATA_COLLECTOR_MOC}
        COMMAND ${QT_MOC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../src/data-collector.cpp -o ${DATA_COLLECTOR_MOC}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../src/data-collector.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/data-collector.h
)
add_custom_target(dataCollectorMoc DEPENDS ${DATA_COLLECTOR_MOC})

add_executable(dataCollectorTest
        data-collector-test.cpp
        data-collector-test.h)
add_dependencies(dataCollectorTest dataCollectorMoc)
target_include_directories(dataCollectorTest PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(dataCollectorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        PkgConfig::KYSDK_DATACOLLECT
        )
add_test(NAME dataCollectorTest COMMAND dataCollectorTest)
set_tests_properties(dataCollectorTest PROPERTIES ENVIRONMENT "HOME=/tmp;XDG_RUNTIME_DIR=/tmp")

add_executable(logUtilsTest
        log-utils-test.cpp
        log-utils-test.h
        ../src/log-utils.cpp)
target_link_libraries(logUtilsTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        )
add_test(NAME logUtilsTest COMMAND logUtilsTest)
set_tests_properties(logUtilsTest PROPERTIES ENVIRONMENT "HOME=/tmp;XDG_RUNTIME_DIR=/tmp")

pkg_check_modules(PANEL_GSETTINGS_QT REQUIRED IMPORTED_TARGET ${AUTOTEST_GSETTINGS_QT})
pkg_check_modules(PANEL_GSETTINGS_GIO REQUIRED IMPORTED_TARGET gio-2.0)
find_program(GLIB_COMPILE_SCHEMAS_EXECUTABLE NAMES glib-compile-schemas REQUIRED)

set(PANEL_GSETTINGS_SCHEMA_DIR ${CMAKE_CURRENT_BINARY_DIR}/panel-gsettings-schemas)
set(PANEL_GSETTINGS_SCHEMA_XML ${CMAKE_CURRENT_SOURCE_DIR}/../data/org.ukui.panel.settings.gschema.xml)
set(PANEL_GSETTINGS_SCHEMA_COMPILED ${PANEL_GSETTINGS_SCHEMA_DIR}/gschemas.compiled)

add_custom_command(
        OUTPUT ${PANEL_GSETTINGS_SCHEMA_COMPILED}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PANEL_GSETTINGS_SCHEMA_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PANEL_GSETTINGS_SCHEMA_XML} ${PANEL_GSETTINGS_SCHEMA_DIR}/org.ukui.panel.settings.gschema.xml
        COMMAND ${GLIB_COMPILE_SCHEMAS_EXECUTABLE} ${PANEL_GSETTINGS_SCHEMA_DIR}
        DEPENDS ${PANEL_GSETTINGS_SCHEMA_XML}
)
add_custom_target(panelGSettingsSchema DEPENDS ${PANEL_GSETTINGS_SCHEMA_COMPILED})

add_executable(panelGSettingsTest
        panel-gsettings-test.cpp
        panel-gsettings-test.h
        ../src/panel-gsettings.cpp)
add_dependencies(panelGSettingsTest panelGSettingsSchema)
target_compile_definitions(panelGSettingsTest PRIVATE
        PANEL_GSETTINGS_TEST_SCHEMA_DIR="${PANEL_GSETTINGS_SCHEMA_DIR}")
target_link_libraries(panelGSettingsTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        PkgConfig::PANEL_GSETTINGS_QT
        PkgConfig::PANEL_GSETTINGS_GIO
        )
add_test(NAME panelGSettingsTest COMMAND panelGSettingsTest)
set_tests_properties(panelGSettingsTest PROPERTIES ENVIRONMENT "HOME=/tmp;XDG_RUNTIME_DIR=/tmp")

set(REMOTE_CONFIG_REPLICA ${CMAKE_CURRENT_BINARY_DIR}/rep_remote-config_replica.h)
set(REMOTE_CONFIG_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/rep_remote-config_source.h)
add_custom_command(
        OUTPUT ${REMOTE_CONFIG_REPLICA} ${REMOTE_CONFIG_SOURCE}
        COMMAND ${REPC_EXECUTABLE} -o replica ${CMAKE_CURRENT_SOURCE_DIR}/../src/remote-config.rep ${REMOTE_CONFIG_REPLICA}
        COMMAND ${REPC_EXECUTABLE} -o source ${CMAKE_CURRENT_SOURCE_DIR}/../src/remote-config.rep ${REMOTE_CONFIG_SOURCE}
        COMMAND sed -i s/REP_REMOTE-CONFIG/REP_REMOTE_CONFIG/g ${REMOTE_CONFIG_REPLICA} ${REMOTE_CONFIG_SOURCE}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../src/remote-config.rep ${CMAKE_CURRENT_SOURCE_DIR}/../src/general-config-define.h
)

add_executable(messageProcessorTest
        message-processor-test.cpp
        message-processor-test.h
        ../src/message-processor.cpp
        ../src/general-config-define.h
        ${REMOTE_CONFIG_REPLICA}
        ${REMOTE_CONFIG_SOURCE})
target_include_directories(messageProcessorTest PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(messageProcessorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::RemoteObjects
        )
add_test(NAME messageProcessorTest COMMAND messageProcessorTest)
set_tests_properties(messageProcessorTest PROPERTIES ENVIRONMENT "HOME=/tmp;XDG_RUNTIME_DIR=/tmp")

add_executable(windowSuppressionStateTest
        window-suppression-state-test.cpp
        window-suppression-state-test.h)
target_link_libraries(windowSuppressionStateTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        )
add_test(NAME windowSuppressionStateTest COMMAND windowSuppressionStateTest)

add_custom_target(unit-tests
        DEPENDS
        screenManagerTest
        dataCollectorTest
        logUtilsTest
        panelGSettingsTest
        messageProcessorTest
        windowSuppressionStateTest)
