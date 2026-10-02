# 检测 Qt 版本
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION , ".")
QT_VER_MAJ = $$member(QT_VERSION , 0)
QT_VER_MIN = $$member(QT_VERSION , 1)
QT_VER_THR = $$member(QT_VERSION , 2)

QT += dbus core xml widgets svg network
TARGET = kysdk-kabase
TEMPLATE = lib

CONFIG += c++11 console link_pkgconfig no_keywords

LIBS += -ldl -lpthread -lsystemd -lkylog -lkyconf -lgif -L/usr/lib/kysdk/kysdk-base

greaterThan(QT_VER_MAJ , 4) {
    LIBS += -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lstb -lfreeimage -lfreeimageplus
} else {
    message("--- v10 project ---")
}

PKGCONFIG += gsettings-qt openssl x11

INCLUDEPATH += /usr/include/kysdk/kysdk-base/ \
               /usr/include/opencv4/

HEADERS += buried_point.hpp \
           log.hpp \
           gsettings.hpp \
           currency.hpp \
           dbus.hpp \
           application_access.hpp \
           kylin_system/xatom_helper.hpp \
           kylin_system/window_management.hpp \
           kylin_system/session_management.hpp \
           kylin_system/user_manual.hpp \
           kylin_system/system_information.hpp \
           kylin_system/theme_management.hpp \
           single_application/single_application.hpp \
           single_application/locked_file.hpp \
           single_application/local_peer.hpp

greaterThan(QT_VER_MAJ , 4) {
    HEADERS += kylin_image_codec/loadmovie.hpp \
               kylin_image_codec/savemovie.hpp \
               kylin_image_codec/kylinimagecodec.hpp \
               kylin_image_codec/kylinimagecodec_global.hpp \
} else {
    message("+++ v10 project +++")
}


SOURCES += buried_point.cpp \
           log.cpp \
           gsettings.cpp \
           currency.cpp \
           dbus.cpp\
           application_access.cpp\
           kylin_system/xatom_helper.cpp \
           kylin_system/window_management.cpp \
           kylin_system/session_management.cpp \
           kylin_system/user_manual.cpp \
           kylin_system/system_information.cpp \
           kylin_system/theme_management.cpp \
           single_application/single_application.cpp \
           single_application/locked_file.cpp \
           single_application/local_peer.cpp \
           single_application/locked_file_unix.cpp

greaterThan(QT_VER_MAJ , 4) {
    SOURCES += kylin_image_codec/image_conver/image_conver.cpp \
               kylin_image_codec/image_load/image_load.cpp \
               kylin_image_codec/image_save/image_save.cpp \
               kylin_image_codec/image_load/loadmovie.cpp \
               kylin_image_codec/image_save/savemovie.cpp \
               kylin_image_codec/kylinimagecodec.cpp \
} else {
    message("=== v10 project ===")
}

# 窗管模块
QT += x11extras KWindowSystem

# Default rules for deployment.
headers.files = application_access.hpp \
                buried_point.hpp \
                currency.hpp \
                dbus.hpp \
                gsettings.hpp \
                log.hpp
headers.path = /usr/include/kysdk/applications/kabase/

image_headers.files = kylin_image_codec/kylinimagecodec.hpp \
                kylin_image_codec/kylinimagecodec_global.hpp \
                kylin_image_codec/loadmovie.hpp \
                kylin_image_codec/savemovie.hpp
image_headers.path = /usr/include/kysdk/applications/kabase/kylin_image_codec/

system_headers.files = kylin_system/session_management.hpp \
                kylin_system/system_information.hpp \
                kylin_system/theme_management.hpp \
                kylin_system/user_manual.hpp \
                kylin_system/window_management.hpp
system_headers.path = /usr/include/kysdk/applications/kabase/kylin_system/

single_app_headers.files = single_application/single_application.hpp
single_app_headers.path = /usr/include/kysdk/applications/kabase/single_application/

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers image_headers system_headers single_app_headers
