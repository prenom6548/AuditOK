TEMPLATE = subdirs

SUBDIRS += \
    libukui-app-widget \
    ukui-appwidget-test \
    ukui-appwidget-manager

ukui-appwidget-test.depends += libukui-app-widget
ukui-appwidget-manager.depends += libukui-app-widget
