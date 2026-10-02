TEMPLATE = subdirs
SUBDIRS = \
    plugins \
    src \

QMAKE_CXXFLAGS += -g

contains(CONFIG, KAIMING){
   message("kaiming")
   qm_files.path = /opt/apps/top.openkylin.kylin-ipmsg/files/share/kylin-ipmsg/data/translations/
   desktop.path = /opt/apps/top.openkylin.kylin-ipmsg/files/share/applications/
   schemes.path = /opt/apps/top.openkylin.kylin-ipmsg/files/share/glib-2.0/schemas/
   database.path = /opt/apps/top.openkylin.kylin-ipmsg/files/share/kylin-ipmsg/data/database/
   guide.path = /opt/apps/top.openkylin.kylin-ipmsg/files/share/kylin-user-guide/data/guide/
} else {
   message("not kaiming")
   qm_files.path = /usr/share/kylin-ipmsg/data/translations/
   desktop.path = /usr/share/applications/
   schemes.path = /usr/share/glib-2.0/schemas/
   database.path = /usr/share/kylin-ipmsg/data/database/
   guide.path = /usr/share/kylin-user-guide/data/guide/
   DEFINES += NOTKAIMING
}
    
# translations
TRANSLATIONS += \
    data/translations/kylin-ipmsg_zh_CN.ts \
    data/translations/kylin-ipmsg_zh_HK.ts \
    data/translations/kylin-ipmsg_bo_CN.ts \
    data/translations/kylin-ipmsg_kk.ts \
    data/translations/kylin-ipmsg_ky.ts \
    data/translations/kylin-ipmsg_ug.ts \
    data/translations/kylin-ipmsg_mn.ts \
    data/translations/kylin-ipmsg_vi.ts \
    data/translations/kylin-ipmsg_th.ts \
    data/translations/kylin-ipmsg_ms.ts

!system($$PWD/data/translations/generate_translations_pm.sh): error("Failed to generate pm")

qm_files.files = data/translations/*.qm
#qm_files.path = /usr/share/kylin-ipmsg/data/translations/
CONFIG += lrelease

RESOURCES += \
    res.qrc

# gsettings
schemes.files += \
    $$PWD/data/schemas/org.kylin-ipmsg-data.gschema.xml \
    $$PWD/data/schemas/org.ukui.log4qt.kylin-ipmsg.gschema.xml
#schemes.path = /usr/share/glib-2.0/schemas/

desktop.files = data/kylin-ipmsg.desktop
#desktop.path = /usr/share/applications/

database.files = data/database/kylin-ipmsg.db
#database.path = /usr/share/kylin-ipmsg/data/database/

INSTALLS +=  \
    desktop  \
    schemes  \
    qm_files \
    database

# V10Pro使用自定义用户手册
greaterThan(QT_MAJOR_VERSION, 5) | greaterThan(QT_MINOR_VERSION, 9) {
    # message("QT_VERSION ("$$QT_VERSION")")
    DEFINES   += __V10Pro__
    #guide.path = /usr/share/kylin-user-guide/data/guide/
    guide.files = data/guide/kylin-ipmsg/

    INSTALLS += guide
}
