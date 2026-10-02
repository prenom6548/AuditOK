TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
    kysdk-appmanager \
    kysdk-soundeffects \
    kysdk-notification \
    kysdk-shortcut \
    kysdk-thememanager

# Default rules for deployment.
headers.files = $${HEADERS}
headers.path = /usr/include/kysdk/desktop/
target.path = /usr/lib/$$QMAKE_HOST.arch-linux-gnu

INSTALLS += target headers exec
