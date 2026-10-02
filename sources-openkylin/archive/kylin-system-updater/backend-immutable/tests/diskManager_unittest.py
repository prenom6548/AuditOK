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

import unittest
import sys
import os
from SystemUpdater.Core.errors import *

# 添加项目根目录到 sys.path
from SystemUpdater.Core.diskManager import check_free_space
class TestCalculator(unittest.TestCase):
    # def test_diskManagerClassInit(self):
        # testclass = diskManager()
        # self.assertIsInstance(testclass, diskManager)

    def test_spaceCheckTrue(self):
        result = check_free_space(1000,1000)
        self.assertEqual(True, result)

    def test_spaceCheckFalse(self):
        with self.assertRaises(UpdateBaseError):
            result = check_free_space(9999999999999999,100000)

    def test_spaceCheckaddis0(self):
        result = check_free_space(100000)
        self.assertEqual(True, result)
    
    def test_spaceCheckaddnotis0(self):
        with self.assertRaises(UpdateBaseError):
            result = check_free_space(9999999999999999)

if __name__ == "__main__":
    unittest.main()