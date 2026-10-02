QT += widgets core sql x11extras KWindowSystem dbus svg

TEMPLATE = lib
DEFINES += GUI_LIBRARY

CONFIG += c++11

VERSION = 1.2.0

# 适配窗口管理器圆角阴影
LIBS +=-lX11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += USE_UKUI_PROTOCOL
DEFINES += USE_UKUI_SHELL_PLUGIN

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

INCLUDEPATH += ../kysdk-waylandhelper/src
LIBS += -L../kysdk-waylandhelper -lkysdk-waylandhelper
LIBS += -L../kysdk-waylandhelper/src/kysdk-ukuiwindowhelper -lkysdk-ukuiwindowhelper

TRANSLATIONS += ./translations/gui_zh_CN.ts \
                ./translations/gui_bo_CN.ts \
                ./translations/gui_zh_HK.ts \
                ./translations/gui_mn.ts \
                ./translations/gui_de.ts \
                ./translations/gui_es.ts \
                ./translations/gui_fr.ts \
                ./translations/gui_kk.ts \
                ./translations/gui_ug.ts \
                ./translations/gui_ky.ts

system(lrelease ./translations/gui_zh_CN.ts ./translations/gui_zh_CN.qm)
system(lrelease ./translations/gui_bo_CN.ts ./translations/gui_bo_CN.qm)
system(lrelease ./translations/gui_zh_HK.ts ./translations/gui_zh_HK.qm)
system(lrelease ./translations/gui_mn.ts ./translations/gui_mn.qm)
system(lrelease ./translations/gui_de.ts ./translations/gui_de.qm)
system(lrelease ./translations/gui_es.ts ./translations/gui_es.qm)
system(lrelease ./translations/gui_fr.ts ./translations/gui_fr.qm)
system(lrelease ./translations/gui_kk.ts ./translations/gui_kk.qm)
system(lrelease ./translations/gui_ug.ts ./translations/gui_ug.qm)
system(lrelease ./translations/gui_ky.ts ./translations/gui_ky.qm)


SOURCES += \
    src/kaboutdialog.cpp \
    src/kaddfilebutton.cpp \
    src/karrowtabbar.cpp \
    src/kbackgroundgroup.cpp\
    src/kbadge.cpp \
    src/kballontip.cpp \
    src/kborderbutton.cpp \
    src/kborderlessbutton.cpp \
    src/kbreadcrumb.cpp \
    src/kbubblewidget.cpp\
    src/kbuttonbox.cpp\
    src/kclocktimer.cpp \
    src/kcolorbutton.cpp\
    src/kcolorcombobox.cpp \
    src/kcomboboxwidget.cpp \
    src/kcommentpanel.cpp \
    src/kcrumb.cpp \
    src/kcustomdateedit.cpp \
    src/kcustomtabbar.cpp \
    src/kdialog.cpp \
    src/kdirtabbar.cpp \
    src/kdragwidget.cpp \
    src/kfilewidget.cpp \
    src/kiconbar.cpp \
    src/kinputdialog.cpp \
    src/kintrowidget.cpp \
    src/kitemwidget.cpp\
    src/klabel.cpp \
    src/klineedit.cpp \
    src/klineframe.cpp\
    src/klistview.cpp\
    src/klistviewdelegate.cpp\
    src/klistwidget.cpp\
    src/kloopplaypage.cpp \
    src/klunarcalendarinfo.cpp \
    src/klunardateedit.cpp \
    src/kmenubutton.cpp \
    src/kmessagebox.cpp\
    src/kminicalendaritem.cpp \
    src/kminicalendarwidget.cpp \
    src/knativetabbar.cpp \
    src/knavigationbar.cpp \
    src/knavigationtrail.cpp \
    src/kpasswordedit.cpp \
    src/kpixmapcontainer.cpp \
    src/kpressbutton.cpp\
    src/kprogressbar.cpp \
    src/kprogresscircle.cpp \
    src/kprogressdialog.cpp \
    src/kprogresslabel.cpp \
    src/kpushbutton.cpp\
    src/ksearchlineedit.cpp \
    src/ksecuritylevelbar.cpp \
    src/ksecurityquestiondialog.cpp \
    src/kshadowhelper.cpp\
    src/kslider.cpp \
    src/kswitchbutton.cpp \
    src/ktabbar.cpp \
    src/ktableheaderview.cpp \
    src/ktablemodel.cpp \
    src/ktableview.cpp \
    src/ktag.cpp \
    src/ktextedit.cpp \
    src/ktexteditor.cpp \
    src/ktitlewindow.cpp \
    src/ktoolbar.cpp \
    src/ktoolbutton.cpp \
    src/ktranslucentfloor.cpp\
    src/kuninstalldialog.cpp \
    src/kwidget.cpp \
    src/kwindowbuttonbar.cpp \
    src/parmscontroller.cpp\
    src/themeController.cpp \
    src/xatom-helper.cpp \
    src/kaddbutton.cpp \
    src/kbaseframe.cpp \
    src/kcomboboxframe.cpp \
    src/kflowlayout.cpp \
    src/klineeditframe.cpp \
    src/kpushbuttonframe.cpp \
    src/kradiobuttonframe.cpp \
    src/ksettinggroup.cpp \
    src/ksliderframe.cpp \
    src/kswitchbuttonframe.cpp \

