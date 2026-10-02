TEMPLATE = subdirs
SUBDIRS += ukui-input-gather \
           ukuiinputgatherclient \
           ukui-input-gather-test \

ukui-input-gather-test.depends = ukuiinputgatherclient
