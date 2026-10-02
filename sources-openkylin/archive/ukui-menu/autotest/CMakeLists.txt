set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5
        COMPONENTS Test Core Gui Quick Qml Widgets LinguistTools DBus X11Extras REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}
        COMPONENTS Test Core Gui Quick Qml Widgets LinguistTools DBus X11Extras REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(Qt5Xdg REQUIRED)
find_package(ukui-quick COMPONENTS platform REQUIRED)
find_package(PkgConfig REQUIRED)

include_directories(../)

set(TEST_FILES
        ../src/commons.h ../src/commons.cpp
        ../src/menu-dbus-service.cpp ../src/menu-dbus-service.h
        ../src/ukui-menu-application.cpp ../src/ukui-menu-application.h
        ../src/windows/menu-main-window.cpp ../src/windows/menu-main-window.h
        ../src/settings/settings.cpp ../src/settings/settings.h
        ../src/settings/user-config.cpp ../src/settings/user-config.h
        ../src/utils/power-button.cpp ../src/utils/power-button.h
        ../src/utils/app-manager.cpp ../src/utils/app-manager.h
        ../src/utils/event-track.cpp ../src/utils/event-track.h
        ../src/utils/security-function-control.cpp ../src/utils/security-function-control.h
        ../src/utils/mask-mouse-area.cpp ../src/utils/mask-mouse-area.h
        ../src/utils/user-info-item.cpp ../src/utils/user-info-item.h
        ../src/utils/user-menu.cpp ../src/utils/user-menu.h
        ../src/utils/sidebar-button-utils.cpp ../src/utils/sidebar-button-utils.h
        ../src/extension/menu-extension-plugin.cpp ../src/extension/menu-extension-plugin.h
        ../src/extension/menu-extension-loader.cpp ../src/extension/menu-extension-loader.h
        ../src/extension/widget-extension.cpp ../src/extension/widget-extension.h
        ../src/extension/context-menu-extension.cpp ../src/extension/context-menu-extension.h
        ../src/extension/context-menu-manager.cpp ../src/extension/context-menu-manager.h
        ../src/extension/widget-extension-model.cpp ../src/extension/widget-extension-model.h
        ../src/extension/widget-model.cpp ../src/extension/widget-model.h
        ../src/extension/menu/app-menu-plugin.cpp ../src/extension/menu/app-menu-plugin.h
        ../src/extension/favorite/folder-model.cpp ../src/extension/favorite/folder-model.h
        ../src/extension/favorite/favorite-filter-model.cpp ../src/extension/favorite/favorite-filter-model.h
        ../src/extension/favorite/favorite-widget.cpp ../src/extension/favorite/favorite-widget.h
        ../src/extension/favorite/favorites-config.cpp ../src/extension/favorite/favorites-config.h
        ../src/extension/favorite/app-favorite-model.cpp ../src/extension/favorite/app-favorite-model.h
        ../src/extension/favorite/favorite-context-menu.cpp ../src/extension/favorite/favorite-context-menu.h
        ../src/extension/favorite/favorite-folder-helper.cpp ../src/extension/favorite/favorite-folder-helper.h
        ../src/extension/favorite/favorite-extension-plugin.cpp ../src/extension/favorite/favorite-extension-plugin.h
        ../src/libappdata/basic-app-model.cpp ../src/libappdata/basic-app-model.h
        ../src/libappdata/basic-app-filter-model.cpp ../src/libappdata/basic-app-filter-model.h
        ../src/libappdata/app-database-interface.cpp ../src/libappdata/app-database-interface.h
        ../src/libappdata/app-category-model.cpp ../src/libappdata/app-category-model.h
        ../src/libappdata/combined-list-model.cpp ../src/libappdata/combined-list-model.h
        ../src/libappdata/recently-installed-model.cpp ../src/libappdata/recently-installed-model.h
        ../src/libappdata/app-page-backend.cpp ../src/libappdata/app-page-backend.h
        ../src/libappdata/app-list-model.cpp ../src/libappdata/app-list-model.h
        ../src/libappdata/app-list-plugin.cpp ../src/libappdata/app-list-plugin.h
        ../src/libappdata/app-search-plugin.cpp ../src/libappdata/app-search-plugin.h
        ../src/libappdata/app-category-plugin.cpp ../src/libappdata/app-category-plugin.h
        ../src/libappdata/app-group-model.cpp ../src/libappdata/app-group-model.h
)

