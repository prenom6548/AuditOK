project(qt5-ukui)

# PLATFORMTHEME_SKIP_TEST=1：Debian 打包，跳过 tests，且不启用覆盖率（避免运行时 gcov 写 /build/...）
# 未设置或非 1：编 tests，全树 --coverage（供单测与 gcovr/lcov）
if(NOT "$ENV{PLATFORMTHEME_SKIP_TEST}" STREQUAL "1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
endif()

add_subdirectory(ukui-styles)
add_subdirectory(libqt5-ukui-style)
add_subdirectory(qt5-ukui-filedialog)
add_subdirectory(qt5-ukui-platformtheme)
add_subdirectory(ukui-qqc2-style)
add_subdirectory(ukui-qml-style-helper)

if(NOT "$ENV{PLATFORMTHEME_SKIP_TEST}" STREQUAL "1")
    add_subdirectory(tests)
    enable_testing()
    add_test(NAME TestSettings
        COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/tests/auto-test/run-auto-test.sh"
                $<TARGET_FILE:test-settings>)
endif()
