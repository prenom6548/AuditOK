TEMPLATE = subdirs
  
CONFIG += ordered

SUBDIRS = \
    registeredQDbus \
    PowerManagementDaemon  \
    ukui-power-manager-tray \
    ukui-upower\
    power

QMAKE_STRIP =
unix:target_path=/usr/share/ukui-power-manager/tlp
tlp.files = $$PWD/3rd/tlp/*
tlp.path = $$target_path

INSTALLS += tlp
