TEMPLATE = subdirs

CONFIG += ordered \
    qt

SUBDIRS = \
#    plugins/plugin.pro \
    libnm-kylin \
    kylin-keyring/kylin-keyring.pro\
#    kylin-nm-gui \
#    kylin-nm-server \
    tests
    
#默认不编译测试用例，仅自测时打开
SUBDIRS -= tests

#kylin-nm-gui.depends = libnm-kylin
#kylin-nm-server.depends = libnm-kylin

#TRANSLATIONS += \
#        translations/kylin-nm_zh_CN.ts \
#        translations/kylin-nm_tr.ts \
#        translations/kylin-nm_bo.ts
