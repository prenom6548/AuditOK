include (../codec/kylinimagecodec/kylinimagecodec.pro)
QT += testlib
QT -= gui
#QT += svg
QT += dbus quickcontrols2
QT += x11extras KWindowSystem printsupport network quick quickwidgets sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += UNUI

CONFIG += c++17

TEMPLATE = app

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage  -fno-access-control -fbranch-count-reg  -fno-exceptions

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt librsvg-2.0

PKGCONFIG += peony
LIBS += -lX11 -lpeony

PKGCONFIG += gio-2.0 gio-unix-2.0
LIBS +=-lgio-2.0 -lglib-2.0

#适配sdk
PKGCONFIG += kysdk-qtwidgets kysdk-waylandhelper kysdk-log kysdk-config kysdk-utils kysdk-datacollect kysdk-ukuiwindowhelper

#sdk不一样的版本路径有更改
INCLUDEPATH += /usr/include/kabase/
INCLUDEPATH += /usr/include/kysdk/applications/kabase/kylin_system/
INCLUDEPATH += /usr/include/kabase/kylin_system/
INCLUDEPATH += /usr/include/kysdk/applications/kabase/
#INCLUDEPATH += /usr/include/kylin-ai/coreai/vision/
#LIBS += -L/usr/lib/kysdk/kysdk-security -lkyfilesec

##opencv
#INCLUDEPATH += /usr/include/opencv4/
#LIBS += -lopencv_core \
#        -lopencv_imgcodecs \
#        -lopencv_imgproc \
#        -lopencv_highgui \
#        -lopencv_dnn \

#INCLUDEPATH += ../src \
#               ../codec \
#               ../codec/plugins/OCR

#contains(QMAKE_HOST.arch, x86_64){
#INCLUDEPATH += /usr/local/include/onnxruntime
#LIBS += -L/usr/local/lib -lonnxruntime
#}

#contains(QMAKE_HOST.arch, aarch64){
#INCLUDEPATH += /usr/local/include/onnxruntime
#LIBS += -L/usr/local/lib -lonnxruntime
#}

INCLUDEPATH += $$PWD/../src \
                $$PWD/../src/controller \
                $$PWD/../src/controller/core \
                $$PWD/../src/global \
                $$PWD/../src/model \
                $$PWD/../src/model/processing/ \
                $$PWD/../src/model/marktool/ \
                $$PWD/../src/model/file/ \
                $$PWD/../src/view \
                $$PWD/../codec/kylinimagecodec \
                $$PWD/../codec/kylinimagecodec/image_conver \
                $$PWD/../codec/kylinimagecodec/image_load \
                $$PWD/../codec/kylinimagecodec/image_save \
                $$PWD/../codec/kylinimagecodec/image_save \
                $$PWD/../codec/kylinimagecodec/image_save \
                $$PWD/../peony-rotate-menu-plugin \

SOURCES += $$PWD/../tests/tst_mytest.cpp \ #
           $$PWD/../src/controller/core/core.cpp \
           $$PWD/../src/controller/core/coreinteraction.cpp \
           $$PWD/../src/controller/core/multitiffoperate.cpp \
           $$PWD/../src/controller/core/albumthumbnail.cpp \
           $$PWD/../src/controller/core/ocr.cpp \
           $$PWD/../src/global/variable.cpp \
           $$PWD/../src/model/marktool/marktool.cpp \
           $$PWD/../src/model/file/file.cpp \
           $$PWD/../src/model/processing/processing.cpp \
           $$PWD/../src/model/processing/flip.cpp \
           $$PWD/../src/model/dbus.cpp \
           $$PWD/../peony-rotate-menu-plugin/rotate-menu-plugin.cpp \
           $$PWD/../peony-rotate-menu-plugin/rotation.cpp
