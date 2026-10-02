TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    horscreen \
    search \
    verscreen

horscreen.file = horscreen/horscreen-test.pro
search.file = search/search-test.pro
verscreen.file = verscreen/verscreen_test.pro