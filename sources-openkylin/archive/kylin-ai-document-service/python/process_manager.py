# /*
#  * Copyright 2024 KylinSoft Co., Ltd.
#  *
#  * This program is free software: you can redistribute it and/or modify it under
#  * the terms of the GNU General Public License as published by the Free Software
#  * Foundation, either version 3 of the License, or (at your option) any later
#  * version.
#  *
#  * This program is distributed in the hope that it will be useful, but WITHOUT
#  * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
#  * details.
#  *
#  * You should have received a copy of the GNU General Public License along with
#  * this program. If not, see <https://www.gnu.org/licenses/>.
#  */

import multiprocessing
import logging

class ProcessManager:
    def __init__(self):
        logging.basicConfig(level=logging.INFO)

    def run_process(self, target, args):
        pool = multiprocessing.Pool()
        async_result = pool.apply_async()
        file_path, result = async_result.get()
        pool.close()
        pool.join()
        # process = multiprocessing.Pool(target=target, args=args)
        return async_result
