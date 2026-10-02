QT += core dbus

TARGET = kysdk-ukenv
TEMPLATE = lib
CONFIG += c++11 console link_pkgconfig

PKGCONFIG += gsettings-qt

HEADERS += src/usermanual.h \
           src/currency.h \
           src/gsettingmonitor.h

SOURCES += src/usermanual.cpp \
           src/currency.cpp \
           src/gsettingmonitor.cpp

#Default rules for deployment
headers.files = $${HEADERS}
headers.path = /usr/include/kysdk/applications/
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers
