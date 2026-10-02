#######################################
# Effect

set(touchclick_SOURCES
    touchclick.cpp
    main.cpp
)

kwin4_add_effect_module(kwin4_effect_touchclick ${touchclick_SOURCES})

# Data files
install(FILES data/circletexture.png DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME})
install(FILES data/ringstexture.png DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME})
