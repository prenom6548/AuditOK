project(app_launcher)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g")

set(${TOP_DIR} ${CMAKE_CURRENT_LIST_DIR}/../../../)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets DBus REQUIRED)
find_package(KF5WindowSystem)

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dbus_app_exit.cpp)

set(COMPONENT
    ${TOP_DIR}/plugins/fault_diagnosis/component/text_label.cpp
    ${TOP_DIR}/plugins/fault_diagnosis/component/utils.cpp
    ${TOP_DIR}/plugins/fault_diagnosis/component/user_experience_dialog.cpp)

include_directories(${TOP_DIR}/kom/)
include_directories(${TOP_DIR}/plugins/fault_diagnosis/component/)
include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT_NAME} ${SRCS} ${COMPONENT} ${CMAKE_CURRENT_LIST_DIR}/resource.qrc)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::DBus)
target_link_libraries(${PROJECT_NAME} PRIVATE KF5::WindowSystem)

pkg_check_modules(KYSDK_QTWIDGETS kysdk-qtwidgets)
if (KYSDK_QTWIDGETS_FOUND)
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

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/share/kylin-os-manager/plugins/tool-box/bin/)
