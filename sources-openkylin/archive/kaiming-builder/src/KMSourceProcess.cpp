/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMSourceProcess.h"
#include "KMBuilderUtils.h"
#include "common/KMProcessUtils.h"

using namespace std;


std::string calculateMd5(const std::string &fileName)
{
    std::ifstream file(fileName, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Unable to open file: " + fileName);
    }
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == nullptr)
    {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }
    if (EVP_DigestInit_ex(mdctx, EVP_md5(), nullptr) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to initialize MD5 digest");
    }
    std::vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0)
    {
        if (EVP_DigestUpdate(mdctx, buffer.data(), file.gcount()) != 1)
        {
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("Failed to update MD5 digest");
        }
    }
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;
    if (EVP_DigestFinal_ex(mdctx, hash, &lengthOfHash) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to finalize MD5 digest");
    }
    EVP_MD_CTX_free(mdctx);
    std::ostringstream oss;
    for (unsigned int i = 0; i < lengthOfHash; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return oss.str();
}
std::string calculateSha512(const std::string &fileName)
{
    std::ifstream file(fileName, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Unable to open file: " + fileName);
    }
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == nullptr)
    {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }
    if (EVP_DigestInit_ex(mdctx, EVP_sha512(), nullptr) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to initialize SHA-512 digest");
    }
    std::vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0)
    {
        if (EVP_DigestUpdate(mdctx, buffer.data(), file.gcount()) != 1)
        {
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("Failed to update SHA-512 digest");
        }
    }
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;
    if (EVP_DigestFinal_ex(mdctx, hash, &lengthOfHash) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to finalize SHA-512 digest");
    }
    EVP_MD_CTX_free(mdctx);
    std::ostringstream oss;
    for (unsigned int i = 0; i < lengthOfHash; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return oss.str();
}
std::string calculateSha256(const std::string &fileName)
{
    std::ifstream file(fileName, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Unable to open file: " + fileName);
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == nullptr)
    {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to initialize SHA-256 digest");
    }

    std::vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0)
    {
        if (EVP_DigestUpdate(mdctx, buffer.data(), file.gcount()) != 1)
        {
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("Failed to update SHA-256 digest");
        }
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;
    if (EVP_DigestFinal_ex(mdctx, hash, &lengthOfHash) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to finalize SHA-256 digest");
    }

    EVP_MD_CTX_free(mdctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < lengthOfHash; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return oss.str();
}
std::string calculateSha1(const std::string &fileName)
{
    std::ifstream file(fileName, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Unable to open file: " + fileName);
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == nullptr)
    {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha1(), nullptr) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to initialize SHA-1 digest");
    }

    std::vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0)
    {
        if (EVP_DigestUpdate(mdctx, buffer.data(), file.gcount()) != 1)
        {
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("Failed to update SHA-1 digest");
        }
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;
    if (EVP_DigestFinal_ex(mdctx, hash, &lengthOfHash) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to finalize SHA-1 digest");
    }

    EVP_MD_CTX_free(mdctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < lengthOfHash; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return oss.str();
}
namespace fs = std::filesystem;
bool is_file1(const std::string &path)
{
    return std::filesystem::is_regular_file(path);
}
bool isSameSha512(string checkStr, string checkFile)
{
    if (checkStr == calculateSha512(checkFile))
	{
		return true;
	} 
    return false;
}
bool isSameSha256(string checkStr, string checkFile)
{
    if (checkStr == calculateSha256(checkFile))
	{
		return true;
	}
    return false;
}
bool isSameSha1(string checkStr, string checkFile)
{
    if (checkStr == calculateSha1(checkFile))
	{
		return true;
	}
    return false;
}
bool isSameMD5(string checkStr, string checkFile)
{
    if (checkStr == calculateMd5(checkFile))
	{
		return true;
	}
    return false;
}

std::string getName(const std::string &path)
{
    size_t pos = path.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        return path.substr(0, pos + 1);
    }
    else
    {
        return "";
    }
}
bool isFile(const std::string &path)
{
    return std::filesystem::is_regular_file(path);
}
int checkPathType(const std::string &path)
{
    fs::path p(path);

    if (!fs::exists(p))
    {
        return 0; // Path does not exist
    }
    else if (fs::is_regular_file(p))
    {
        return 1; // Path is a file
    }
    else if (fs::is_directory(p))
    {
        return 2; // Path is a directory
    }

    return 0; // Path exists but is neither a regular file nor a directory
}

