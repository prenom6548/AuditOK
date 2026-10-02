set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Place non-interactive items automated tests here.
# Name executables with `test-*` or `*_test` so `tests/runUnitTest.sh` can discover them.

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Qml Quick Test Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Qml Quick Test Widgets REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(gsettings-qt REQUIRED IMPORTED_TARGET gsettings-qt)
if (QT_VERSION_MAJOR EQUAL 5)
    pkg_check_modules(QtXdg REQUIRED IMPORTED_TARGET Qt5Xdg)
else()
    pkg_check_modules(QtXdg REQUIRED IMPORTED_TARGET Qt6Xdg)
endif()

add_executable(test-icon-heuristics
    test-icon-heuristics.cpp
)

target_include_directories(test-icon-heuristics PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform
    ${CMAKE_CURRENT_SOURCE_DIR}/../../framework
)

target_link_libraries(test-icon-heuristics PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick-items-plugin
)

set(TEST_ICON_HEURISTICS_RUNTIME_DIR "${CMAKE_BINARY_DIR}/items/autotest/runtime")
file(MAKE_DIRECTORY "${TEST_ICON_HEURISTICS_RUNTIME_DIR}")
file(CHMOD "${TEST_ICON_HEURISTICS_RUNTIME_DIR}"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)
add_test(NAME test-icon-heuristics COMMAND test-icon-heuristics)
set_tests_properties(test-icon-heuristics PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;XDG_RUNTIME_DIR=${TEST_ICON_HEURISTICS_RUNTIME_DIR}"
)

add_executable(test-color-mixer
    test-color-mixer.cpp
    ../color-mixer.cpp
    ../color-mixer.h
    ../../platform/ukui/ukui-theme-proxy.cpp
    ../../platform/ukui/ukui-theme-proxy.h
)

target_include_directories(test-color-mixer PRIVATE
    ..
    ../../platform
    ../../platform/ukui
)

target_link_libraries(test-color-mixer PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    PkgConfig::gsettings-qt
)

add_test(NAME test-color-mixer COMMAND test-color-mixer)

add_executable(test-icon-helper
    test-icon-helper.cpp
    icon-helper-test.qrc
    ../icon-helper.cpp
    ../icon-helper.h
)

target_include_directories(test-icon-helper PRIVATE ..)

target_link_libraries(test-icon-helper PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    PkgConfig::QtXdg
)

add_test(NAME test-icon-helper COMMAND test-icon-helper)

add_executable(test-icon-provider
    test-icon-provider.cpp
    icon-helper-test.qrc
    ../icon-provider.cpp
    ../icon-provider.h
    ../icon-helper.cpp
    ../icon-helper.h
)

target_include_directories(test-icon-provider PRIVATE ..)

target_link_libraries(test-icon-provider PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    PkgConfig::QtXdg
)

add_test(NAME test-icon-provider COMMAND test-icon-provider)

add_executable(test-theme-icon
    test-theme-icon.cpp
    icon-helper-test.qrc
)

target_include_directories(test-theme-icon PRIVATE
    ..
    ../../core
    ../../platform
)

target_link_libraries(test-theme-icon PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    ukui-quick::platform
    ukui-quick-items-plugin
)

add_test(NAME test-theme-icon COMMAND test-theme-icon)

add_executable(test-icon
    test-icon.cpp
)

target_include_directories(test-icon PRIVATE
    ..
    ../../core
    ../../platform
)

target_link_libraries(test-icon PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick-items-plugin
)

add_test(NAME test-icon COMMAND test-icon)

set(TEST_LOTTIE_RUNTIME_DIR "${CMAKE_BINARY_DIR}/items/autotest/lottie-runtime")
file(MAKE_DIRECTORY "${TEST_LOTTIE_RUNTIME_DIR}")
file(CHMOD "${TEST_LOTTIE_RUNTIME_DIR}"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)

foreach(LOTTIE_TEST_TARGET IN ITEMS
        test-lottie-attached
        test-lottie-item
        test-lottie-renderer
        test-lottie-host-runtime
        test-lottie-runtime-service
        test-icon-lottie)
    add_executable(${LOTTIE_TEST_TARGET}
        ${LOTTIE_TEST_TARGET}.cpp
    )
    target_include_directories(${LOTTIE_TEST_TARGET} PRIVATE
        ..
        ../../core
        ../../platform
        ../../framework
    )

    target_link_libraries(${LOTTIE_TEST_TARGET} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Test
    )
    target_compile_definitions(${LOTTIE_TEST_TARGET} PRIVATE
        UKUI_QUICK_LOTTIE_CORE_TEST_EXPORTS
        UKUI_QUICK_LOTTIE_INTERNAL_TEST_EXPORTS
        UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM
    )
    # These detailed suites use private runtime seams and therefore link only
    # the never-installed test plugin. Production Host coverage is provided by
    # a separate black-box integration target.
    target_link_libraries(${LOTTIE_TEST_TARGET} PRIVATE
        ukui-quick-items-test-plugin
    )
    add_test(NAME ${LOTTIE_TEST_TARGET} COMMAND ${LOTTIE_TEST_TARGET})
    set_tests_properties(${LOTTIE_TEST_TARGET} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=generic;GSETTINGS_BACKEND=memory;XDG_RUNTIME_DIR=${TEST_LOTTIE_RUNTIME_DIR}"
    )
