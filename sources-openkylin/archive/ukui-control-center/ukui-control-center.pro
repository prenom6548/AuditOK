TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
    libukcc \
    changeOtherUserPwd \
    changeUserPwd \
    checkUserPwdWithPAM \
    registeredQDbus \
    plugins\
    registeredSession \
    shell \
    group-manager-server \
    tests \
    fuzz \

#默认不编译测试用例，仅自测时打开
SUBDIRS -= tests

SUBDIRS -= fuzz

TRANSLATIONS += \
    shell/res/i18n/zh_CN.ts \
    shell/res/i18n/tr.ts \
    shell/res/i18n/en_US.ts \
    shell/res/i18n/kk.ts \
    shell/res/i18n/ky.ts \
    shell/res/i18n/ug.ts \
    shell/res/i18n/bo_CN.ts \
    shell/res/i18n/de.ts \
    shell/res/i18n/es.ts \
    shell/res/i18n/fr.ts \
    shell/res/i18n/lzh.ts \
    shell/res/i18n/mn_MN.ts \
    shell/res/i18n/zh_Hant.ts \
    shell/res/i18n/zh_HK.ts \
    shell/res/i18n/vi.ts \  # 京语
    shell/res/i18n/ms.ts \  # 马来语
    shell/res/i18n/th.ts \  # 泰语


# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

isEmpty(PREFIX) {
    PREFIX = /usr
}

qm_files.path = $${PREFIX}/share/ukui-control-center/shell/res/i18n/
qm_files.files = shell/res/i18n/*.qm

ts_files.path = $${PREFIX}/share/ukui-control-center/shell/res/i18n/
ts_files.files = shell/res/i18n/*.ts


search_file.path = $${PREFIX}/share/ukui-control-center/shell/res/
search_file.files = shell/res/search.xml

INCLUDEPATH += /usr/lib/gcc/aarch64-linux-gnu/9/include/

INSTALLS += qm_files \
            ts_files \
            search_file

HEADERS += \
    shell/utils/xatom-helper.h

SOURCES += \
    shell/utils/xatom-helper.cpp
