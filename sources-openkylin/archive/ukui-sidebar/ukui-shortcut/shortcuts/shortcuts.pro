TEMPLATE = subdirs
SUBDIRS +=  $$PWD/brightness-shortcut \
            $$PWD/system-setting-shortcut \
            $$PWD/status-change-shortcut \
            $$PWD/screenshot-shortcut \
            $$PWD/wifi-shortcut \
            $$PWD/auto-rotation-shortcut \
            $$PWD/bluetooth-shortcut \
            $$PWD/flight-mode-shortcut \
            $$PWD/multi-screen-shortcut \
            $$PWD/volume-shortcut \
            $$PWD/night-mode-shortcut \
            $$PWD/power-mode-shortcut

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += ordered