endforeach()

add_executable(test-lottie-production-hosts
    test-lottie-production-hosts.cpp
)
target_include_directories(test-lottie-production-hosts PRIVATE
    ..
    ../../core
    ../../platform
    ../../framework
)
target_link_libraries(test-lottie-production-hosts PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    PkgConfig::gsettings-qt
    ukui-quick::platform
    ukui-quick-items-plugin
)
add_test(NAME test-lottie-production-hosts COMMAND test-lottie-production-hosts)
set_tests_properties(test-lottie-production-hosts PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=generic;GSETTINGS_BACKEND=memory;XDG_RUNTIME_DIR=${TEST_LOTTIE_RUNTIME_DIR}"
)

foreach(LOTTIE_THEME_TEST_TARGET IN ITEMS
        test-lottie-attached
        test-lottie-item
        test-lottie-renderer
        test-lottie-host-runtime
        test-lottie-runtime-service
        test-icon-lottie)
    target_link_libraries(${LOTTIE_THEME_TEST_TARGET} PRIVATE
        PkgConfig::gsettings-qt
        ukui-quick::platform
    )
endforeach()

target_link_libraries(test-lottie-renderer PRIVATE PkgConfig::THORVG)

add_executable(lottie-blocking-unzip-helper EXCLUDE_FROM_ALL
    lottie-blocking-unzip-helper.cpp
)
target_link_libraries(lottie-blocking-unzip-helper PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)
target_compile_definitions(test-lottie-runtime-service PRIVATE
    LOTTIE_BLOCKING_UNZIP_HELPER_PATH="$<TARGET_FILE:lottie-blocking-unzip-helper>"
)
add_dependencies(test-lottie-runtime-service lottie-blocking-unzip-helper)

# This is an explicitly manual benchmark: it requires external real resources,
# so it is intentionally neither a test-* executable nor a CTest case.
add_executable(lottie-runtime-benchmark EXCLUDE_FROM_ALL
    lottie-runtime-benchmark.cpp
)

if(UKUI_QUICK_ENABLE_COVERAGE)
    target_compile_definitions(lottie-runtime-benchmark PRIVATE
        LOTTIE_RUNTIME_BENCHMARK_COVERAGE="enabled"
    )
else()
    target_compile_definitions(lottie-runtime-benchmark PRIVATE
        LOTTIE_RUNTIME_BENCHMARK_COVERAGE="disabled"
    )
endif()

target_include_directories(lottie-runtime-benchmark PRIVATE
    ..
    ../../core
    ../../platform
    ../../framework
)

target_link_libraries(lottie-runtime-benchmark PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    ukui-quick-items-test-plugin
)

target_compile_definitions(lottie-runtime-benchmark PRIVATE
    UKUI_QUICK_LOTTIE_CORE_TEST_EXPORTS
    UKUI_QUICK_LOTTIE_INTERNAL_TEST_EXPORTS
    UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM
)

target_compile_definitions(lottie-runtime-benchmark PRIVATE
    LOTTIE_RUNTIME_BENCHMARK_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
)

add_executable(test-action-extension
    test-action-extension.cpp
    ../action-extension.cpp
    ../action-extension.h
)

target_include_directories(test-action-extension PRIVATE ..)

target_link_libraries(test-action-extension PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)

add_test(NAME test-action-extension COMMAND test-action-extension)

add_executable(test-menu-item
    test-menu-item.cpp
    ../menu-item.cpp
    ../menu-item.h
    ../icon-helper.cpp
    ../icon-helper.h
)

target_include_directories(test-menu-item PRIVATE ..)

target_link_libraries(test-menu-item PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    PkgConfig::QtXdg
)

add_test(NAME test-menu-item COMMAND test-menu-item)

add_executable(test-shadowed-rectangle
    test-shadowed-rectangle.cpp
)

target_include_directories(test-shadowed-rectangle PRIVATE
    ..
)

target_link_libraries(test-shadowed-rectangle PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick-items-plugin
)

add_test(NAME test-shadowed-rectangle COMMAND test-shadowed-rectangle)

add_executable(test-painted-rectangle-item
    test-painted-rectangle-item.cpp
)

