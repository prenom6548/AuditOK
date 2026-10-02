TEMPLATE = subdirs

SUBDIRS += \
    libukui-appwidget-plugin \
    libukui-appwidget-manager \
    libukui-appwidget-provider

#编译依赖顺序
libukui-appwidget-plugin.depends += libukui-appwidget-manager
unix {
    installPath = $$[QT_INSTALL_LIBS]/qt5/qml/org/ukui/appwidget/
#    installPath = /usr/share/appwidget/qml/
    template.files = org.ukui.appwidget/*
    template.path = $$installPath
    INSTALLS += template
}
