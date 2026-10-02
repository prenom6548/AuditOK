#!/bin/bash
xgettext --output=new.pot ./src/notifications.c
msgmerge --update ./po/zh_CN/kylin-system-updater-notify-new.po ./new.pot
msgfmt ./po/zh_CN/kylin-system-updater-notify-new.po -o ./mo/zh_CN/kylin-system-updater-notify-new.mo
msgmerge --update ./po/zh_HK/kylin-system-updater-notify-new.po ./new.pot
msgfmt ./po/zh_HK/kylin-system-updater-notify-new.po -o ./mo/zh_HK/kylin-system-updater-notify-new.mo
msgmerge --update ./po/bo_CN/kylin-system-updater-notify-new.po ./new.pot
msgfmt ./po/bo_CN/kylin-system-updater-notify-new.po -o ./mo/bo_CN/kylin-system-updater-notify-new.mo
msgmerge --update ./po/mn/kylin-system-updater-notify-new.po ./new.pot
msgfmt ./po/mn/kylin-system-updater-notify-new.po -o ./mo/mn/kylin-system-updater-notify-new.mo
msgmerge --update ./po/ug/kylin-system-updater-notify-new.po ./new.pot
msgfmt ./po/ug/kylin-system-updater-notify-new.po -o ./mo/ug/kylin-system-updater-notify-new.mo
msgmerge --update ./po/ky/kylin-system-updater-notify-new.po ./new.pot
msgfmt ./po/ky/kylin-system-updater-notify-new.po -o ./mo/ky/kylin-system-updater-notify-new.mo
msgmerge --update ./po/kk/kylin-system-updater-notify-new.po ./new.pot
msgfmt ./po/kk/kylin-system-updater-notify-new.po -o ./mo/kk/kylin-system-updater-notify-new.mo
msgmerge --update ./po/de/kylin-system-updater-notify-new.po ./new.pot
msgfmt ./po/de/kylin-system-updater-notify-new.po -o ./mo/de/kylin-system-updater-notify-new.mo
msgmerge --update ./po/fr/kylin-system-updater-notify-new.po ./new.pot
msgfmt ./po/fr/kylin-system-updater-notify-new.po -o ./mo/fr/kylin-system-updater-notify-new.mo
msgmerge --update ./po/es/kylin-system-updater-notify-new.po ./new.pot
msgfmt ./po/es/kylin-system-updater-notify-new.po -o ./mo/es/kylin-system-updater-notify-new.mo
