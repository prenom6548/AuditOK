/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "service.h"

namespace fs = std::filesystem;

std::vector<std::string> collectAllFiles(const std::string& path) {
    std::vector<std::string> files;

    // 检查路径是否存在
    if (!fs::exists(path)) {
        std::cerr << "Path does not exist." << std::endl;
        return files;
    }

    // 根据路径类型执行不同逻辑
    if (fs::is_regular_file(path)) {  // 输入是文件
        std::cout << "input is filepath." << std::endl;
        files.push_back(path);
    } else if (fs::is_directory(path)) {  // 输入是目录
        // 递归遍历目录下的所有文件
        std::cout << "input is directory." << std::endl;
        std::cout << "start to collect all filepahts." << std::endl;
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().string());
            }
        }
    }
    std::cout << "collect all filepahts sucess." << std::endl;
    return files;
}


void createCollection(const std::string& name) {
    std::cout << "createCollection start." << std::endl;
    bool ret = knowledgebase_tool::Service::getInstance()->createCollection(name);

    if (!ret) {
        std::cerr << "createCollection failed." << std::endl;
        return;
    }
    std::cout << "createCollection sucess." << std::endl;
    std::cout << "createCollection name is: " << name << std::endl;
}

void addFiles(const std::string& name, const std::string& filepath) {
    std::vector<std::string> input = collectAllFiles(filepath);
    if (input.empty()) {
        std::cerr << "Error: no file found in the directory." << std::endl;
        return;
    }
    std::cout << "addFiles start." << std::endl;
    bool ret = knowledgebase_tool::Service::getInstance()->addFiles(name, input);
    if (!ret) {
        std::cerr << "addFiles failed." << std::endl;
    }
    std::cout << "addFiles sucess." << std::endl;
}

void deleteFiles(const std::string& name, const std::string& filepath) {
    std::vector<std::string> input{filepath};
    std::cout << "deleteFiles start." << std::endl;
    bool ret = knowledgebase_tool::Service::getInstance()->deleteFiles(name, input);
    if (!ret) {
        std::cerr << "deleteFiles failed." << std::endl;
    }
    std::cout << "deleteFiles sucess." << std::endl;
}

void updateFiles(const std::string& name, const std::string& filepath) {
    std::cout << "updateFiles start." << std::endl;
    std::vector<std::string> input = collectAllFiles(filepath);
    if (input.empty()) {
        std::cerr << "Error: no file found in the directory." << std::endl;
        return;
    }
    bool ret = knowledgebase_tool::Service::getInstance()->updateFiles(name, input);
    if (!ret) {
        std::cerr << "updateFiles failed." << std::endl;
    }
    std::cout << "updateFiles sucess." << std::endl;
}
void search(const std::string& name, const std::string& text, float threshold = 0.6) {
    std::cout << "search start." << std::endl;
    bool ret = knowledgebase_tool::Service::getInstance()->search(name, text, threshold);
    if (!ret) {
        std::cerr << "search failed." << std::endl;
    }
    std::cout << "search sucess." << std::endl;
}
void dropCollection(const std::string& name) {
    std::cout << "dropCollection start." << std::endl;
    bool ret = knowledgebase_tool::Service::getInstance()->dropCollection(name);
    if (!ret) {
        std::cerr << "dropCollection failed." << std::endl;
    }
    std::cout << "dropCollection sucess." << std::endl;
}
void query(const std::string& name) {
    std::cout << "query start." << std::endl;
    bool ret = knowledgebase_tool::Service::getInstance()->query(name);
    if (!ret) {
        std::cerr << "query failed." << std::endl;
    }
    std::cout << "query sucess." << std::endl;
}

void printUsage(const std::string& progName) {
    std::cerr << "Usage: " << progName << " <database_name> <command> [args...]\n";
    std::cerr << "Commands:\n";
    std::cerr << "  [name] --create                        Create the collection\n";
    std::cerr << "  [name] --add <filepath>                Add a file\n";
    std::cerr << "  [name] --delete <filepath>             Delete a file\n";
    std::cerr << "  [name] --update <filepath>             Update a file\n";
    std::cerr << "  [name] --search <text> <threshold>     Search for text\n";
    std::cerr << "  [name] --query                         Query the collection\n";
    std::cerr << "  [name] --drop                          Drop the collection\n";
}
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    std::string dbName = argv[1];
    std::string command = argv[2];

    if (command == "--create") {
        if (argc != 3) {
            std::cerr << "Error: --create does not require additional arguments\n";
            return 1;
        }
        createCollection(dbName);
    } else if (command == "--add") {
        if (argc < 4) {
            std::cerr << "Error: --add does not require additional arguments\n";
            return 1;
        }
        addFiles(dbName, argv[3]);
    } else if (command == "--delete") {
        if (argc < 4) {
            std::cerr << "Error: --delete does not require additional arguments\n";
            return 1;
        }
        deleteFiles(dbName, argv[3]);
    } else if (command == "--update") {
        if (argc < 4) {
            std::cerr << "Error: --update does not require additional arguments\n\n";
            return 1;
        }
        updateFiles(dbName, argv[3]);
    } else if (command == "--search") {
        if (argc < 4) {
            std::cerr << "Error: --search does not require additional arguments\n\n";
            return 1;
        }
        double threshold = atof(argv[4]);
        search(dbName, argv[3], threshold);
    } else if (command == "--drop") {
        if (argc != 3) {
            std::cerr << "Error: --drop does not require additional arguments\n";
            return 1;
        }
        dropCollection(dbName);
    } else if (command == "--query") {
        if (argc != 3) {
            std::cerr << "Error: --query does not require additional arguments\n";
            return 1;
        }
        query(dbName);
    } else {
        std::cerr << "Error: Unknown command '" << command << "'\n";
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}