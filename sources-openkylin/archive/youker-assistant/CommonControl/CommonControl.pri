INCLUDEPATH	+= $$PWD
DEPENDPATH      += $$PWD
include($$PWD/../commonfunc/commonfunc.pri)
HEADERS		+= \
                $$PWD/kabuttonproxystyle.h \
                $$PWD/kagroupbutton.h \
                $$PWD/kainfotitle.h \
                $$PWD/kalabel.h \
                $$PWD/kaslider.h \
                $$PWD/katabbar.h \
                $$PWD/katabbutton.h \
                $$PWD/kdriveinfoitem.h \
                $$PWD/kinfolistitem.h \
                $$PWD/loadingwidget.h \
                $$PWD/kdrivelabel.h \
                $$PWD/../src/imageutil.h \
                $$PWD/../src/util.h  \
                $$PWD/../src/loadwidget.h \
                
SOURCES		+= \
                $$PWD/kabuttonproxystyle.cpp \
                $$PWD/kagroupbutton.cpp \
                $$PWD/kainfotitle.cpp \
                $$PWD/kalabel.cpp \
                $$PWD/kaslider.cpp \
                $$PWD/katabbar.cpp \
                $$PWD/katabbutton.cpp \
                $$PWD/kdriveinfoitem.cpp \
                $$PWD/kinfolistitem.cpp \
                $$PWD/loadingwidget.cpp \
                $$PWD/kdrivelabel.cpp \
                $$PWD/../src/imageutil.cpp \
                $$PWD/../src/util.cpp  \
                $$PWD/../src/loadwidget.cpp
