# Make sure it produce gb18030.so instead of libgb18030.so
add_library(gb18030 SHARED gb18030.cpp)
target_link_libraries(gb18030 PRIVATE Fcitx5::Core Fcitx5::Utils)
install(TARGETS gb18030 DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5")

# Addon config file
# We need additional layer of conversion because we want PROJECT_VERSION in it.
configure_file(gb18030-addon.conf.in.in gb18030-addon.conf.in)
fcitx5_translate_desktop_file("${CMAKE_CURRENT_BINARY_DIR}/gb18030-addon.conf.in" gb18030-addon.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gb18030-addon.conf" RENAME gb18030.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

# Input Method registration file
fcitx5_translate_desktop_file(gb18030.conf.in gb18030.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gb18030.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")