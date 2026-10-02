TEMPLATE = subdirs
SUBDIRS += common \
           systembackup-daemon

common.file = common/common-test.pro
systembackup-daemon.file = systembackup-daemon/systembackup-daemon-test.pro     