if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(TEST_FILES ../data/org.ukui.menu.xml menu-dbus-service.h UkuiMenu::MenuDbusService)
else()
    qt5_add_dbus_adaptor(TEST_FILES ../data/org.ukui.menu.xml menu-dbus-service.h UkuiMenu::MenuDbusService)
endif()

add_executable(appCategoryModelTest
        ../src/data-entity.cpp
        ../src/libappdata/basic-app-filter-model.h
        ../src/libappdata/app-category-model.cpp
        src/libappdata/app-category-model-test.cpp)

target_link_libraries(appCategoryModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)

add_test(NAME appCategoryModelTest COMMAND $<TARGET_FILE:appCategoryModelTest>)

add_executable(appListPluginTest
        ../src/libappdata/app-list-plugin.cpp
        src/libappdata/app-list-plugin-test.cpp)
target_link_libraries(appListPluginTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME appListPluginTest COMMAND $<TARGET_FILE:appListPluginTest>)

add_executable(appGroupModelTest
        ../src/data-entity.cpp
        ../src/libappdata/app-group-model.cpp
        src/libappdata/app-group-model-test.cpp)
target_link_libraries(appGroupModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME appGroupModelTest COMMAND $<TARGET_FILE:appGroupModelTest>)

add_executable(appListModelTest
        ../src/data-entity.cpp
        ../src/libappdata/app-list-plugin.cpp
        ../src/libappdata/app-list-model.cpp
        ../src/extension/context-menu-extension.h
        ../src/extension/context-menu-manager.h
        src/libappdata/app-list-model-test.cpp)
target_link_libraries(appListModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME appListModelTest COMMAND $<TARGET_FILE:appListModelTest>)

add_executable(appCategoryPluginTest
        ../src/data-entity.cpp
        ../src/libappdata/app-list-plugin.cpp
        ../src/libappdata/combined-list-model.cpp
        ../src/libappdata/app-category-plugin.cpp
        ../src/libappdata/app-category-model.h
        ../src/libappdata/recently-installed-model.h
        ../src/utils/event-track.h
        src/libappdata/app-category-plugin-test.cpp)
target_link_libraries(appCategoryPluginTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME appCategoryPluginTest COMMAND $<TARGET_FILE:appCategoryPluginTest>)

add_executable(dataEntityTest
        ../src/data-entity.cpp
        src/data-entity-test.cpp)
target_link_libraries(dataEntityTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME dataEntityTest COMMAND $<TARGET_FILE:dataEntityTest>)

add_executable(appDatabaseInterfaceTest
        ../src/data-entity.cpp
        ../src/libappdata/app-database-interface.cpp
        src/libappdata/fakes/application-info.cpp
        src/libappdata/fakes/user-config.cpp
        src/libappdata/fakes/settings.cpp
        src/libappdata/app-database-interface-test.cpp)
target_include_directories(appDatabaseInterfaceTest PRIVATE
        src/libappdata/fakes
)
target_link_libraries(appDatabaseInterfaceTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME appDatabaseInterfaceTest COMMAND $<TARGET_FILE:appDatabaseInterfaceTest>)

add_executable(combinedListModelTest
        ../src/data-entity.cpp
        ../src/libappdata/combined-list-model.cpp
        src/libappdata/combined-list-model-test.cpp)
target_link_libraries(combinedListModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME combinedListModelTest COMMAND $<TARGET_FILE:combinedListModelTest>)

add_executable(appPageBackendTest
        ../src/data-entity.cpp
        ../src/libappdata/app-list-plugin.cpp
        ../src/libappdata/app-list-model.cpp
        ../src/libappdata/app-page-backend.cpp
        ../src/libappdata/app-category-plugin.h
        ../src/libappdata/app-search-plugin.h
        ../src/extension/context-menu-extension.h
        ../src/extension/context-menu-manager.h
        src/libappdata/app-page-backend-test.cpp)
target_link_libraries(appPageBackendTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME appPageBackendTest COMMAND $<TARGET_FILE:appPageBackendTest>)

