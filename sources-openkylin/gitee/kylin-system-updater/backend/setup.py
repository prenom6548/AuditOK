#!/usr/bin/env python3
# -*- Mode: Python; indent-tabs-mode: nil; tab-width: 4; coding: utf-8 -*-
from distutils.core import setup
from DistUtilsExtra.command import (
    build_extra, build_i18n, build_help)

disabled = []
class CustomBuild(build_extra.build_extra):
    def run(self):
        build_extra.build_extra.run(self)

setup(
    packages=[  'SystemUpdater',
                'SystemUpdater.backend',
                'SystemUpdater.Core'   
    ],
    cmdclass={  "build": CustomBuild,
                "build_i18n": build_i18n.build_i18n
                # "build_help": build_help.build_help
    }
)
