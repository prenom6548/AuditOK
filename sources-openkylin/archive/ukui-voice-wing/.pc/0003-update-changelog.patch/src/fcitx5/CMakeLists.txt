message(STATUS "Building ukui-voice-wing fcitx5 module")

find_package(Fcitx5Core REQUIRED)
find_package(Fcitx5ModuleDBus REQUIRED)

include_directories(${fcitx5_INCLUDE_DIRS})
add_library(${PROJECT_NAME}-fcitx5 MODULE module.cpp)
target_link_libraries(${PROJECT_NAME}-fcitx5 PRIVATE Fcitx5::Core Fcitx5::Module::DBus)

install(TARGETS ${PROJECT_NAME}-fcitx5 DESTINATION "${CMAKE_INSTALL_LIBDIR}/fcitx5/")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/voice-wing.conf.in" "${CMAKE_CURRENT_BINARY_DIR}/voice-wing.conf" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/voice-wing.conf" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/fcitx5/addon")
