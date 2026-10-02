TEMPLATE = subdirs

SUBDIRS += core \
           coreinteraction \
           errorreact \
           viewcontrol

core.file = core/core/core-test.pro
coreinteraction.file = core/coreinteraction/coreinteraction-test.pro
errorreact.file = core/errorreact/errorreact-test.pro
viewcontrol.file = core/viewcontrol/viewcontrol-test.pro
