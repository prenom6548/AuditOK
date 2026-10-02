QT += core concurrent
PKGCONFIG += kysdk-ocr
HEADERS += \
    $$PWD/kylinocr.h \
    $$PWD/ocr.h

SOURCES += \
   $$PWD/kylinocr.cpp \
   $$PWD/ocr.cpp
