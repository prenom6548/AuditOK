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


#include "promptidmanager.h"

#include <sys/stat.h>

#include <algorithm>
#include <filesystem>

PromptIdManager::PromptIdManager() { loadPromptIdList(); }

void PromptIdManager::setDefaultPromptIdList(const std::list<int>& idList) {
    if (!promptIdList_.empty()) {
        return;
    }
    promptIdList_ = idList;
    savePromptIdList();
}

std::list<int> PromptIdManager::promptIdList() { return promptIdList_; }

bool PromptIdManager::addPromptId(int id) {
    if (std::find(promptIdList_.begin(), promptIdList_.end(), id) !=
        promptIdList_.end()) {
        fprintf(stderr, "Add prompt failed, %i %s", id, "already exists!\n");
        return false;
    }

    promptIdList_.push_back(id);
    return savePromptIdList();
}

bool PromptIdManager::deletePromptId(int id) {
    if (std::find(promptIdList_.begin(), promptIdList_.end(), id) ==
        promptIdList_.end()) {
        fprintf(stderr, "Failed to delete prompt word id, %i %s", id,
                "not exists!\n");
        return false;
    }

    promptIdList_.remove(id);
    return savePromptIdList();
}

bool PromptIdManager::updatePromptIdOrder(const std::list<int>& newOrder) {
    promptIdList_ = newOrder;
    return savePromptIdList();
}

void PromptIdManager::loadPromptIdList() {
    struct stat buffer;
    bool fileExists = (stat(configPath_.c_str(), &buffer) == 0);
    if (!fileExists) {
        createConfigFile();
    } else {
        if (buffer.st_size == 0) {
            createConfigFile();
        } else {
            readConfigFile();
        }
    }
}

bool PromptIdManager::savePromptIdList() {
    std::ofstream file(configPath_, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        fprintf(stderr, "Failed to open configuration file: %s\n",
                configPath_.c_str());
        return false;
    }

    for (auto it = promptIdList_.begin(); it != promptIdList_.end(); ++it) {
        file << *it;
        file << ",";
    }
    file.close();
    return true;
}

void PromptIdManager::createConfigFile() {
    namespace fs = std::filesystem;
    if (!fs::exists(configDir_)) {
        if (!fs::create_directories(configDir_)) {
            fprintf(stderr, "Failed to create directory: %s\n",
                    configDir_.c_str());
            return;
        }
    }
    std::ofstream file(configPath_);
    if (!file.is_open()) {
        fprintf(stderr, "Failed to create configuration file: %s\n",
                configPath_.c_str());
        return;
    }
    file.close();
}

void PromptIdManager::readConfigFile() {
    std::ifstream file(configPath_);
    if (!file.is_open()) {
        fprintf(stderr, "Failed to read configuration file: %s\n",
                configPath_.c_str());
        return;
    }

    std::string line;
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string idStr;
        promptIdList_.clear();
        while (std::getline(ss, idStr, ',')) {
            int id = std::stoi(idStr);
            promptIdList_.push_back(id);
        }
    }
    file.close();
}
