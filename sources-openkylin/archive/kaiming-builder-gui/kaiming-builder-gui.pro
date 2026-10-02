QT       += core gui
QT += concurrent

# 适配窗口管理器圆角阴影
QT += KWindowSystem x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 link_pkgconfig

LIBS +=-lX11

PKGCONFIG += yaml-cpp

QMAKE_CXXFLAGS += -Wall -ggdb -Wno-implicit-fallthrough -Wno-return-local-addr -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIC -fPIE -pie -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack
QMAKE_LFLAGS += -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    basedialog.cpp \
    baseinfoframe.cpp \
    buildoptionsdialog.cpp \
    buildsettingdialog.cpp \
    buildsettingwidget.cpp \
    common/KMInstalledAppQuery.cpp \
    common/kmversion.cpp \
    common/subprocess.cpp \
    common/utils.cpp \
    component/chooselineedit.cpp \
    component/clicklabel.cpp \
    component/doubleclickwidget.cpp \
    component/keywordhighlighter.cpp \
    component/outputedit.cpp \
    componentwidget.cpp \
    debianconvert/debianconvertdialog.cpp \
    debianconvert/generateprojectpage.cpp \
    debianconvert/installenvpage.cpp \
    debianconvert/projectpathpage.cpp \
    debianconvert/selectenvpage.cpp \
    logic/BuildSettingManager.cpp \
    logic/HistoryManager.cpp \
    logic/KMBuilderApi.cpp \
    logic/KMBuilderManifest.cpp \
    logic/KMBuilderSearch.cpp \
    logic/KMBuilderStringUtils.cpp \
    logic/KMParseSummaryJson.cpp \
    logic/KMRef.cpp \
    logic/KMYAMLNodeHelper.cpp \
    main.cpp \
    mainwindow.cpp \
    manifestwidget.cpp \
    messageboxutils.cpp \
    outputwidget.cpp \
    packagelistdialog.cpp \
    permissiondialog.cpp \
    permissionwidget.cpp \
    selectdependsdialog.cpp \
    setupinfoframe.cpp \
    sourcedialog.cpp \
    stackedwidgetmanager.cpp \
    packageframe.cpp \
    startwidget.cpp \
    xatom-helper.cpp

HEADERS += \
    basedialog.h \
    baseinfoframe.h \
    buildoptionsdialog.h \
    buildsettingdialog.h \
    buildsettingwidget.h \
    common/KMInstalledAppQuery.h \
    common/dynamiccreator.h \
    common/kmversion.h \
    common/singleton.h \
    common/spinlock_mutex.h \
    common/subprocess.h \
    common/utils.h \
    component/chooselineedit.h \
    component/clicklabel.h \
    component/doubleclickwidget.h \
    component/keywordhighlighter.h \
    component/outputedit.h \
    componentwidget.h \
    debianconvert/debianconvertdialog.h \
    debianconvert/generateprojectpage.h \
    debianconvert/installenvpage.h \
    debianconvert/projectpathpage.h \
    debianconvert/selectenvpage.h \
    globalinfo.h \
    globalsignals.h \
    logic/BuildSettingManager.h \
    logic/HistoryManager.h \
    logic/KMBuilderApi.h \
    logic/KMBuilderManifest.h \
    logic/KMBuilderSearch.h \
    logic/KMBuilderStringUtils.h \
    logic/KMParseSummaryJson.h \
    logic/KMRef.h \
    logic/KMStructData.h \
    logic/KMYAMLNodeHelper.h \
    mainwindow.h \
    manifestwidget.h \
    messageboxutils.h \
    outputwidget.h \
    packagelistdialog.h \
    permissiondialog.h \
    permissionwidget.h \
    selectdependsdialog.h \
    setupinfoframe.h \
    sourcedialog.h \
    stackedwidgetmanager.h \
    packageframe.h \
    startwidget.h \
    xatom-helper.h

TRANSLATIONS += qt_zh_CN.ts
message($$system(./generate_translations_qm.sh))

RESOURCES += \
    app.qrc

# Default rules for deployment.

contains(CONFIG, KAIMING) {
    message("=== 启用 Kaiming 构建配置 ===")
    qnx: target.path = /opt/apps/top.openkylin.kaiming-builder-gui/files/bin/
    else: unix:!android: target.path = /opt/apps/top.openkylin.kaiming-builder-gui/files/bin/
    
    data.files += kaiming-builder-gui.desktop 
    data.path = /opt/apps/top.openkylin.kaiming-builder-gui/files/share/applications/
    
    icons.files += kaiming-builder-gui.png
    icons.path = /opt/apps/top.openkylin.kaiming-builder-gui/files/share/pixmaps/
    INSTALLS += icons 

} else {
    message("=== 启用默认构建配置 ===")
    qnx: target.path = /opt/kaiming-tools/bin
    else: unix:!android: target.path = /opt/kaiming-tools/bin
    
    data.files = data/kaiming-builder-gui.desktop \
                 kaiming-builder-gui.png
    data.path = /opt/kaiming-tools/share/kaiming-builder-gui
}
!isEmpty(target.path): INSTALLS += target
INSTALLS += data 

DISTFILES += \
    generate_translations_qm.sh

#DISTFILES += \
#    resources/images/kylin-id.svg \
#    resources/images/view-more-horizontal-symbolic.svg \
#    resources/symbos/preferences-system-search-symbolic.svg \
#    resources/symbos/window-close-symbolic.svg

FORMS +=

