TEMPLATE = subdirs

SUBDIRS = $$PWD/service/service.pro $$PWD/app/app.pro $$PWD/test/test.pro \
    peony-share-menu-plugin

daemonConfig.files = $$PWD/data/org.ukui.samba.share.config.conf
daemonConfig.path = /usr/share/dbus-1/system.d/

daemonPolicy.files = $$PWD/data/org.ukui.samba.share.config.policy
daemonPolicy.path = /usr/share/polkit-1/actions

daemonService.files = $$PWD/data/org.ukui.samba.share.config.service
daemonService.path = /usr/share/dbus-1/system-services


INSTALLS += daemonConfig daemonPolicy daemonService

SKIP_TEST = $$(EXTENSIONS_SKIP_TEST)
isEmpty(SKIP_TEST) {
    message("build with tests")
    QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_CXXFLAGS += --coverage
    LIBS += -lgcov
}

daemonLimit.files = $$PWD/data/org.ukui.samba.share.config.limit
daemonLimit.path = /usr/share/dbus-1/conf/

daemonLimit1.files = $$PWD/data/org.ukui.samba.share.config.limit
daemonLimit1.path = /usr/share/peony-extensions/kylin-config

daemonVerify.files = $$PWD/data/org.ukui.samba.share.config.limit.verify
daemonVerify.path = /usr/share/dbus-1/conf/

daemonVerify1.files = $$PWD/data/org.ukui.samba.share.config.limit.verify
daemonVerify1.path = /usr/share/peony-extensions/kylin-config

conf.files = $$PWD/data/org.ukui.samba.share.config.yaml
conf.path = /usr/share/peony-extensions/kylin-config

INSTALLS += daemonConfig daemonPolicy daemonService daemonLimit daemonLimit1 daemonVerify daemonVerify1 conf
