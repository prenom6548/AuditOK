TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
    tests_accounts \
    tests_battery \
    tests_disk \
    tests_global \
    tests_hardware \
    tests_image \
    tests_location \
    tests_packages \
    tests_realtime \
    tests_storage \
    tests_systemtime \
    tests_net \
    tests_powermanagement \
    tests_proc \
    tests_search \
    tests_systeminfo

# Default rules for deployment.
headers.files = $${HEADERS}
headers.path = /usr/include/kysdk/base/
target.path = /usr/lib/$$QMAKE_HOST.arch-linux-gnu
INSTALLS += target headers
