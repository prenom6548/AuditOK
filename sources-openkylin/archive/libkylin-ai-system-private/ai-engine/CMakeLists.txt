cmake_minimum_required(VERSION 3.12)

project(
  "KylinAiEngine"
  VERSION 0.0.1
    DESCRIPTION
    "A header-only c++ library that provides type safety and user defined literals for handling pyhsical values defined in the International System of Units."
    HOMEPAGE_URL  "https://github.com/bernedom/SI")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

add_library(KylinAiEngine INTERFACE)

# Adding the install interface generator expression makes sure that the include
# files are installed to the proper location (provided by GNUInstallDirs)
target_include_directories(
    KylinAiEngine
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(KylinAiEngine INTERFACE cxx_std_17)

# locations are provided by GNUInstallDirs
install(TARGETS KylinAiEngine
        EXPORT KylinAiEngine_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file("KylinAiEngineConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/KylinAiEngineConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/KylinAiEngineConfig.cmake"
  INSTALL_DESTINATION
  ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

configure_file("${PROJECT_SOURCE_DIR}/cmake/KylinAiEngineFunctions.cmake"
  ${CMAKE_CURRENT_BINARY_DIR}/KylinAiEngineFunctions.cmake
  COPYONLY)

install(EXPORT KylinAiEngine_Targets
        FILE KylinAiEngineTargets.cmake
        NAMESPACE KylinAiEngine::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(FILES "${PROJECT_BINARY_DIR}/KylinAiEngineConfig.cmake"
              "${PROJECT_BINARY_DIR}/KylinAiEngineConfigVersion.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/KylinAiEngineFunctions.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/kylin-ai DESTINATION include)

message(PROJECT_SOURCE_DIR::${PROJECT_SOURCE_DIR}::${CMAKE_INSTALL_INCLUDEDIR})

