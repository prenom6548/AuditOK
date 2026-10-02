TEMPLATE = subdirs
SUBDIRS =src \
         codec\
         peony-rotate-menu-plugin \
#         photo-viewer-aiassistant-plugin \
         tests

TRANSLATIONS += \
    translations/kylin-photo-viewer_zh_CN.ts \
    translations/kylin-photo-viewer_bo_CN.ts \
    translations/kylin-photo-viewer_ky.ts \
    translations/kylin-photo-viewer_kk.ts \
    translations/kylin-photo-viewer_ug.ts \
    translations/kylin-photo-viewer_mn.ts \
    translations/kylin-photo-viewer_zh_Hant.ts \
    translations/kylin-photo-viewer_vi.ts \
    translations/kylin-photo-viewer_ms.ts \
    translations/kylin-photo-viewer_th.ts \

!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate pm")
qm_files.files = translations/*.qm
qm_files.path = /usr/share/kylin-photo-viewer/translations/

#settings.files = data/org.kylin.photo.viewer.gschema.xml
settings.files += \
    $$PWD/data/org.kylin.photo.viewer.gschema.xml

settings.path = /usr/share/glib-2.0/schemas/

desktop.files = data/kylin-photo-viewer.desktop
desktop.path = /usr/share/applications/

guide.files = data/kylin-photo-viewer/
guide.path = /usr/share/kylin-user-guide/data/guide/

icons.files = res/kyview_logo.png
icons.path = /usr/share/pixmaps/

INSTALLS +=desktop settings icons qm_files guide

