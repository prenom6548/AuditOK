cmake_minimum_required(VERSION 3.12)

project("KysdkAiCommon" VERSION 1.0.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/kylin-ai/common/")

add_library(KysdkAiCommon SHARED
    ${SOURCES})

set_target_properties(KysdkAiCommon PROPERTIES
    OUTPUT_NAME "kysdk-ai-common"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(
    KysdkAiCommon
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(TARGETS KysdkAiCommon
        EXPORT KysdkAiCommon_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file("KysdkAiCommonConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/KysdkAiCommonConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/KysdkAiCommonConfig.cmake"
  INSTALL_DESTINATION
  ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(EXPORT KysdkAiCommon_Targets
        FILE KysdkAiCommonTargets.cmake
        NAMESPACE KysdkAiCommon::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(FILES "${PROJECT_BINARY_DIR}/KysdkAiCommonConfig.cmake"
              "${PROJECT_BINARY_DIR}/KysdkAiCommonConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/kylin-ai DESTINATION include)
install(DIRECTORY usr/ DESTINATION /usr)
