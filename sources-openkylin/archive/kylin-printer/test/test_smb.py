#!/bin/python3

import smbc
# import pysmb
import errno
import os


class AuthContext:
    def __init__ (self, parent=None, workgroup='', user='', passwd=''):
        self.passes = 0
        self.has_failed = False
        self.auth_called = False
        self.tried_guest = False
        self.cancel = False
        self.use_user = user
        self.use_password = passwd
        self.use_workgroup = workgroup
        self.dialog_shown = False
        self.parent = parent


    def callback (self, server, share, workgroup, user, password):
        self.auth_called = True
        self.for_server = server
        self.for_share = share

        if self.use_user:
            if self.use_workgroup:
                workgroup = self.use_workgroup

            return (workgroup, self.use_user, self.use_password)

        user = ''
        password = ''
        return (workgroup, user, password)

debug = 0
smbc_auth = AuthContext(None, 'WORKGROUP','kylin', 'kk123123')
ctx = smbc.Context (debug=debug,
                    auth_fn=smbc_auth.callback)

entries = None

try:
    entries = ctx.opendir ("smb://10.41.120.174").getdents ()
except Exception as e:
    smbc_auth.failed (e)

if entries:
    for entry in entries:
        print(entry.name)
        # if entry.smbc_type in [smbc.PRINTER_SHARE]:
        #     print(entry.name)
# ctx.optionUseKerberos = True
f = ctx.open ("smb://%s/%s" % ('10.41.120.174', 'HP-LaserJet-M408'),
                              os.O_RDWR, 0o777)
        

# smbclient -L smb://10.41.120.171 -N


# lpadmin -p aaa -E -v smb://kylin:kk123123@10.41.120.171/Generic-CUPS-BRF-Printer -m drv:///cupsfilters.drv/textonly.ppd


#172.20.107.254:8888/api/v1/addUserPpd