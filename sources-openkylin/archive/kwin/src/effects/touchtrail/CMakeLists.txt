#######################################
# Effect

set(touchtrail_SOURCES
    touchtrail.cpp
    main.cpp
)

kwin4_add_effect_module(kwin4_effect_touchtrail ${touchtrail_SOURCES})


# Data files
install(FILES data/trailing.png DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME})