add_executable(basicAppFilterModelTest
        ../src/data-entity.cpp
        ../src/libappdata/basic-app-filter-model.cpp
        ../src/libappdata/basic-app-filter-model.h
        ../src/libappdata/basic-app-model.h
        ../src/utils/security-function-control.h
        src/libappdata/basic-app-filter-model-test.cpp)
target_link_libraries(basicAppFilterModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME basicAppFilterModelTest COMMAND $<TARGET_FILE:basicAppFilterModelTest>)

add_executable(basicAppModelTest
        ../src/data-entity.cpp
        ../src/libappdata/basic-app-model.cpp
        ../src/libappdata/app-database-interface.h
        src/libappdata/fakes/user-config.cpp
        src/libappdata/basic-app-model-test.cpp)
target_link_libraries(basicAppModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME basicAppModelTest COMMAND $<TARGET_FILE:basicAppModelTest>)

add_executable(recentlyInstalledModelTest
        ../src/data-entity.cpp
        ../src/libappdata/recently-installed-model.cpp
        ../src/libappdata/basic-app-filter-model.h
        src/libappdata/fakes/user-config.cpp
        src/libappdata/recently-installed-model-test.cpp)
target_link_libraries(recentlyInstalledModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME recentlyInstalledModelTest COMMAND $<TARGET_FILE:recentlyInstalledModelTest>)

add_executable(appSearchPluginTest
        ../src/data-entity.cpp
        ../src/libappdata/app-list-plugin.cpp
        ../src/libappdata/app-search-plugin.cpp
        ../src/libappdata/app-search-plugin.h
        ../src/libappdata/app-database-interface.h
        ../src/libappdata/basic-app-filter-model.h
        ../src/libappdata/recently-installed-model.h
        ../src/extension/context-menu-extension.h
        src/libappdata/fakes/UkuiSearchTask.h
        src/libappdata/fakes/ukui-search-task.cpp
        src/libappdata/app-search-plugin-test.cpp)
target_include_directories(appSearchPluginTest BEFORE PRIVATE
        src/libappdata/fakes
)
target_link_libraries(appSearchPluginTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME appSearchPluginTest COMMAND $<TARGET_FILE:appSearchPluginTest>)

add_executable(settingsTest
        ../src/settings/settings.cpp
        ../src/settings/settings.h
        src/settings/fakes/qgsettings.h
        src/settings/fakes/qgsettings.cpp
        src/settings/fakes/libkypackages.h
        src/settings/fakes/libkypackages.cpp
        src/settings/fakes/QDBusInterface
        src/settings/fakes/QDBusReply
        src/settings/settings-test.cpp)
target_include_directories(settingsTest BEFORE PRIVATE
        src/settings/fakes
)
target_compile_definitions(settingsTest PRIVATE
        UKUI_MENU_GLOBAL_CONFIG_FILE=\"${CMAKE_BINARY_DIR}/autotest-settings-global.conf\"
)
target_link_libraries(settingsTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME settingsTest COMMAND $<TARGET_FILE:settingsTest>)

add_executable(userConfigTest
        ../src/settings/user-config.cpp
        ../src/settings/user-config.h
        src/settings/user-config-test.cpp)
target_link_libraries(userConfigTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME userConfigTest COMMAND $<TARGET_FILE:userConfigTest>)
set_tests_properties(userConfigTest PROPERTIES
        ENVIRONMENT "HOME=${CMAKE_BINARY_DIR}/userConfigTestHome"
)

add_executable(appMenuPluginTest
        ../src/data-entity.cpp
        ../src/extension/menu-extension-plugin.cpp
        ../src/extension/context-menu-extension.cpp
        ../src/extension/menu/app-menu-plugin.cpp
        ../src/libappdata/app-database-interface.h
        ../src/libappdata/basic-app-filter-model.h
        ../src/libappdata/recently-installed-model.h
        src/libappdata/fakes/settings.cpp
        ../src/utils/security-function-control.h
        ../src/utils/app-manager.h
        src/extension/menu/app-menu-plugin-test.cpp)
target_include_directories(appMenuPluginTest BEFORE PRIVATE
        src/libappdata/fakes
)
target_link_libraries(appMenuPluginTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        ukui-quick::platform
)
add_test(NAME appMenuPluginTest COMMAND $<TARGET_FILE:appMenuPluginTest>)

