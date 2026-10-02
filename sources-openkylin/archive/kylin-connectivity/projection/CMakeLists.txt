cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(projection)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Network LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network LinguistTools REQUIRED)

# common
set(PROJECTION_DEVICECOMMON_SOURCES
        devicecommon/qscrcpyevent.h
        )
source_group(devicecommon FILES ${PROJECTION_DEVICECOMMON_SOURCES})

# device
set(PROJECTION_DEVICE_SOURCES
        device/baseserver.h
        device/baseserver.cpp
        device/basedevice.h
        device/basedevice.cpp
        device/android/input.h
        device/android/keycodes.h
        device/controller/controller.h
        device/controller/controller.cpp
        device/controller/inputconvert/inputconvertbase.h
        device/controller/inputconvert/inputconvertbase.cpp
        device/controller/inputconvert/inputconvertnormal.h
        device/controller/inputconvert/inputconvertnormal.cpp
        device/controller/inputconvert/inputconvertgame.h
        device/controller/inputconvert/inputconvertgame.cpp
        device/controller/inputconvert/controlmsg.h
        device/controller/inputconvert/controlmsg.cpp
        device/controller/inputconvert/keymap/keymap.h
        device/controller/inputconvert/keymap/keymap.cpp
        device/controller/receiver/devicemsg.h
        device/controller/receiver/devicemsg.cpp
        device/controller/receiver/receiver.h
        device/controller/receiver/receiver.cpp
        device/decoder/avframeconvert.h
        device/decoder/avframeconvert.cpp
        device/decoder/decoder.h
        device/decoder/decoder.cpp
        device/decoder/fpscounter.h
        device/decoder/fpscounter.cpp
        device/decoder/videobuffer.h
        device/decoder/videobuffer.cpp
        device/filehandler/filehandler.h
        device/filehandler/filehandler.cpp
        device/recorder/recorder.h
        device/recorder/recorder.cpp
        device/render/qyuvopenglwidget.h
        device/render/qyuvopenglwidget.cpp
        device/server/inputchinese.pb.h
        device/server/inputchinese.pb.cc
        device/server/tcpserver.h
        device/server/tcpserver.cpp
        device/server/videosocket.h
        device/server/videosocket.cpp
        device/server/inputsocket.cpp
        device/server/inputsocket.h
        device/stream/stream.h
        device/stream/stream.cpp
        device/usbdevice/usbserver.h
        device/usbdevice/usbserver.cpp
        device/usbdevice/usbdevice.h
        device/usbdevice/usbdevice.cpp
        device/wifidevice/wifiserver.h
        device/wifidevice/wifiserver.cpp
        device/wifidevice/wifidevice.h
        device/wifidevice/wifidevice.cpp
        device/deviceui/toolform.h
        device/deviceui/toolform.cpp
        device/deviceui/videoform.h
        device/deviceui/videoform.cpp
        device/deviceui/videoform.ui
        device/server/inputsocket.cpp
        device/server/inputsocket.h
        )
source_group(device FILES ${PROJECTION_DEVICE_SOURCES})

# devicemanage
set(PROJECTION_DEVICEMANAGE_SOURCES
        devicemanage/devicemanage.h
        devicemanage/devicemanage.cpp
        )
source_group(devicemanage FILES ${PROJECTION_DEVICEMANAGE_SOURCES})

# fontawesome
set(PROJECTION_FONTAWESOME_SOURCES
        fontawesome/iconhelper.h
        fontawesome/iconhelper.cpp
        )
source_group(fontawesome FILES ${PROJECTION_FONTAWESOME_SOURCES})

# uibase
set(PROJECTION_UIBASE_SOURCES
        uibase/keepratiowidget.h
        uibase/keepratiowidget.cpp
        uibase/magneticwidget.h
        uibase/magneticwidget.cpp
        uibase/videotitle.h
        uibase/videotitle.cpp
        uibase/scrollsettingwidget.cpp
        uibase/scrollsettingwidget.h
        )
source_group(uibase FILES ${QC_UIBASE_SOURCES})

# util
set(PROJECTION_UTIL_SOURCES
        util/apputil.h
        util/apputil.cpp
        util/compat.h
        util/bufferutil.h
        util/bufferutil.cpp
        util/mousetap/mousetap.h
        util/mousetap/mousetap.cpp
        util/mousetap/xmousetap.h
        util/mousetap/xmousetap.cpp
        )


source_group(util FILES ${PROJECTION_UTIL_SOURCES})

set(PROJECTION
        ${PROJECTION_DEVICECOMMON_SOURCES}
        ${PROJECTION_DEVICE_SOURCES}
        ${PROJECTION_DEVICEMANAGE_SOURCES}
        ${PROJECTION_FONTAWESOME_SOURCES}
        ${PROJECTION_UIBASE_SOURCES}
        ${PROJECTION_UTIL_SOURCES} )

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

add_library(projection ${PROJECTION})


target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/devicecommon)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/filehandler)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/android)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/decoder)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/controller)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/controller/receiver)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/controller/inputconvert)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/controller/inputconvert/keymap)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/server)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/stream)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/render)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/deviceui)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/wifidevice)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/usbdevice)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/device/recorder)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/fontawesome)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/util)
target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/uibase)

target_include_directories(projection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/devicemanage)


find_package(KF5WindowSystem COMPONENTS KF5WindowSystemConfig REQUIRED)
find_package(Qt5X11Extras COMPONENTS X11Extras REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(KYSDK_QTWIDGETS kysdk-qtwidgets)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_QTWIDGETS_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_QTWIDGETS_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_QTWIDGETS_LIBRARIES})

pkg_check_modules(KYSDKKABASE_PKG kysdk-kabase)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDKKABASE_PKG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDKKABASE_PKG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDKKABASE_PKG_LIBRARIES})

pkg_check_modules(KYSDK_WAYLANDHELPER kysdk-waylandhelper)
target_include_directories(${PROJECT_NAME} PUBLIC ${KYSDK_WAYLANDHELPER_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${KYSDK_WAYLANDHELPER_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${KYSDK_WAYLANDHELPER_LIBRARIES})

target_include_directories(projection PRIVATE /usr/include/KF5/KWindowSystem)
target_include_directories(projection PRIVATE ${TOP_DIR}/../kabase/Qt)
target_link_libraries(projection PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        ${Qt5X11Extras_LIBRARIES}
        KF5::WindowSystem
        common
        xcb
        pthread
        )

find_package(PkgConfig)
if (PkgConfig_FOUND)
    pkg_check_modules(FFmpeg libavformat>=58 libavcodec>=58 libavutil>=56 libswscale>=5)
    if (FFmpeg_FOUND)
        target_link_options(${CMAKE_PROJECT_NAME} PRIVATE ${FFmpeg_LDFLAGS})
        target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE ${FFmpeg_CFLAGS})
        set(EXTERNAL_LIBS_FFMPEG ${FFmpeg_LIBRARIES})
    endif ()
endif ()

target_link_libraries(projection PRIVATE ${EXTERNAL_LIBS_FFMPEG})

pkg_check_modules(GSettings REQUIRED
        gsettings-qt
        )

target_include_directories(projection PRIVATE ${GSettings_INCLUDE_DIRS})
target_link_libraries(projection PRIVATE ${GSettings_LIBRARIES})

