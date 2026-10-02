QT += gui
TEMPLATE = lib
DEFINES += KYLINIMAGECODEC_LIBRARY
QMAKE_PROJECT_NAME = kylinimagecodec
VERSION = 1.0.0

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += link_pkgconfig
PKGCONFIG += librsvg-2.0 cairo

#stb
LIBS += -lstb \

#gif
LIBS += -lgif \

#freeimage
LIBS += -lfreeimage \
        -lfreeimageplus \

INCLUDEPATH += $$PWD/

SOURCES += \
    $$PWD/image_conver/image_conver.cpp \
    $$PWD/image_load/image_load.cpp \
    $$PWD/image_save/image_save.cpp \
    $$PWD/image_load/loadmovie.cpp \
    $$PWD/image_save/savemovie.cpp \
    $$PWD/kylinimagecodec.cpp

HEADERS += \
    $$PWD/loadmovie.h \
    $$PWD/savemovie.h \
    $$PWD/kylinimagecodec.h \
    $$PWD/kylinimagecodec_global.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

header.files += *.h
header.path = /usr/include/kylin_image_codec/
INSTALLS +=header
