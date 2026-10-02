#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""Main loop for aptdaemon."""

__all__ = ("mainloop", "get_main_loop")

from gi.repository import GLib

mainloop = GLib.MainLoop()

def get_main_loop():
    """Return the glib main loop as a singleton."""
    return mainloop