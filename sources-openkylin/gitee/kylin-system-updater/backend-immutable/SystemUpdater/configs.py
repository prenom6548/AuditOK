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

import json
import logging
DEFAULT_SETTINGS = "/etc/kylin-system-updater/settings-default.json"
RUN_SETTINGS = "/var/cache/kylin-system-updater/settings.json"
class JSONConfig:
    def __init__(self, settings_files = []):
        self.configs = {}

        # 循环读取所有的配置文件，并将配置进行合并，最后的配置文件优先级最高
        for config_file in settings_files:
            try:
                with open(config_file, "r") as f:
                    config_data = json.load(f)
                    self.configs = {**self.configs, **config_data}  # 使用字典解包进行合并
            except Exception as e:
                pass

        self.settings_files = settings_files
        self.skip_log = ["history", "update_history","system"]

    def get(self, key, default=None):
        return self.configs.get(key, default)

    def set(self, key, value):
        self.configs[key] = value

    def __getitem__(self, key):
        return self.configs[key]

    def __setitem__(self, key, value):
        if key not in self.skip_log:
            logging.info(f"Settings: setting '{key}' to '{value}'")
        self.configs[key] = value
        self.save()

    def update(self, new_data):
        if isinstance(new_data, dict):
            self.configs.update(new_data)
        else:
            raise TypeError("update() expects a dictionary")
        
    def save(self):
        with open(self.settings_files[-1], 'w') as f:
            json.dump(self.configs, f, indent=4)

"""
 使用方式：
    获取值
    1、settings.get("name")  == "Bob"
    2、settings.get("name1", "default") == "default"
    3、settings.get("local",{}).get("pre",{}).get("en") == "xxxxx"

    设置值
    1、settings.set("name", "Bob")
    # 仅支持一级设置
    2、settings["name"] = "Bob"
    3、支持多级设置
        a、locals = settings.get("local",{})
        b、locals["pre"]["en"] = "xxxxx"

    保存
    settings.save()
"""

settings = JSONConfig(
    settings_files=[
        DEFAULT_SETTINGS,
        RUN_SETTINGS,
    ]
)