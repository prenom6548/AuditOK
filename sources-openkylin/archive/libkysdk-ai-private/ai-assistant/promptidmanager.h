/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef PROMPTIDMANAGER_H
#define PROMPTIDMANAGER_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

class PromptIdManager {
public:
    PromptIdManager();

    void setDefaultPromptIdList(const std::list<int>& idList);
    std::list<int> promptIdList();
    bool addPromptId(int id);
    bool deletePromptId(int id);
    bool updatePromptIdOrder(const std::list<int>& newOrder);

private:
    void loadPromptIdList();
    bool savePromptIdList();

    void createConfigFile();
    void readConfigFile();

private:
    const std::string configDir_ =
        std::string(getenv("HOME")) + "/.config/kylin-ai";
    const std::string configPath_ =
        std::string(getenv("HOME")) + "/.config/kylin-ai/promptOrder.conf";
    std::list<int> promptIdList_;
};

#endif
