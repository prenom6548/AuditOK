cmake_minimum_required(VERSION 3.12)
project(KylinAiProto VERSION 1.0.0)

include(GNUInstallDirs)
add_library(KylinAiProto INTERFACE)

install(TARGETS KylinAiProto
        EXPORT KylinAiProto_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file("KylinAiProtoConfigVersion.cmake"
                                    VERSION ${PROJECT_VERSION}
                                    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/KylinAiProtoConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/KylinAiProtoConfig.cmake"
    INSTALL_DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

configure_file("${PROJECT_SOURCE_DIR}/cmake/KylinAiProtoFunctions.cmake"
  ${CMAKE_CURRENT_BINARY_DIR}/KylinAiProtoFunctions.cmake
  COPYONLY)

configure_file("${PROJECT_SOURCE_DIR}/cmake/KylinAiProtoGenerate.cmake"
        ${CMAKE_CURRENT_BINARY_DIR}/KylinAiProtoGenerate.cmake
        COPYONLY)

install(EXPORT KylinAiProto_Targets
  FILE KylinAiProtoTargets.cmake
  NAMESPACE KylinAiProto::
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(FILES "${PROJECT_BINARY_DIR}/KylinAiProtoConfig.cmake"
  "${PROJECT_BINARY_DIR}/KylinAiProtoConfigVersion.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KylinAiProtoFunctions.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KylinAiProtoGenerate.cmake"
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(DIRECTORY protocols/usr/ DESTINATION /usr)
