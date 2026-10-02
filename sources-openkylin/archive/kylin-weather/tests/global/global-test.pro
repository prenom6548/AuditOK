TEMPLATE = subdirs

SUBDIRS += \
    corevar \
    rotatechangeinfo \
    viewvar

# 指定每个子项目的.pro文件
corevar.file = corevar/corevar_test.pro
rotatechangeinfo.file = rotatechangeinfo/rotatechangeinfo_test.pro
viewvar.file = viewvar/viewvar_test.pro
