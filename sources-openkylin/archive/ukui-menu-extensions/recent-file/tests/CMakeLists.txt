set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Test DBus Qml Widgets Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Test DBus Qml Widgets Quick REQUIRED)

if (QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5Xdg REQUIRED)
elseif(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6Xdg REQUIRED)
endif()

include_directories(../)

set(TEST_RUNTIME_OUTPUT_DIR "${CMAKE_BINARY_DIR}/recent-file/tests")

add_executable(recentFileTest
    ../recent-file-extension.h
    ../recent-file-extension.cpp
    ../event-track.cpp
    recent-file-test.cpp)

add_executable(eventTrackTest
    ../event-track.cpp
    event-track-test.cpp)

set_target_properties(recentFileTest eventTrackTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${TEST_RUNTIME_OUTPUT_DIR}"
)

target_link_options(eventTrackTest PRIVATE
        -Wl,--wrap=kdk_dia_data_init
        -Wl,--wrap=kdk_dia_upload_default
        -Wl,--wrap=kdk_dia_upload_search_content
        -Wl,--wrap=kdk_dia_data_free)

target_link_options(recentFileTest PRIVATE
        -Wl,--wrap=g_file_monitor_directory
        -Wl,--wrap=g_file_new_for_uri
        -Wl,--wrap=g_signal_connect_data
        -Wl,--wrap=g_file_enumerate_children_async
        -Wl,--wrap=g_file_enumerate_children_finish
        -Wl,--wrap=g_file_enumerator_next_files_async
        -Wl,--wrap=g_file_enumerator_next_files_finish
        -Wl,--wrap=g_file_enumerator_get_child
        -Wl,--wrap=g_app_info_get_default_for_uri_scheme
        -Wl,--wrap=g_file_query_default_handler
        -Wl,--wrap=g_file_query_info
        -Wl,--wrap=g_app_info_get_default_for_type
        -Wl,--wrap=g_desktop_app_info_new
        -Wl,--wrap=g_app_info_launch
        -Wl,--wrap=g_file_delete)

find_package(ukui-menu REQUIRED)

find_package(PkgConfig REQUIRED)
set(EXTERNAL_LIBS "")
set(PC_PKGS glib-2.0 gio-unix-2.0 kysdk-datacollect kysdk-systime)

foreach(lib IN ITEMS ${PC_PKGS})
    pkg_check_modules(${lib} REQUIRED ${lib})
    if(${${lib}_FOUND})
        include_directories(${${lib}_INCLUDE_DIRS})
        link_directories(${${lib}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS ${${lib}_LIBRARIES})
    endif()
endforeach()

target_link_libraries(recentFileTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}Xdg
        ${EXTERNAL_LIBS}
        ukui-menu
)

target_link_libraries(eventTrackTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ${EXTERNAL_LIBS}
)

add_test(NAME recentFileTest COMMAND ${CMAKE_CURRENT_BINARY_DIR}/recentFileTest)
add_test(NAME eventTrackTest COMMAND ${CMAKE_CURRENT_BINARY_DIR}/eventTrackTest)