add_executable(contextMenuManagerTest
        ../src/data-entity.cpp
        ../src/extension/context-menu-extension.cpp
        ../src/extension/context-menu-manager.h
        src/extension/fakes/menu-extension-loader.h
        src/extension/fakes/basic-app-filter-model.h
        src/extension/context-menu-manager-test.cpp)
target_include_directories(contextMenuManagerTest BEFORE PRIVATE
        src/extension/fakes
)
target_link_libraries(contextMenuManagerTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME contextMenuManagerTest COMMAND $<TARGET_FILE:contextMenuManagerTest>)

add_executable(menuExtensionLoaderTest
        ../src/data-entity.cpp
        ../src/extension/menu-extension-plugin.h
        ../src/extension/widget-extension.h
        src/extension/menu-extension-loader-test.cpp)
target_link_libraries(menuExtensionLoaderTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME menuExtensionLoaderTest COMMAND $<TARGET_FILE:menuExtensionLoaderTest>)
set_tests_properties(menuExtensionLoaderTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(widgetExtensionModelTest
        ../src/extension/widget-extension.h
        ../src/extension/widget-extension-model.h
        src/extension/widget-extension-model-test.cpp)
target_link_libraries(widgetExtensionModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
)
add_test(NAME widgetExtensionModelTest COMMAND $<TARGET_FILE:widgetExtensionModelTest>)

add_executable(widgetModelTest
        ../src/extension/widget-extension.h
        ../src/extension/widget-model.h
        src/extension/widget-model-test.cpp)
target_link_libraries(widgetModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
)
add_test(NAME widgetModelTest COMMAND $<TARGET_FILE:widgetModelTest>)

set(APP_FAVORITE_MODEL_MOC
        ${CMAKE_CURRENT_BINARY_DIR}/src/extension/favorite/moc_app-favorite-model.cpp)
if(COMMAND qt_generate_moc)
    qt_generate_moc(../src/extension/favorite/app-favorite-model.h
            ${APP_FAVORITE_MODEL_MOC})
else()
    qt5_generate_moc(../src/extension/favorite/app-favorite-model.h
            ${APP_FAVORITE_MODEL_MOC})
endif()

add_executable(appFavoriteModelTest
        ../src/data-entity.cpp
        ../src/extension/context-menu-extension.cpp
        ../src/libappdata/app-database-interface.h
        ../src/extension/favorite/app-favorite-model.h
        ../src/extension/favorite/favorite-folder-helper.h
        ../src/extension/favorite/favorites-config.h
        src/extension/favorite/app-favorite-model-test.cpp)
add_custom_target(appFavoriteModelTest_moc DEPENDS
        ${APP_FAVORITE_MODEL_MOC}
)
add_dependencies(appFavoriteModelTest appFavoriteModelTest_moc)
target_include_directories(appFavoriteModelTest BEFORE PRIVATE
        ../src
        ${CMAKE_CURRENT_BINARY_DIR}/src/extension/favorite
        src/extension/favorite
)
target_link_libraries(appFavoriteModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME appFavoriteModelTest COMMAND $<TARGET_FILE:appFavoriteModelTest>)

add_executable(favoriteContextMenuTest
        ../src/data-entity.cpp
        ../src/extension/context-menu-extension.cpp
        ../src/extension/favorite/favorite-folder-helper.h
        ../src/extension/favorite/favorite-context-menu.h
        src/extension/favorite/favorite-context-menu-test.cpp)
target_link_libraries(favoriteContextMenuTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME favoriteContextMenuTest COMMAND $<TARGET_FILE:favoriteContextMenuTest>)

add_executable(favoriteFilterModelTest
        ../src/data-entity.cpp
        ../src/extension/context-menu-extension.cpp
        ../src/extension/favorite/favorite-filter-model.h
        ../src/extension/favorite/favorite-folder-helper.h
        ../src/utils/security-function-control.h
        src/extension/favorite/favorite-filter-model-test.cpp)
target_link_libraries(favoriteFilterModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME favoriteFilterModelTest COMMAND $<TARGET_FILE:favoriteFilterModelTest>)

add_executable(favoriteFolderHelperTest
        ../src/data-entity.cpp
        ../src/libappdata/app-database-interface.h
        ../src/extension/favorite/favorite-folder-helper.h
        ../src/extension/favorite/favorites-config.h
        ../src/utils/security-function-control.h
        ../src/utils/event-track.h
        src/extension/favorite/favorite-folder-helper-test.cpp)
target_link_libraries(favoriteFolderHelperTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME favoriteFolderHelperTest COMMAND $<TARGET_FILE:favoriteFolderHelperTest>)

add_executable(favoriteWidgetTest
        ../src/extension/widget-extension.cpp
        ../src/extension/widget-extension.h
        ../src/extension/favorite/favorite-widget.h
        src/extension/favorite/favorite-widget-test.cpp)
target_link_libraries(favoriteWidgetTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
)
add_test(NAME favoriteWidgetTest COMMAND $<TARGET_FILE:favoriteWidgetTest>)

add_executable(favoriteExtensionPluginTest
        ../src/data-entity.cpp
        ../src/extension/menu-extension-plugin.h
        ../src/extension/widget-extension.h
        src/extension/favorite/favorite-extension-plugin-test.cpp)
target_link_libraries(favoriteExtensionPluginTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME favoriteExtensionPluginTest COMMAND $<TARGET_FILE:favoriteExtensionPluginTest>)
set_tests_properties(favoriteExtensionPluginTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(favoritesConfigTest
        ../src/data-entity.cpp
        ../src/libappdata/app-database-interface.h
        ../src/extension/favorite/favorites-config.h
        ../src/extension/favorite/favorite-folder-helper.h
        src/extension/favorite/favorites-config-test.cpp)
target_link_libraries(favoritesConfigTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME favoritesConfigTest COMMAND $<TARGET_FILE:favoritesConfigTest>)

add_executable(folderModelTest
        ../src/data-entity.cpp
        ../src/extension/context-menu-extension.cpp
        ../src/libappdata/app-database-interface.h
        ../src/extension/favorite/folder-model.h
        ../src/extension/favorite/favorite-folder-helper.h
        ../src/utils/security-function-control.h
        src/extension/favorite/folder-model-test.cpp)
target_link_libraries(folderModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME folderModelTest COMMAND $<TARGET_FILE:folderModelTest>)

add_executable(appManagerTest
        ../src/data-entity.cpp
        ../src/libappdata/app-database-interface.h
        ../src/libappdata/basic-app-filter-model.h
        ../src/utils/app-manager.h
        src/utils/app-manager-test.cpp)
target_link_libraries(appManagerTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        ukui-quick::platform
)
add_test(NAME appManagerTest COMMAND $<TARGET_FILE:appManagerTest>)

add_executable(maskMouseAreaTest
        ../src/utils/mask-mouse-area.cpp
        ../src/utils/mask-mouse-area.h
        src/utils/mask-mouse-area-test.cpp)
target_link_libraries(maskMouseAreaTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::QuickPrivate
        Qt${QT_VERSION_MAJOR}::Qml
)
add_test(NAME maskMouseAreaTest COMMAND $<TARGET_FILE:maskMouseAreaTest>)
set_tests_properties(maskMouseAreaTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(eventTrackTest
        ../src/utils/event-track.cpp
        ../src/utils/event-track.h
        src/utils/fakes/kysdk/diagnosetest/libkydatacollect.cpp
        src/utils/event-track-test.cpp)
target_include_directories(eventTrackTest BEFORE PRIVATE
        src/utils/fakes
)
target_link_libraries(eventTrackTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME eventTrackTest COMMAND $<TARGET_FILE:eventTrackTest>)

add_executable(securityFunctionControlTest
        ../src/utils/security-function-control.cpp
        ../src/utils/security-function-control.h
        src/utils/fakes/function-control.cpp
        src/utils/security-function-control-test.cpp)
target_include_directories(securityFunctionControlTest BEFORE PRIVATE
        src/utils/fakes
)
target_link_libraries(securityFunctionControlTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME securityFunctionControlTest COMMAND $<TARGET_FILE:securityFunctionControlTest>)

add_executable(powerButtonTest
        ../src/utils/power-button.h
        src/utils/power-button-test.cpp)
target_include_directories(powerButtonTest BEFORE PRIVATE
        src/utils/fakes
)
target_link_libraries(powerButtonTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME powerButtonTest COMMAND $<TARGET_FILE:powerButtonTest>)
set_tests_properties(powerButtonTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(userMenuTest
        ../src/utils/user-menu.cpp
        ../src/utils/user-menu.h
        src/utils/user-menu-test.cpp)
target_link_libraries(userMenuTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME userMenuTest COMMAND $<TARGET_FILE:userMenuTest>)

add_executable(userInfoItemTest
        ../src/utils/user-info-item.cpp
        ../src/utils/user-info-item.h
        src/utils/user-info-item-test.cpp)
target_link_libraries(userInfoItemTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME userInfoItemTest COMMAND $<TARGET_FILE:userInfoItemTest>)

add_executable(sidebarButtonUtilsTest
        ../src/utils/sidebar-button-utils.h
        ../src/utils/user-info-item.cpp
        ../src/utils/user-info-item.h
        ../src/utils/user-menu.cpp
        ../src/utils/user-menu.h
        src/utils/sidebar-button-utils-test.cpp)
target_include_directories(sidebarButtonUtilsTest BEFORE PRIVATE
        src/utils/fakes
)
target_link_libraries(sidebarButtonUtilsTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME sidebarButtonUtilsTest COMMAND $<TARGET_FILE:sidebarButtonUtilsTest>)

add_executable(panelFocusBridgeTest
        ../src/utils/panel-focus-bridge.h
        src/utils/panel-focus-bridge-test.cpp)
target_link_libraries(panelFocusBridgeTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME panelFocusBridgeTest COMMAND $<TARGET_FILE:panelFocusBridgeTest>)

add_executable(menuMainWindowTest
        ../src/windows/menu-main-window.cpp
        ../src/windows/menu-main-window.h
        src/settings/fakes/qgsettings.cpp
        src/windows/fakes/settings.h
        src/windows/fakes/context-menu-manager.h
        src/windows/fakes/security-function-control.h
        src/windows/fakes/window-helper.h
        src/windows/menu-main-window-test.cpp)
target_include_directories(menuMainWindowTest BEFORE PRIVATE
        src/windows/fakes
        src/settings/fakes
)
target_link_libraries(menuMainWindowTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        glib-2.0
        gobject-2.0
        gio-2.0
)
add_test(NAME menuMainWindowTest COMMAND $<TARGET_FILE:menuMainWindowTest>)

add_executable(menuDbusServiceTest
        ../src/menu-dbus-service.h
        src/dbus/fakes/menuadaptor.h
        src/menu-dbus-service-test.cpp)
target_include_directories(menuDbusServiceTest BEFORE PRIVATE
        src/dbus/fakes
)
target_link_libraries(menuDbusServiceTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME menuDbusServiceTest COMMAND $<TARGET_FILE:menuDbusServiceTest>)
set_tests_properties(menuDbusServiceTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(ukuiMenuApplicationTest
        ../src/commons.cpp
        ../src/commons.h
        ../src/ukui-menu-application.cpp
        ../src/ukui-menu-application.h
        ../src/data-entity.cpp
        src/application/fakes/settings.h
        src/application/fakes/power-button.h
        src/application/fakes/sidebar-button-utils.h
        src/application/fakes/mask-mouse-area.h
        src/application/fakes/app-manager.h
        src/application/fakes/context-menu-manager.h
        src/application/fakes/security-function-control.h
        src/application/fakes/event-track.h
        src/application/fakes/widget-model.h
        src/application/fakes/app-group-model.h
        src/application/fakes/app-page-backend.h
        src/application/fakes/menu-dbus-service.h
        src/application/fakes/menu-main-window.h
        src/application/fakes/keep-alive-client.h
        src/application/fakes/favorite/folder-model.h
        src/application/fakes/favorite/favorite-filter-model.h
        src/ukui-menu-application-test.cpp)
target_include_directories(ukuiMenuApplicationTest BEFORE PRIVATE
        src/application/fakes
        src/application/fakes/favorite
)
target_link_libraries(ukuiMenuApplicationTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME ukuiMenuApplicationTest COMMAND $<TARGET_FILE:ukuiMenuApplicationTest>)
