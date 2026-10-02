if (QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5Xdg REQUIRED)
else ()
    find_package(Qt6Xdg REQUIRED)
endif()

set(statusNotifierHostTest_SRCS
    status-notifier-model.cpp status-notifier-model.h
    item-icon-provider.cpp item-icon-provider.h
    icon-item.cpp icon-item.h
    tray-app.cpp tray-app.h
#    setting.cpp setting.h
        main.cpp )
set(QRC_FILES qml.qrc)

find_package(PkgConfig REQUIRED)
set(UKUI_SNI_EXTERNAL_LIBS "")
if (QT_VERSION_MAJOR EQUAL "5")
    set(UKUI_SNI_PC_PKGS gsettings-qt)
elseif (QT_VERSION_MAJOR EQUAL "6")
    set(UKUI_SNI_PC_PKGS gsettings-qt6)
endif()
foreach(external_lib IN ITEMS ${UKUI_SNI_PC_PKGS})
        pkg_check_modules(${external_lib} REQUIRED ${external_lib})
        if(${${external_lib}_FOUND})
                include_directories(${${external_lib}_INCLUDE_DIRS})
                link_directories(${${external_lib}_LIBRARY_DIRS})
                list(APPEND UKUI_SNI_EXTERNAL_LIBS ${${external_lib}_LIBRARIES})
        endif()
endforeach()
message(STATUS "External libraries found: ${UKUI_SNI_EXTERNAL_LIBS}")

include_directories(${CMAKE_SOURCE_DIR}/status-notifier-host)
message(${CMAKE_SOURCE_DIR})

if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(statusNotifierHostTest_SRCS dbus/org.kde.TrayAppStatusNotifierItem.xml tray-app.h TrayApp)
else()
    qt5_add_dbus_adaptor(statusNotifierHostTest_SRCS dbus/org.kde.TrayAppStatusNotifierItem.xml tray-app.h TrayApp)
endif()

qt_add_dbus_interface(statusNotifierHostTest_SRCS ../dbus/org.kde.StatusNotifierWatcher.xml statusnotifierwatcher_interface)

add_executable(ukui-sni-test ${statusNotifierHostTest_SRCS} ${QRC_FILES})

target_link_libraries(ukui-sni-test PRIVATE
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_MAJOR_VERSION}::Quick
        Qt${QT_MAJOR_VERSION}Xdg
        ukui-sni-host
        )
