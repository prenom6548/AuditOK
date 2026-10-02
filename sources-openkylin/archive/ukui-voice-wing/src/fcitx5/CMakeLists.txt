message(STATUS "Building ukui-voice-wing fcitx5 module")

find_package(Fcitx5Core REQUIRED)
find_package(Fcitx5ModuleDBus REQUIRED)

string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" FCITX5_MAJOR_VERSION "${Fcitx5Core_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.([0-9]+)\\.[0-9]+.*" "\\1" FCITX5_MINOR_VERSION "${Fcitx5Core_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" FCITX5_PATCH_VERSION "${Fcitx5Core_VERSION}")
message(NOTICE "FCITX_INSTALL_CMAKECONFIG_DIR: [${FCITX_INSTALL_CMAKECONFIG_DIR}], Fcitx5Core_VERSION: [${Fcitx5Core_VERSION}], FCITX5_MAJOR_VERSION: [${FCITX5_MAJOR_VERSION}], FCITX5_MINOR_VERSION: [${FCITX5_MINOR_VERSION}], FCITX5_PATCH_VERSION: [${FCITX5_PATCH_VERSION}]")


include_directories(${fcitx5_INCLUDE_DIRS})
add_library(${PROJECT_NAME}-fcitx5 MODULE module.cpp)
target_link_libraries(${PROJECT_NAME}-fcitx5 PRIVATE Fcitx5::Core Fcitx5::Module::DBus)

if(FCITX5_MAJOR_VERSION GREATER_EQUAL 5 AND FCITX5_MINOR_VERSION GREATER_EQUAL 1 AND FCITX5_PATCH_VERSION GREATER_EQUAL 19)
	set_property(TARGET ${PROJECT_NAME}-fcitx5 PROPERTY CXX_STANDARD 20)
	set_property(TARGET ${PROJECT_NAME}-fcitx5 PROPERTY CXX_STANDARD_REQUIRED ON)
endif()

install(TARGETS ${PROJECT_NAME}-fcitx5 DESTINATION "${CMAKE_INSTALL_LIBDIR}/fcitx5/")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/voice-wing.conf.in" "${CMAKE_CURRENT_BINARY_DIR}/voice-wing.conf" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/voice-wing.conf" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/fcitx5/addon")
