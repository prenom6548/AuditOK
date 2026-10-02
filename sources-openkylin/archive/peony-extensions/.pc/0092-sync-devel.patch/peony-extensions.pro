TEMPLATE = subdirs

VERSION = 3.2.2
DEFINES += VERSION='\\"$${VERSION}\\"'

SUBDIRS = \
#    peony-admin                         \
    peony-share                         \
    peony-drive-rename                  \
    peony-send-to-device                \
    peony-set-wallpaper                 \
    peony-bluetooth-plugin              \
    peony-engrampa-menu-plugin          \
    peony-extension-computer-view       \
    peony-menu-plugin-mate-terminal     \
    peony-intelligent-data-management-service \
    peony-intelligent-plugin            \
    peony-spaceview-plugin              \
    org.ukui.desktop-clock-widget       \
    org.ukui.desktop-image-widget       \
#    test-computer-view

# 根据环境变量设置是否编译tests子项目，默认编译
SKIP_TEST = $$(EXTENSIONS_SKIP_TEST)
isEmpty(SKIP_TEST) {
    message("build with tests")
    SUBDIRS += tests
} else {
    message("skip tests")
}
