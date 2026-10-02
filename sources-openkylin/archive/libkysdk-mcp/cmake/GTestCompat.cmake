if(TARGET GTest::gtest)
    set(MCP_GTEST_TARGET GTest::gtest)
    set(MCP_GTEST_MAIN_TARGET GTest::gtest_main)
elseif(TARGET GTest::GTest)
    set(MCP_GTEST_TARGET GTest::GTest)
    set(MCP_GTEST_MAIN_TARGET GTest::Main)
elseif(TARGET gtest)
    set(MCP_GTEST_TARGET gtest)
    set(MCP_GTEST_MAIN_TARGET gtest_main)
else()
    include(FetchContent)

    set(BUILD_GMOCK OFF CACHE BOOL "Disable gmock for MCP tests" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "Disable gtest install rules" FORCE)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    if(TARGET GTest::gtest)
        set(MCP_GTEST_TARGET GTest::gtest)
        set(MCP_GTEST_MAIN_TARGET GTest::gtest_main)
    elseif(TARGET gtest)
        set(MCP_GTEST_TARGET gtest)
        set(MCP_GTEST_MAIN_TARGET gtest_main)
    else()
        message(FATAL_ERROR "Unsupported GTest package: failed to provide usable gtest targets")
    endif()
endif()
