TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
    kyconf2-gtest \
    kyconfig-gtest \
    kygsettings-gtest \
    kylog-gtest \
    kytimer-gtest \
    kyutils-gtest \
    kyskiplist-gtest

# Default rules for deployment.
headers.files = $${HEADERS}
headers.path = /usr/include/kysdk/desktop/
target.path = /usr/lib/$$QMAKE_HOST.arch-linux-gnu
INSTALLS += target headers
