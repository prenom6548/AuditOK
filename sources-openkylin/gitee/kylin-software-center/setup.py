 #!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import glob
from setuptools import setup
import DistUtilsExtra.command.build_extra
import DistUtilsExtra.command.build_i18n
import DistUtilsExtra.command.clean_i18n
from subprocess import call
#from DistUtilsExtra.command import build_extra
#import build_i18n_ext as build_i18n


PO_DIR = 'po'

for po in glob.glob(os.path.join(PO_DIR, '*.po')):
    lang = os.path.basename(po[:-3])
    mo = os.path.join(PO_DIR, 'kylin-software-center.mo')
    target_dir = os.path.dirname(mo)
    if not os.path.isdir(target_dir):
        os.makedirs(target_dir)
    try:
        return_code = call(['msgfmt', '-o', mo, po])
    except OSError:
        print('Translation not available, please install gettext')
        break
    if return_code:
        raise Warning('Error when building locales')
cmdclass ={
            "build" : DistUtilsExtra.command.build_extra.build_extra,
            "build_i18n" :  DistUtilsExtra.command.build_i18n.build_i18n,
            #"clean": DistUtilsExtra.command.clean_i18n.clean_i18n,
}


data_files=[
    ('bin/', ['ubuntu-kylin-software-center']),
    ('../etc/dbus-1/system.d/', ['backend/aptdaemon/conf/com.kylin.softwarecenter.conf']),
    ('share/dbus-1/system-services/', ['backend/aptdaemon/conf/com.kylin.softwarecenter.service']),
    ('share/polkit-1/actions/', ['backend/aptdaemon/conf/com.kylin.softwarecenter.policy']),
    ('lib/python3/dist-packages/kylin-software-center-daemon/', glob.glob('backend/aptdaemon/dbus_service')),
    ('share/applications/',['kylin-software-center.desktop']),
    ('share/pixmaps/',['kylin-software-center.png']),
    ('../etc/rsyslog.d/',['58-kylin-software-center.conf']),
    ('share/icons/hicolor/128x128/apps/',['kylin-software-center.png']),
    ('share/kylin-software-center/backend', glob.glob('backend/*.py')),
    ('share/kylin-software-center/backend/service/', glob.glob('backend/service/*.txt')),
    ('share/kylin-software-center/backend/run_app/', glob.glob('backend/run_app/*.py')),
    ('share/kylin-software-center/backend/service/', glob.glob('backend/service/*.py')),
    ('share/kylin-software-center/backend/service/', glob.glob('backend/service/*.txt')),
    ('share/kylin-software-center/data/ads/', glob.glob('data/ads/*.png')),
    ('share/kylin-software-center/', ['kylin-software-center.conf']),
    ('share/kylin-software-center/data/icons/', glob.glob('data/icons/*.png')),
    ('share/kylin-software-center/data/coverimage/', glob.glob('data/coverimage/*')),
    ('share/kylin-software-center/data/new_arrivals/', glob.glob('data/new_arrivals/*')),
    ('share/kylin-software-center/data/user_profile/', glob.glob('data/user_profile/*')),
    ('share/kylin-software-center/data/clientAPItest/', glob.glob('data/clientAPItest/*')),
    ('share/kylin-software-center/data/winicons/', glob.glob('data/winicons/*')),
    ('share/kylin-software-center/data/', ['data/uksc.db']),
    ('share/kylin-software-center/data/clientAPItest/', glob.glob('data/clientAPItest/*')),
    ('share/kylin-software-center/models/', glob.glob('models/*')),
    ('share/kylin-software-center/res/', glob.glob('res/*.png')),
    ('share/kylin-software-center/res/', glob.glob('res/*.gif')),
    ('share/kylin-software-center/res/uninstall/', glob.glob('res/uninstall/*')),
    ('share/kylin-software-center/ui/', glob.glob('ui/*')),
    ('share/kylin-software-center/utils/', glob.glob('utils/*')),
    ('share/kylin-software-center/po/', glob.glob('po/*')),
    ('share/kylin-software-center/kydroid/', glob.glob('kydroid/*')),
    ('share/kylin-software-center/service/', glob.glob('service/*')),
    ('share/kylin-software-center/',['kylin-software-center.py']),
    ('share/kylin-user-guide/data/guide/kylin-store/zh_CN/image/',glob.glob('kylin-software-center-help/zh_CN/image/*')),
    ('share/kylin-user-guide/data/guide/kylin-store/',['kylin-software-center-help/software-store.png']),
    ('share/kylin-user-guide/data/guide/kylin-store/zh_CN/',['kylin-software-center-help/zh_CN/index.md']),
    ('share/kylin-user-guide/data/guide/kylin-store/en_US/image/',glob.glob('kylin-software-center-help/en_US/image/*')),
    ('share/kylin-user-guide/data/guide/kylin-store/en_US/',['kylin-software-center-help/en_US/index.md']),
    # ('../etc/xdg/autostart/',['kylin-software-center-autostart.desktop']),
    ('lib/systemd/user/',['kylin-software-center.service']),
    ('/lib/systemd/system/',['backend/aptdaemon/conf/softwarecenter-daemon.service']),
    ('share/dbus-1/services/', ['backend/utildaemon/conf/com.kylin.utiliface.service']),
    ]

#
# 函数名:找.mo文件
# Function: find .mo file
# 
def find_mo_files():
    data_files = []
    for mo in glob.glob(os.path.join(PO_DIR, '*', 'kylin-software-center.mo')):
        lang = os.path.basename(os.path.dirname(mo))
        dest = os.path.join('share', 'locale', lang, 'LC_MESSAGES')
        data_files.append((dest, [mo]))
    return data_files

data_files.extend(find_mo_files())



setup(name="kylin-software-center",
    version="1.3.10",
    author="Ubuntu Kylin Team",
    author_email="ubuntukylin-members@list.launchpad.net",
    url="https://launchpad.net/kylin-software-center",
    license="GNU General Public License (GPL)",
    packages = [ 'kylin_software_center_daemon',],
    package_dir = {
        '': '.',
    },
    install_requires = [ 'setuptools', ],
    cmdclass = cmdclass,
    data_files=data_files,
)

