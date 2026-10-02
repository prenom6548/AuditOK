QT += testlib widgets
CONFIG += testcase
TEMPLATE = app

TARGET = test_splitline

INCLUDEPATH += ../../ \
               ../../../../view/horscreen/ \
               ../../../..

SOURCES += main.cpp \
           test_splitline.cpp \
           ../../../../view/horscreen/splitline.cpp

HEADERS += test_splitline.h \
           ../../../../view/horscreen/splitline.h

# Coverage flags
QMAKE_CXXFLAGS += --coverage -fno-exceptions -fbranch-count-reg
QMAKE_LFLAGS += --coverage -lgcov