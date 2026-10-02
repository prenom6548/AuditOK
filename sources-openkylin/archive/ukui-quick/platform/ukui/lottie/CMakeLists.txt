find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui REQUIRED)

set(LOTTIE_CORE_SOURCES
    lottie-asset.cpp
    lottie-asset.h
    lottie-core-visibility_p.h
    dot-lottie-parser.cpp
    dot-lottie-parser.h
    lottie-cache-manager.cpp
    lottie-cache-manager.h
    lottie-source-reference.cpp
    lottie-source-reference.h
)

function(add_lottie_core_target target)
    add_library(${target} STATIC ${LOTTIE_CORE_SOURCES})
    set_target_properties(${target} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
    )
    target_link_libraries(${target} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
    )
endfunction()

# Internal Lottie Core is linked into its consumers and is never installed or
# exported as a supported library ABI.
add_lottie_core_target(ukui-quick-lottie-core)
set_target_properties(ukui-quick-lottie-core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

if(BUILD_TEST)
    # White-box tests use an uninstalled variant whose test-only seams are
    # available to the test plugin and test executables.
    add_lottie_core_target(ukui-quick-lottie-core-test)
    target_compile_definitions(ukui-quick-lottie-core-test PRIVATE
        UKUI_QUICK_LOTTIE_CORE_TEST_EXPORTS
    )
endif()
