/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#pragma once
#include <string>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <locale>
#include <codecvt>
#include <regex>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <cstdlib>

namespace fs = std::filesystem;
std::uint32_t calculateCrc32(const std::string &fileName);
std::string calculateMd5(const std::string &fileName);
std::string calculateSha512(const std::string &fileName);
std::string calculateSha256(const std::string &fileName);
std::string calculateSha1(const std::string &fileName);
bool isSameSha512(std::string checkStr, std::string checkFile);
bool isSameSha256(std::string checkStr, std::string checkFile);
bool isSameSha1(std::string checkStr, std::string checkFile);
bool isSameMD5(std::string checkStr, std::string checkFile);
std::string downloadFiles(const std::string &url, const std::string &localPathStr);
bool isFile(const std::string &path);
std::string getName(const std::string &path);
bool fileCopy(const std::string &source, const std::string &destination, const std::vector<std::string> &skipPatterns = {});
std::string checkSum(std::string checkSha512String, std::string checkSha256String, std::string checkSha1String, std::string checkMd5String, std::string checkFile);
std::string getFileNameWithoutExtension(const std::string &filePath);
bool unZip(const std::string &extracThere, const std::string &zipPath);
bool executeShellScript(const std::string &script);
bool executeShellScripts(const std::vector<std::string> &scripts);
bool flattenDirectory(const fs::path &directory, int levels);
