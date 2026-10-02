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
import re
import json
import logging
import datetime
from datetime import datetime

from SystemUpdater.Core.Database import Sqlite3Server

LOG_FILE_PATH = "/var/log/kylin-system-updater/kylin-system-updater.log.1"

logging.basicConfig(
    format=f'%(asctime)s %(levelname)s: %(message)s',
    level=logging.INFO
)

class sqliteTests():
    def __init__(self):
        self.sqlite3_server = Sqlite3Server(self)
    
    def test_insert_into_display(self):
        # insert: check_time
        self.sqlite3_server.insert_into_display("check_time", create_time_stamp()[0:-4])

class dataAcqTests():
    def __init__(self):
        pass

    def check_upload_format(self):
        def _sys_transmit_mode():
            message_template = {
                "packageName": "",
                "messageType": "",
                "createTimeStamp": "",
                "old_version": "",
                "new_version": "",
                "source": "",
                "updater_version": "",
                "status": "",
                "errorCode": "",
                "error_string": "",
                "error_desc": "",
                "old_update_version": "",
                "new_update_version": "",
                "serialNumber": "",
                "updateID": "",
                "transactionID": "",
                "upgrade_mode": "",
                "type": "",
                "RegisterNumber": "",
                "UUID": ""
            }
            match_count = 1
            missing_fields = []

            with open(LOG_FILE_PATH, "r") as f:
                lines = f.readlines()
            for line in lines:
                if "Send to sys_transmit" in line:
                    logging.info("Find the reported data *********************************************** (%d):", match_count)
                    # match = re.search(r'< ({.*}) >', line)
                    # match = re.match(r"(\d{2}-\d{2},\d{2}:\d{2}:\d{2}) .*?Send to sys_transmit: < (.*)>", line)
                    match = ""
                    if match:
                        log_time = match.group(1)  # 获取时间
                        json_str = match.group(2)  # 获取 JSON 字符串
                        json_dict = json.loads(json_str)
                        message_dict = json.loads(json_dict["message"])

                        for head_msg in ["time","hostname","ip","name","lv","message"]:
                            if head_msg not in json_dict.keys():
                                logging.warning("%s not in info: %s", line)

                        for mt in message_template.keys():
                            if mt not in message_dict.keys():
                                missing_fields.append(mt)
                            else:
                                message_dict.pop(mt)

                        if missing_fields:
                            logging.info("missing fields <%s>: %s", log_time, missing_fields)
                        if message_dict:
                            logging.info("extraneous fields <%s>: %s", log_time, message_dict)
                        if not missing_fields and not message_dict:
                            logging.info("ok - <%s>", log_time)

                    match_count += 1
                    missing_fields = []

        _sys_transmit_mode()


def create_time_stamp():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]

if __name__ == "__main__":
    sqliteTests().test_insert_into_display()
    dataAcqTests().check_upload_format()
