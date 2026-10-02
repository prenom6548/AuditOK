cmake_minimum_required(VERSION 3.14)
project(ukui-search LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (BUILD_TEST)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
enable_testing()
endif ()

include(GNUInstallDirs)

add_subdirectory(3rd-parties/qtsingleapplication)
add_subdirectory(perftools)
add_subdirectory(libchinese-segmentation)
add_subdirectory(libsearch)
add_subdirectory(frontend)
add_subdirectory(ukuisearch-systemdbus)
add_subdirectory(search-ukcc-plugin)
add_subdirectory(ukui-search-service)
add_subdirectory(ukui-search-app-data-service)
add_subdirectory(ukui-search-service-dir-manager)

if (BUILD_TEST)
    add_subdirectory(tests)
endif ()
