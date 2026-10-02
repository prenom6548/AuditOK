QT += widgets gui dbus

CONFIG += c++11 link_pkgconfig
PKGCONFIG += \
                gtest \
                gtest_main \
                dbus-1 \
                glib-2.0 \
                udev \
                libusb-1.0 \
                sqlite3\
                uuid\
                libbiometric\
                opencv4\
                
                

QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline
QMAKE_CFLAGS += --coverage -fno-inline

LIBS += -ldl -lgcov -ldbus-glib-1 -lgobject-2.0 -lglib-2.0 -lblkid -lsystemd -lhd -ludev -lcurl 

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
                ../../libwechat

SOURCES += \
    verify.cpp \
    ../../libwechat/kywechat.c\
    ../../libwechat/ksb_http.c\
    ../../libwechat/cJSON.c\
    ../../libwechat/wechat_internal.cpp\
    ../../libwechat/wechat_realization.cpp\
    ../../wechat_driver.c
    
HEADERS += \
    ../../libwechat/wechat_internal.h\
    ../../libwechat/wechat_realization.h\
    ../../libwechat/wechat_type.h\
    ../../libwechat/kywechat.h\
    ../../libwechat/ksb_http.h\


# Default rules for deployment.
headers.files = $${HEADERS}
headers.path = /usr/include/biometric/
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers