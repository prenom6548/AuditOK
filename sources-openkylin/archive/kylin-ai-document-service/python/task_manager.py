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

import logging

class TaskManager:
    def __init__(self):
        logging.basicConfig(level=logging.INFO)

    def create_task_id(self,request):
        # 生成唯一任务ID
        task_id = request.file_path
        return task_id

    def receive_task(self, request):
        logging.info(f"Received task: {request.file_path}")
        task_id = self.create_task_id(request=request)
        return task_id
