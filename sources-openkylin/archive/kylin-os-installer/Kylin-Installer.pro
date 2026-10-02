TEMPLATE = subdirs

SUBDIRS += \
    src/plugins/VirtualKeyboard \
    src/plugins/Reflex \
    src/plugins/PluginDll \
    src/plugins/PluginService \
    src/plugins/KChoiceLanguage \
    src/plugins/KChoiceImg \
    src/plugins/KTimeZone \
    src/plugins/PluginTask \
    src/plugins/KyLicense \
    src/plugins/KUserRegister \
    src/plugins/KPartition \
    src/plugins/KInstallAPP \
    src/plugins/KNetConnect \
    src/plugins/KChoiceEdition \
    src/Installer_main


CONFIG += ordered \
    qt \
    qt


TRANSLATIONS += \
    $$PWD/language/zh_CN.ts \
    $$PWD/language/en_US.ts \
    $$PWD/language/bo_CN.ts \
    $$PWD/language/kk_CN.ts \
    $$PWD/language/ug_CN.ts \
    $$PWD/language/ky_CN.ts \
    $$PWD/language/zh_HK.ts \
    $$PWD/language/ar_AE.ts \
    $$PWD/language/de_DE.ts \
    $$PWD/language/es_ES.ts \
    $$PWD/language/fr_FR.ts \
    $$PWD/language/vi_VN.ts \
    $$PWD/language/mn_MN.ts
#INCLUDEPATH += /home/xy/Downloads/workdir/git/2020-6new


QT += widgets




