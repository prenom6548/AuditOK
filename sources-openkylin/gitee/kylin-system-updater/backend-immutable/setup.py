#!/usr/bin/env python3
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
