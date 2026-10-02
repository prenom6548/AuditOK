# adb
set(COMMON_ADB_SOURCES
        adb/adbprocess.h
        adb/adbprocess.cpp
        )
source_group(adb FILES ${COMMON_ADB_SOURCES})

set(COMMON_UIBASE_SOURCES
        uibase/tipwidget.h
        uibase/tipwidget.cpp
        uibase/loadanimation.h
        uibase/loadanimation.cpp
        )
source_group(uibase FILES ${COMMON_UIBASE_SOURCES})

set(COMMON_SOURCES
        config.h
        config.cpp
        fileinfo.h
        fileinfo.cpp
        fileinfolist.h
        fileinfolist.cpp
        generatetools.h
        generatetools.cpp
        highlight-effect.h
        publicattributes.hpp
        serverstatus.h
        serverstatus.cpp
        tablemodelistener.cpp
        tablemodelistener.h
        util.h
        util.cpp)

set(COMMON
        ${COMMON_ADB_SOURCES}
        ${COMMON_UIBASE_SOURCES}
        ${COMMON_SOURCES}
)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

add_library(common ${COMMON})

find_package(Qt5DBus COMPONENTS Dbus REQUIRED)
find_package(PkgConfig REQUIRED) 

pkg_check_modules(KYSDK_DIAGNOSTICS kysdk-diagnostics)
target_include_directories(common PUBLIC ${KYSDK_DIAGNOSTICS_INCLUDE_DIRS})
target_link_directories(common PUBLIC ${KYSDK_DIAGNOSTICS_LIBRARY_DIRS})
target_link_libraries(common PUBLIC ${KYSDK_DIAGNOSTICS_LIBRARIES})

pkg_check_modules(UUID uuid)
target_include_directories(common PUBLIC ${UUID_INCLUDE_DIRS})
target_link_directories(common PUBLIC ${UUID_LIBRARY_DIRS})
target_link_libraries(common PUBLIC ${UUID_LIBRARIES})

target_include_directories(common PUBLIC adb)
target_include_directories(common PUBLIC uibase)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(common PUBLIC ${TOP_DIR}/../kabase/)

target_link_libraries(common PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        ${Qt5DBus_LIBRARIES}
        qrencode
        )