#           ../src/view/colordialog.cpp \
#           ../src/view/colorsliderstyle.cpp \
#           ../src/view/colorsquare.cpp \
#           ../src/view/gradientslider.cpp \
#           ../src/view/maskwidget.cpp \
#           ../src/view/multitifftoolbtn.cpp \
#           ../src/view/brushsettings.cpp \
#           ../src/view/marking.cpp \
#           ../src/view/ocrresultwidget.cpp \
#           ../src/view/cutwidget.cpp \
#           ../src/view/edit.cpp \
#           ../src/view/information.cpp \
#           ../src/view/kyview.cpp \
#           ../src/view/menumodule.cpp \
#           ../src/view/navigator.cpp \
#           ../src/view/openimage.cpp \
#           ../src/view/painterthick.cpp \
#           ../src/view/replacebackgroundwidget.cpp \
#           ../src/view/scanimagewidget.cpp \
#           ../src/view/showimagewidget.cpp \
#           ../src/view/sidebar.cpp \
#           ../src/view/sidebardelegate.cpp \
#           ../src/view/sidelist.cpp \
#           ../src/view/titlebar.cpp \
#           ../src/view/toolbar.cpp \
#           ../src/view/multitifftoolwid.cpp \
#           ../src/imageproviderthumb.cpp \
#           ../src/mattingimageprovider.cpp \
#           ../src/global/horizontalorverticalmode.cpp \
#           ../src/global/mattinginteractiveqml.cpp \
#           ../src/global/scannerinteractiveqml.cpp \
#           ../src/global/interactiveqml.cpp \

HEADERS += $$PWD/../src/controller/interaction.h \
           $$PWD/../src/controller/core/core.h \
           $$PWD/../src/controller/core/coreinteraction.h \
           $$PWD/../src/controller/core/multitiffoperate.h \
           $$PWD/../src/controller/core/albumthumbnail.h \
           $$PWD/../src/controller/core/ocr.h \
           $$PWD/../src/controller/core/base.h \
           $$PWD/../src/global/variable.h \
           $$PWD/../src/model/marktool/marktool.h \
           $$PWD/../src/model/file/file.h \
           $$PWD/../src/model/processing/processing.h \
           $$PWD/../src/model/processing/processingbase.h \
           $$PWD/../src/model/processing/flip.h \
           $$PWD/../src/model/dbus.h \
           $$PWD/../peony-rotate-menu-plugin/rotate-menu-plugin.h \
           $$PWD/../peony-rotate-menu-plugin/rotation.h \
           $$PWD/../peony-rotate-menu-plugin/peony-rotate-menu-plugin_global.h
#           ../src/view/colordialog.h \
#           ../src/view/colorsliderstyle.h \
#           ../src/view/colorsquare.h \
#           ../src/view/gradientslider.h \
#           ../src/view/maskwidget.h \
#           ../src/view/multitifftoolbtn.h \
#           ../src/view/brushsettings.h \
#           ../src/view/marking.h \
#           ../src/view/ocrresultwidget.h \
#           ../src/view/cutwidget.h \
#           ../src/view/edit.h \
#           ../src/view/information.h \
#           ../src/view/kyview.h \
#           ../src/view/menumodule.h \
#           ../src/view/navigator.h \
#           ../src/view/openimage.h \
#           ../src/view/painterthick.h \
#           ../src/view/replacebackgroundwidget.h \
#           ../src/view/scanimagewidget.h \
#           ../src/view/showimagewidget.h \
#           ../src/view/sidebar.h \
#           ../src/view/sidebardelegate.h \
#           ../src/view/sidelist.h \
#           ../src/view/sizedate.h \
#           ../src/view/titlebar.h \
#           ../src/view/toolbar.h \
#           ../src/view/multitifftoolwid.h \
#           ../src/imageprovider.h \
#           ../src/imageproviderthumb.h \
#           ../src/mattingimageprovider.h \
#           ../src/scannerimageprovider.h \
#           ../src/global/horizontalorverticalmode.h \
#           ../src/global/mattinginteractiveqml.h \
#           ../src/global/scannerinteractiveqml.h \
#           ../src/global/interactiveqml.h \

#RESOURCES += \
#    testimage/testimage.qrc

MOC_DIR = $$PWD/../tests
OBJECTS_DIR = $$PWD/../tests