target_include_directories(test-painted-rectangle-item PRIVATE
    ..
)

target_link_libraries(test-painted-rectangle-item PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick-items-plugin
)

add_test(NAME test-painted-rectangle-item COMMAND test-painted-rectangle-item)

add_executable(test-gradient-border-item
    test-gradient-border-item.cpp
)

target_include_directories(test-gradient-border-item PRIVATE
    ..
    ../../platform
)

target_link_libraries(test-gradient-border-item PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
    ukui-quick-items-plugin
)

add_test(NAME test-gradient-border-item COMMAND test-gradient-border-item)

add_executable(test-line-edit-context-menu
    test-line-edit-context-menu.cpp
    ../line-edit-context-menu.cpp
    ../line-edit-context-menu.h
)

target_include_directories(test-line-edit-context-menu PRIVATE
    ..
)

target_link_libraries(test-line-edit-context-menu PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)

add_test(NAME test-line-edit-context-menu COMMAND test-line-edit-context-menu)

add_executable(test-menu
    test-menu.cpp
    ../menu.cpp
    ../menu.h
    ../menu-item.cpp
    ../menu-item.h
    ../icon-helper.cpp
    ../icon-helper.h
)

target_include_directories(test-menu PRIVATE
    ..
)

target_link_libraries(test-menu PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    PkgConfig::QtXdg
)

add_test(NAME test-menu COMMAND test-menu)

add_executable(test-tooltip-proxy
    test-tooltip-proxy.cpp
    ../tooltip-proxy.cpp
    ../tooltip-proxy.h
)

target_include_directories(test-tooltip-proxy PRIVATE
    ..
)

target_link_libraries(test-tooltip-proxy PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)

add_test(NAME test-tooltip-proxy COMMAND test-tooltip-proxy)

add_executable(test-tooltip-utils
    test-tooltip-utils.cpp
)

target_include_directories(test-tooltip-utils PRIVATE
    ..
    ../views
    ../../platform
    ../../core
)

target_link_libraries(test-tooltip-utils PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick-items-plugin
)

add_test(NAME test-tooltip-utils COMMAND test-tooltip-utils)

add_executable(test-window-blur-behind
    test-window-blur-behind.cpp
    ../window-blur-behind.cpp
    ../window-blur-behind.h
)

target_include_directories(test-window-blur-behind PRIVATE
    ..
    ../../platform
)

target_link_libraries(test-window-blur-behind PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-window-blur-behind COMMAND test-window-blur-behind)

add_executable(test-ukui-style-window
    test-ukui-style-window.cpp
    ../views/ukui-style-window.cpp
    ../views/ukui-style-window.h
)

target_include_directories(test-ukui-style-window PRIVATE
    ..
    ../views
    ../../platform
)

target_link_libraries(test-ukui-style-window PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::platform
)

add_test(NAME test-ukui-style-window COMMAND test-ukui-style-window)

add_executable(test-content-window
    test-content-window.cpp
    ../views/content-window.cpp
    ../views/content-window.h
)

target_include_directories(test-content-window PRIVATE
    ..
    ../views
    ../../platform
    ../../core
)

target_link_libraries(test-content-window PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::core
    ukui-quick::platform
)

add_test(NAME test-content-window COMMAND test-content-window)

add_executable(test-ukui-quick-items-plugin
    test-ukui-quick-items-plugin.cpp
)

target_include_directories(test-ukui-quick-items-plugin PRIVATE
    ..
)

target_link_libraries(test-ukui-quick-items-plugin PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick-items-plugin
)

add_test(NAME test-ukui-quick-items-plugin COMMAND test-ukui-quick-items-plugin)

add_executable(test-tooltip-dialog
    test-tooltip-dialog.cpp
)

target_include_directories(test-tooltip-dialog PRIVATE
    ..
    ../views
    ../../platform
    ../../core
)

target_link_libraries(test-tooltip-dialog PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::core
    ukui-quick::platform
    ukui-quick-items-plugin
)

add_test(NAME test-tooltip-dialog COMMAND test-tooltip-dialog)

add_executable(test-tooltip
    test-tooltip.cpp
)

target_include_directories(test-tooltip PRIVATE
    ..
    ../views
    ../../platform
    ../../core
)

target_link_libraries(test-tooltip PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::core
    ukui-quick::platform
    ukui-quick-items-plugin
)

add_test(NAME test-tooltip COMMAND test-tooltip)

add_executable(test-shadowed-texture
    test-shadowed-texture.cpp
)

target_include_directories(test-shadowed-texture PRIVATE
    ..
    ../scenegraph
)

target_link_libraries(test-shadowed-texture PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick-items-plugin
)

add_test(NAME test-shadowed-texture COMMAND test-shadowed-texture)
