#######################################
# Effect

set(ubr_SOURCES
        ubreffect.cpp
        xatom-helper.cpp
        ubrtexturehelper.cpp
        shaderhelper.cpp
        main.cpp
        )

kconfig_add_kcfg_files(ubr_SOURCES
        ubrconfig.kcfgc
        )

kwin4_add_effect_module(kwin4_effect_ubr ${ubr_SOURCES})

#######################################
# Config
set(kwin_ubr_config_SRCS ubrconfig.cpp)
ki18n_wrap_ui(kwin_ubr_config_SRCS ubrconfig.ui)
kconfig_add_kcfg_files(kwin_ubr_config_SRCS ubrconfig.kcfgc)

kwin_add_effect_config(kwin_ubr_config ${kwin_ubr_config_SRCS})

target_link_libraries(kwin_ubr_config
        KF5::ConfigWidgets
        KF5::I18n
        Qt::DBus
        KWinEffectsInterface
        )