bool should_skip(const fs::path &path, const std::vector<std::regex> &regexPatterns)
{
    for (const auto &re : regexPatterns)
    {
        if (std::regex_match(path.filename().string(), re))
        {
            return true;
        }
    }
    return false;
}

void recursive_copy(const fs::path &src, const fs::path &dst, const std::vector<std::regex> &regexPatterns)
{
    if (should_skip(src, regexPatterns))
    {
        return;
    }

    if (fs::is_directory(src))
    {
        fs::create_directories(dst);
        for (const auto &entry : fs::directory_iterator(src))
        {
            recursive_copy(entry.path(), dst / entry.path().filename(), regexPatterns);
        }
    }
    else if (fs::is_regular_file(src))
    {
        // string a = src;
        // string b = dst;
        
        // fs::path sourcePath(src);
        // fs::path destPath(dst);

        if (fs::is_directory(dst))
        {
            fs::copy_file(src, dst / src.filename(), fs::copy_options::overwrite_existing);
        }
        else
        {
            fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
        }
    }
}

bool fileCopy(const std::string &source, const std::string &destination, const std::vector<std::string> &skipPatterns)
{
    try
    {
        // Check if source exists
        if (!fs::exists(source))
        {
            return false;
        }

        // Compile regex patterns
        std::vector<std::regex> regexPatterns;
        for (const auto &pattern : skipPatterns)
        {
            regexPatterns.emplace_back(pattern);
        }

        // Start the copying process
        recursive_copy(fs::path(source), fs::path(destination), regexPatterns);

        return true;
    }
    catch (const fs::filesystem_error &e)
    {
        return false;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

string checkSum(std::string checkSha512String, std::string checkSha256String, std::string checkSha1String, std::string checkMd5String, std::string checkFile)
{
     // 使用 lambda 函数简化校验逻辑
    auto check = [](const std::string& checksum, auto isSameFunc, const std::string& errorMsg, const std::string& file) -> std::string 
    {
        return !checksum.empty() && !isSameFunc(checksum, file) ? errorMsg : "";
    };

    // 按照优先级依次检查
    std::string result;
    if (!(result = check(checkSha512String, isSameSha512, "Sha512 check error", checkFile)).empty()) return result;
    if (!(result = check(checkSha256String, isSameSha256, "Sha256 check error", checkFile)).empty()) return result;
    if (!(result = check(checkSha1String, isSameSha1, "Sha1 check error", checkFile)).empty()) return result;
    if (!(result = check(checkMd5String, isSameMD5, "MD5 check error", checkFile)).empty()) return result;

    return "";
}

std::string getFileNameWithoutExtension(const std::string &filePath)
{
    std::filesystem::path path(filePath);

    std::string fileName = path.filename().string();

    std::string::size_type pos = fileName.find_last_of('.');
    if (pos != std::string::npos)
    {
        fileName = fileName.substr(0, pos);
    }

    return fileName;
}

bool flattenDirectory(const fs::path &directory, int levels)
{
    if (levels <= 0)
    {
        return true;
    }

    if (!fs::exists(directory) || !fs::is_directory(directory))
    {
        return false;
    }

    // Create a temporary directory to hold the flattened structure
    fs::path tempDir = fs::temp_directory_path() / "flatten_temp";
    if (!fs::create_directory(tempDir))
    {
        return false;
    }

    try
    {
        for (const auto &entry : fs::recursive_directory_iterator(directory))
        {
            if (entry.is_regular_file())
            {
                fs::path relativePath = fs::relative(entry.path(), directory);
                fs::path newPath;
                int level_count = levels;

                // Remove the specified number of levels from the relative path
                for (auto it = relativePath.begin(); it != relativePath.end(); ++it)
                {
                    if (level_count > 0)
                    {
                        --level_count;
                    }
                    else
                    {
                        newPath /= *it;
                    }
                }

                // Move the file to the new location in the temporary directory
                fs::path destPath = tempDir / newPath;
                fs::create_directories(destPath.parent_path());
                fs::rename(entry.path(), destPath);
            }
        }

        // Remove the original directory and move the flattened structure back
        fs::remove_all(directory);
        fs::rename(tempDir, directory);
    }
    catch (const fs::filesystem_error &e)
    {
        return false;
    }

    return true;
}

// 检查解压目录中是否有单一顶层目录
bool hasTopLevelDirectory(const std::string &dirPath)
{
    int directoryCount = 0;
    for (const auto &entry : fs::directory_iterator(dirPath))
    {
        if (entry.is_directory())
        {
            directoryCount++;
        }
    }
    return directoryCount == 1; // 如果目录数为1，表示存在顶层目录
}

bool unZip(const std::string &extracThere, const std::string &zipPath)
{
    KMBuilderUtils::mkpath(extracThere);
    std::vector<std::string> args;
    std::string result;
    std::string exec;

    if (zipPath.size() >= 7 && zipPath.compare(zipPath.size() - 7, 7, ".tar.gz") == 0)
    {
        exec = "tar";
        args = {"-xzf", zipPath, "-C", extracThere, "--no-same-owner", "--strip-components=1"};  
        if (!KMProcessUtils::spawn(exec, args, result))
        {
            return false;
        }
    }
    else if (zipPath.size() >= 8 && zipPath.compare(zipPath.size() - 8, 8, ".tar.bz2") == 0)
    {
        exec = "tar";
        args = {"-xjf", zipPath, "-C", extracThere, "--no-same-owner", "--strip-components=1"};
        if (!KMProcessUtils::spawn(exec, args, result))
        {
            return false;
        }
    }
    else if (zipPath.size() >= 7 && zipPath.compare(zipPath.size() - 7, 7, ".tar.xz") == 0)
    {
        exec = "tar";
        args = {"-xJf", zipPath, "-C", extracThere, "--no-same-owner", "--strip-components=1"};
        if (!KMProcessUtils::spawn(exec, args, result))
        {
            return false;
        }
    }
    else if (zipPath.size() >= 4 && zipPath.compare(zipPath.size() - 4, 4, ".zip") == 0)
    {
        // 临时解压目录
        std::string zipDir = zipPath.substr(0, zipPath.find_last_of('/')) + "/temp";
        KMBuilderUtils::mkpath(zipDir);
        exec = "unzip";
        args = {zipPath, "-d", zipDir};
        if (!KMProcessUtils::spawn(exec, args, result))
        {
            return false;
        }  
        
        bool hasTopDir = hasTopLevelDirectory(zipDir);

        std::string actualSourceDir = hasTopDir ? fs::directory_iterator(zipDir)->path().string() : zipDir; 
  
        if (!fileCopy(actualSourceDir, extracThere))
        {
            return false;
        }

        // 删除临时目录
        exec = "rm";
        args = {"-rf", zipDir};  

        if (!KMProcessUtils::spawn(exec, args, result))
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    
    return true;
}
bool executeShellScript(const std::string &script)
{
    cout << "execute: " << script << endl;
    std::array<char, 1024> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(script.c_str(), "r"), pclose);
    if (!pipe)
    {
        std::cout << "popen() failed!" << std::endl;
        return false;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return true;
}
bool executeShellScripts(const std::vector<std::string> &scripts)
{
    std::string result;
    for (const auto &script : scripts)
    {
        std::cout << "Executing command: " << script << std::endl;
        std::array<char, 1024> buffer;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(script.c_str(), "r"), pclose);
        if (!pipe)
        {
            return false;
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        result += "\n"; // Separate outputs of different commands
    }
    return true;
}
