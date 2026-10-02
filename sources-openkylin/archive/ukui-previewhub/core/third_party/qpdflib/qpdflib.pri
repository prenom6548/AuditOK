# TEMPLATE = lib
# TARGET = qpdf

# CONFIG += dll
# CONFIG += c++11
INCLUDEPATH += $$PWD

QT += widgets\
      webengine\
      webenginecore\
      webenginewidgets\
      webchannel

DEFINES += QPDFLIB_BUILD

HEADERS =\
    $$PWD/qpdfwidget.h \
    $$PWD/pdfjsbridge.h

SOURCES =\
    $$PWD/qpdfwidget.cpp \
    $$PWD/pdfjsbridge.cpp

RESOURCES += $$PWD/pdfview.qrc
