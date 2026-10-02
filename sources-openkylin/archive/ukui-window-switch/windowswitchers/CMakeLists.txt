# packages
set(KWIN_NAME "kwin")
set(KDE_INSTALL_DATADIR "/usr/share")
set(KDE_INSTALL_KSERVICES5DIR "/usr/share/kservices5")
install( DIRECTORY thumbnail_grid   DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/tabbox )

# service files
install( FILES thumbnail_grid/metadata.desktop   DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_thumbnail_grid.desktop )