HEADERS += \
    src/gui_g.h \
    src/kaboutdialog.h \
    src/kaddfilebutton.h \
    src/karrowtabbar.h \
    src/kbackgroundgroup.h\
    src/kbadge.h \
    src/kballontip.h \
    src/kborderbutton.h \
    src/kborderlessbutton.h \
    src/kbreadcrumb.h \
    src/kbubblewidget.h\
    src/kbuttonbox.h\
    src/kclocktimer.h \
    src/kcolorbutton.h\
    src/kcolorcombobox.h \
    src/kcomboboxwidget.h \
    src/kcommentpanel.h \
    src/kcrumb.h \
    src/kcustomdateedit.h \
    src/kcustomtabbar.h \
    src/kdialog.h \
    src/kdirtabbar.h \
    src/kdragwidget.h \
    src/kfilewidget.h \
    src/kiconbar.h \
    src/kinputdialog.h \
    src/kintrowidget.h \
    src/kitemwidget.h\
    src/klabel.h \
    src/klineedit.h \
    src/klineframe.h\
    src/klistviewdelegate.h\
    src/klistview.h\
    src/klistwidget.h\
    src/kloopplaypage.h \
    src/klunarcalendarinfo.h \
    src/klunardateedit.h \
    src/kmenubutton.h \
    src/kmessagebox.h\
    src/kminicalendaritem.h \
    src/kminicalendarwidget.h \
    src/knativetabbar.h \
    src/knavigationbar.h \
    src/knavigationtrail.h \
    src/kpasswordedit.h \
    src/kpixmapcontainer.h \
    src/kpressbutton.h\
    src/kprogressbar.h \
    src/kprogresscircle.h \
    src/kprogressdialog.h \
    src/kprogresslabel.h \
    src/kpushbutton.h\
    src/ksearchlineedit.h \
    src/ksecuritylevelbar.h \
    src/ksecurityquestiondialog.h \
    src/kshadowhelper.h\
    src/kslider.h \
    src/kswitchbutton.h \
    src/ktabbar.h \
    src/ktableheaderview.h \
    src/ktablemodel.h \
    src/ktableview.h \
    src/ktag.h \
    src/ktextedit.h \
    src/ktexteditor.h \
    src/ktitlewindow.h \
    src/ktoolbar.h \
    src/ktoolbutton.h \
    src/ktranslucentfloor.h\
    src/kuninstalldialog.h \
    src/kwidget.h \
    src/kwindowbuttonbar.h \
    src/parmscontroller.h\
    src/themeController.h \
    src/accessinfohelper.h \
    src/xatom-helper.h \
    src/kaddbutton.h \
    src/kbaseframe.h \
    src/kcomboboxframe.h \
    src/kflowlayout.h \
    src/klineeditframe.h \
    src/kpushbuttonframe.h \
    src/kradiobuttonframe.h \
    src/ksettinggroup.h \
    src/ksliderframe.h \
    src/kswitchbuttonframe.h \
   
# Default rules for deployment.
headers.files = $${HEADERS}
headers.path = /usr/include/kysdk/applications/
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers

RESOURCES += \
    sdk_res.qrc

DISTFILES += \
    translations/gui_bo_CN.ts
