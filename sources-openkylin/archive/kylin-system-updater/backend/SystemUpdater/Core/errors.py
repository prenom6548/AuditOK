#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Exception classes"""

# __all__ = ("UpdateBaseError")

import logging
from selectors import EpollSelector
import sys
from .enums import *

PY3K = sys.version_info.major > 2

class UpdateBaseError(Exception):

    """Internal error if a transaction could not be processed successfully."""

    _dbus_error_name = "org.debian.apt.TransactionFailed"

    def __init__(self, code,header=None,desc=None,details="",*args):
        if not args:
            # Avoid string replacements if not used
            details = details.replace("%", "%%")
        args = tuple([_convert_unicode(arg) for arg in args])
        details = _convert_unicode(details)
        self.code = code
        self.details = details
        self.details_args = args
        if header == None:
            self.header = get_error_string_from_enum(self.code)
        else:
            self.header = header
        
        if desc == None:
            self.desc = get_error_description_from_enum(self.code)
        else:
            self.desc = desc

        Exception.__init__(self, *args)
        # AptDaemonError.__init__(self, "%s: %s" % (code, details % args))

    def __unicode__(self):
        return "%s" % \
               (get_error_string_from_enum(self.code))

    def __str__(self):
        if PY3K:
            return self.__unicode__()
        else:
            return self.__unicode__().encode("utf-8")

class UpdateProgressExit(Exception):
    def __init__(self,*args):
        Exception.__init__(self, *args)
        logging.info("Update Progress wiil be Exit...")

def _convert_unicode(text, encoding="UTF-8"):
    """Always return an unicode."""
    if PY3K and not isinstance(text, str):
        text = str(text, encoding, errors="ignore")
    elif not PY3K and not isinstance(text, unicode):
        text = unicode(text, encoding, errors="ignore")
    return text

# vim:ts=4:sw=4:et
