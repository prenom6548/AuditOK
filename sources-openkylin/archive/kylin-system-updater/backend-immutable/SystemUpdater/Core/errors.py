# /*
#  * Copyright (C) 2020, KylinSoft Co., Ltd.
#  *
#  * This program is free software: you can redistribute it and/or modify
#  * it under the terms of the GNU General Public License as published by
#  * the Free Software Foundation, either version 3 of the License, or
#  * (at your option) any later version.
#  *
#  * This program is distributed in the hope that it will be useful,
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  * GNU General Public License for more details.
#  *
#  * You should have received a copy of the GNU General Public License
#  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
#  */

#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Exception classes"""

import logging
from .enums import *

class UpdateBaseError(Exception):

    """Internal error if a transaction could not be processed successfully."""

    _dbus_error_name = "org.debian.apt.TransactionFailed"

    def __init__(self, code,desc=None,*args):
        self.code = code
        
        if not desc:
            self.desc = get_error_description_from_enum(self.code)
        else:
            self.desc = desc

        Exception.__init__(self, *args)

    def __str__(self):
        return "%s" % \
            (get_error_description_from_enum(self.code))

class UpdateProgressExit(Exception):
    def __init__(self,*args):
        Exception.__init__(self, *args)
        logging.info("Update Progress wiil be Exit...")

class DebDeltaError(UpdateBaseError):

    """Internal error if a transaction could not be processed successfully."""

# vim:ts=4:sw=4:et
