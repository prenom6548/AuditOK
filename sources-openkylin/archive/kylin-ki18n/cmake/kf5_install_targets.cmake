# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# 安装共享库与插件；在 KI18N_RS_FULL_INSTALL 时加入 EXPORT KF5I18nTargets。
# EXPORT_NAME 在各自 add_library 子目录中设置。

if(KI18N_RS_FULL_INSTALL)
    install(TARGETS KF5I18n KF5I18nLocaleData
        EXPORT KF5I18nTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
else()
    install(TARGETS KF5I18n KF5I18nLocaleData
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

install(TARGETS ktranscript
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt5/plugins/kf5
)

install(TARGETS ki18nlocaledataqmlplugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt5/qml/org/kde/i18n/localeData
)
install(FILES ${CMAKE_SOURCE_DIR}/cpp/localedata-qml/qmldir
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt5/qml/org/kde/i18n/localeData
)
