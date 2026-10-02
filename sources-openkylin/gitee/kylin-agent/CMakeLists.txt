cmake_minimum_required(VERSION 3.10)
project(KylinAgent VERSION 0.9.6 LANGUAGES CXX)
set(KYLIN_AGENT_RUNTIME_VERSION "0.9.9")
set(KYLIN_AGENT_CUA_VERSION "0.9.7")
set(BUILD_VERSION "0.9.6")
set(APP_EXECUTABLE_NAME kylin-agent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt5 configuration
find_package(Qt5 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets 
    Sql 
    Network 
    Xml
    Svg
    WebSockets
    Multimedia
)
find_package(ZLIB REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(KYSDKQTWIDGETS REQUIRED kysdk-qtwidgets)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/ui)
include_directories(${PROJECT_SOURCE_DIR}/include/models)
include_directories(${PROJECT_SOURCE_DIR}/include/services)
include_directories(${PROJECT_SOURCE_DIR}/include/utils)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/models
    ${CMAKE_CURRENT_SOURCE_DIR}/include/services
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
)

set(SOURCES
    src/main.cpp
    src/app.cpp
    src/models/session.cpp
    src/models/chatmodel.cpp
    src/services/databaseservice.cpp
    src/services/sessionservice.cpp
    src/services/modelservice.cpp
    src/services/modeldownloadservice.cpp
    src/services/cuamodelconfigsync.cpp
    src/services/realtimeasrservice.cpp
    src/services/apiservice.cpp
    src/services/configservice.cpp
    src/services/gatewayservice.cpp
    src/services/hermesgatewayservice.cpp
    src/services/controlservice.cpp
    src/ui/mainwindow.cpp
    src/ui/sessionlistwidget.cpp
    src/ui/chatwidget.cpp
    src/ui/modelsettingsdialog.cpp
    src/ui/sidebar.cpp
    src/ui/settingsdialog.cpp
    src/ui/modelsettingswidget.cpp
    src/ui/cuamodelsettingswidget.cpp
    src/ui/voicemodelsettingswidget.cpp
    src/ui/skillssettingswidget.cpp
    src/ui/personasettingswidget.cpp
    src/ui/memorysettingswidget.cpp
    src/ui/logsettingswidget.cpp
    src/ui/usagesettingswidget.cpp
    src/ui/generalsettingswidget.cpp
    src/utils/svgicon.cpp
    src/utils/thememanager.cpp
    src/utils/kylinfiledialog.cpp
)

set(HEADERS
    include/app.h
    include/models/session.h
    include/models/chatmodel.h
    include/services/databaseservice.h
    include/services/sessionservice.h
    include/services/modelservice.h
    include/services/modeldownloadservice.h
    include/services/cuamodelconfigsync.h
    include/services/realtimeasrservice.h
    include/services/apiservice.h
    include/services/configservice.h
    include/services/gatewayservice.h
    include/services/hermesgatewayservice.h
    include/services/controlservice.h
    include/ui/mainwindow.h
    include/ui/sessionlistwidget.h
    include/ui/chatwidget.h
    include/ui/modelsettingsdialog.h
    include/ui/sidebar.h
    include/ui/settingsdialog.h
    include/ui/modelsettingswidget.h
    include/ui/cuamodelsettingswidget.h
    include/ui/voicemodelsettingswidget.h
    include/ui/skillssettingswidget.h
    include/ui/personasettingswidget.h
    include/ui/memorysettingswidget.h
    include/ui/logsettingswidget.h
    include/ui/usagesettingswidget.h
    include/ui/generalsettingswidget.h
    include/utils/svgicon.h
    include/utils/thememanager.h
    include/utils/kylinfiledialog.h
    include/utils/kylininputdialog.h
)

qt_add_resources(SOURCES res/res.qrc) # 将.qrc生成的源文件添加到SOURCES变量


add_executable(${APP_EXECUTABLE_NAME}
    ${SOURCES}
    ${HEADERS}
)

target_compile_definitions(${APP_EXECUTABLE_NAME} PRIVATE
    KYLIN_AGENT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    KYLIN_AGENT_VERSION="${PROJECT_VERSION}"
    KYLIN_AGENT_BUILD_VERSION="${BUILD_VERSION}"
    KYLIN_AGENT_RUNTIME_VERSION="${KYLIN_AGENT_RUNTIME_VERSION}"
    KYLIN_AGENT_CUA_VERSION="${KYLIN_AGENT_CUA_VERSION}"
)

target_include_directories(${APP_EXECUTABLE_NAME} PRIVATE ${KYSDKQTWIDGETS_INCLUDE_DIRS})
target_link_directories(${APP_EXECUTABLE_NAME} PRIVATE ${KYSDKQTWIDGETS_LIBRARY_DIRS})

target_link_libraries(${APP_EXECUTABLE_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Network
    Qt5::Xml
    Qt5::Svg
    Qt5::WebSockets
    Qt5::Multimedia
    ZLIB::ZLIB
    ${KYSDKQTWIDGETS_LIBRARIES}
)

# Install targets
install(TARGETS ${APP_EXECUTABLE_NAME} DESTINATION /usr/bin)
install(FILES packaging/kylin-agent.desktop DESTINATION /usr/share/applications)
install(PROGRAMS scripts/agent_runtime_install.sh
                 scripts/kylin_agent_ctl.py
                 scripts/ydotool_install.sh
                 scripts/seed_runtime_skills.sh
                 scripts/repair_hermes_home.sh
                 scripts/install_runtime_cache.sh
        DESTINATION /usr/local/share/kylin-agent/scripts)
install(FILES res/ai-agent.png
        DESTINATION /usr/share/icons/hicolor/512x512/apps
        RENAME kylin-agent.png)
