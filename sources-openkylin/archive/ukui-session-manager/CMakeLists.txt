cmake_minimum_required(VERSION 3.10)
# KF5 *QchTargets.cmake still sets IMPORTED on custom targets.
# CMP0160 (CMake 3.29+) makes that property read-only and breaks find_package.
if(POLICY CMP0160)
    cmake_policy(SET CMP0160 OLD)
endif()
project(ukui-session-manager)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Handle the Qt rcc code generator automatically.
set(CMAKE_AUTORCC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INSTALL_PREFIX /usr)

include(GNUInstallDirs)

# Enable funcion names and line numbers even for release builds
add_definitions("-DQT_MESSAGELOGCONTEXT")

# Find the QtWidgets library
find_package(Qt5 COMPONENTS Widgets Quick Core Multimedia X11Extras REQUIRED)
find_package(Qt5LinguistTools)
find_package(KF5IdleTime)
find_package(Qt5DBus)
find_package(KF5CoreAddons)
find_package(KF5Config)
find_package(KF5WindowSystem)

# For debug: cmake -DCMAKE_BUILD_TYPE=debug
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

# i18n
set(UKUI_TRANSLATIONS_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/ukui/translations)
add_definitions(
    -DUKUI_TRANSLATIONS_DIR="${UKUI_TRANSLATIONS_DIR}"
)
if (NOT DEFINED UPDATE_TRANSLATIONS)
    set(UPDATE_TRANSLATIONS "No")
endif()

# To create a new ts file: lupdate -recursive . -target-language en_US -ts translations/en_US.ts -no-ui-lines
# update ts file: lupdate -recursive . -target-language  translations/bo_CN.ts -ts translations/bo_CN.ts -no-ui-lines
file(GLOB TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")

# cmake -DUPDATE_TRANSLATIONS=yes
if (UPDATE_TRANSLATIONS)
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()
add_custom_target(translations ALL DEPENDS ${QM_FILES})
install(FILES ${QM_FILES} DESTINATION ${UKUI_TRANSLATIONS_DIR}/${PROJECT_NAME})

add_subdirectory(ukui-session)
add_subdirectory(tools)
add_subdirectory(ukuismserver)
add_subdirectory(data)
#add_subdirectory(tests)
