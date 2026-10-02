
set(CURTAIN_SOURCES
        curtain/curtain.h
        curtain/curtain.cpp
        )
source_group(curtain FILES ${CURTAIN_SOURCES})

set(PCSCREEN_SOURCES
        pcscreen/pcscreen.h
        pcscreen/pcscreen.cpp
        )
source_group(pcscreen FILES ${PCSCREEN_SOURCES})

set(CONTROL_SOURCES
        control/control.h
        control/control.cpp
        )
source_group(control FILES ${CONTROL_SOURCES})

set(SOURCES
        ${CURTAIN_SOURCES}
        ${PCSCREEN_SOURCES}
        ${CONTROL_SOURCES}
        pcscreenmanage.h
        pcscreenmanage.cpp
        suspendtabbar.cpp
        suspendtabbar.h
        screenmanagemsg.pb.h
        screenmanagemsg.pb.cc
        )
add_library(pcscreenmanage ${SOURCES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(KYSDK_QTWIDGETS kysdk-qtwidgets)
target_include_directories(pcscreenmanage PUBLIC ${KYSDK_QTWIDGETS_INCLUDE_DIRS})
target_link_directories(pcscreenmanage PUBLIC ${KYSDK_QTWIDGETS_LIBRARY_DIRS})
target_link_libraries(pcscreenmanage PUBLIC ${KYSDK_QTWIDGETS_LIBRARIES})

target_include_directories(pcscreenmanage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/curtain)
target_include_directories(pcscreenmanage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pcscreen)
target_include_directories(pcscreenmanage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/control)
target_include_directories(pcscreenmanage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

pkg_check_modules(QWEBENGINEWIDGETS REQUIRED Qt5WebEngineWidgets)
include_directories(${QWEBENGINEWIDGETS_INCLUDE_DIRS})
link_directories(${QWEBENGINEWIDGETS_LIBRARY_DIRS})

target_link_libraries(pcscreenmanage PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        ${QWEBENGINEWIDGETS_LIBRARIES}
        common
        )




