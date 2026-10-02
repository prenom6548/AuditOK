set(UI_BASE_WIDGET
        basewidget/loaddialog.h
        basewidget/loaddialog.cpp
        basewidget/transmissiondialog.h
        basewidget/transmissiondialog.cpp
        basewidget/backgroundwin.cpp
        basewidget/backgroundwin.h
        basewidget/levitationbutton.cpp
        basewidget/levitationbutton.h
        basewidget/messagedialog.h
        basewidget/messagedialog.cpp
        basewidget/disconnectbutton.h
        basewidget/disconnectbutton.cpp
        basewidget/tipdisplaywin.h
        basewidget/tipdisplaywin.cpp
        )
source_group(basewidget FILES ${UI_BASE_WIDGET})

set(UI_FILEMANAGEVIEW_SOURCES
        filemanageview/filemanagewin.h
        filemanageview/filemanagewin.cpp
        filemanageview/multimediawin.h
        filemanageview/multimediawin.cpp
        filemanageview/androiditem.h
        filemanageview/androiditem.cpp
        filemanageview/clicklabel.h
        filemanageview/clicklabel.cpp
        filemanageview/androidhomepage.h
        filemanageview/androidhomepage.cpp
        filemanageview/fileviewdelegate.h 
        filemanageview/fileviewdelegate.cpp
        filemanageview/fileview.h
        filemanageview/fileview.cpp
        )
source_group(filemanageview FILES ${UI_FILEMANAGEVIEW_SOURCES})

set(UI_INITCONNECTWIN_SOURCES
        connectinterface/mobileconnectwin.h
        connectinterface/mobileconnectwin.cpp
        connectinterface/mobileqrcode.h
        connectinterface/mobileqrcode.cpp
        connectinterface/usbconnectwin.h
        connectinterface/usbconnectwin.cpp
        connectinterface/searchdevicewin.h
        connectinterface/searchdevicewin.cpp
        connectinterface/searchdeviceitem.h
        connectinterface/searchdeviceitem.cpp
        connectinterface/connectinterface.h
        connectinterface/connectinterface.cpp
        connectinterface/devicecodewidget.h
        connectinterface/devicecodewidget.cpp
        connectinterface/devicecodeitem.cpp
        connectinterface/devicecodeitem.h
        )
source_group(connectinterface FILES ${UI_INITCONNECTWIN_SOURCES})

set(UI_VIEW_SOURCES
        view/connectedwin.h
        view/connectedwin.cpp
        view/homepage.h
        view/homepage.cpp
        view/titlebar.h
        view/titlebar.cpp
        )
source_group(view FILES ${UI_VIEW_SOURCES})

set(UI_SOURCES
        ${UI_BASE_WIDGET}
        ${UI_CLASSIFICATIONLIST_SOURCES}
        ${UI_INITCONNECTWIN_SOURCES}
        ${UI_STORAGELIST_SOURCES}
        ${UI_VIEW_SOURCES}
        ${UI_FILEMANAGEVIEW_SOURCES}
        mainwindow.h
        mainwindow.cpp)

add_library(ui ${UI_SOURCES})

target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/basewidget)
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/connectinterface)
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/view)
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/filemanageview)
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt5DBus COMPONENTS Dbus REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(KYSDK_QTWIDGETS kysdk-qtwidgets)
target_include_directories(ui PUBLIC ${KYSDK_QTWIDGETS_INCLUDE_DIRS})
target_link_directories(ui PUBLIC ${KYSDK_QTWIDGETS_LIBRARY_DIRS})
target_link_libraries(ui PUBLIC ${KYSDK_QTWIDGETS_LIBRARIES})

pkg_check_modules(KYSDK_UKENV kysdk-ukenv)
target_include_directories(ui PUBLIC ${KYSDK_UKENV_INCLUDE_DIRS})
target_link_directories(ui PUBLIC ${KYSDK_UKENV_LIBRARY_DIRS})
target_link_libraries(ui PUBLIC ${KYSDK_UKENV_LIBRARIES})

pkg_check_modules(GSETTINGS gsettings-qt)
target_include_directories(ui PUBLIC ${GSETTINGS_INCLUDE_DIRS})
target_link_directories(ui PUBLIC ${GSETTINGS_LIBRARY_DIRS})
target_link_libraries(ui PUBLIC ${GSETTINGS_LIBRARIES})

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(ui PUBLIC ${TOP_DIR}/../kabase/Qt)
target_include_directories(kylin-connectivity PUBLIC ${TOP_DIR}/3rdparty/ftpserver/fineftp-server/include/)

target_link_libraries(ui PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        ${Qt5DBus_LIBRARIES}
        projection
        ffmpegthumbnailer
        connectmanage
        filetransfer
        filesync
        filesearch
        pcscreenmanage
        common
        discovery
        fineftp::server
        )




