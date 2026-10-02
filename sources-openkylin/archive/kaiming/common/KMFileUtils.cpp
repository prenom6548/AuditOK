/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMFileUtils.h"

#include <sys/mman.h>
#include <libgen.h>
#include <fstream>
#include <random>
#include <glib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>

#include "KMProcessUtils.h"
#include "KMConfig.h"
#include "KMLogger.h"

/**
 * @brief 获取文件时间戳
 * 
 * @param filePath 文件路径
 * @return int 文件时间戳
 */
int KMFileUtils::getFileTimestamp(const std::string &filePath)
{
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) == 0)
    {
        return fileInfo.st_mtime;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief 判断文件是否过期
 * 
 * 比较当前时间戳和文件时间戳大小，如果当前时间戳大于文件时间戳，并且时间差大于1小时，则返回true，否则返回false
 * 
 * @param filePath 文件路径
 * @param expiredTime 过期时间，单位为秒
 * @return true 文件过期
 * @return false 文件未过期
 */
bool KMFileUtils::isFileExpired(const std::string &filePath, int expiredTime)
{
    int currentTime = time(nullptr);
    int fileTimestamp = getFileTimestamp(filePath);

    bool isExpired = currentTime - fileTimestamp > expiredTime;

    return isExpired;
}

/**
 * @brief 判断文件是否是 json 文件
 * 
 * @param filePath 文件路径
 * @return true 是 json 文件
 * @return false 不是 json 文件
 */
bool KMFileUtils::isJsonFile(const std::string &filePath)
{
    std::ifstream inputFile(filePath);

    if (inputFile.is_open())
    {
        std::string jsonContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
        std::regex commentPattern(R"(\/\/[^\n]*|\/\*[\s\S]*?\*\/)");
        std::string cleanedJson = std::regex_replace(jsonContent, commentPattern, "");

        inputFile.close();

        // 使用 nlohmann::json 解析
        try
        {
            json jsonData;
            jsonData = json::parse(cleanedJson);
            return true;
        }
        catch (const std::exception &e)
        {
            kmlogger.error("Failed to parse json file: %s", e.what());
            return false;
        }
    }
    else
    {
        return false;
    }
}

/**
 * @brief 判断文件是否存在
 * 
 * @param fileName 文件名
 * @return true 文件存在
 * @return false 文件不存在
 */
bool KMFileUtils::fileExists(const std::string &fileName)
{
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}

std::string KMFileUtils::getCurrentTime()
{
    time_t now = time(nullptr);
    char buf[80];

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    return std::string(buf);
}

/**
 * @brief 创建文件或者清空已有文件并写入当前时间
 * 
 * @param fileName 
 * @param fileName 文件名
 * @return true 文件操作成功
 * @return false 文件操作失败
 */
bool KMFileUtils::createFileIfNotExist(const std::string &fileName)
{
    std::ofstream outFile(fileName, std::ios::trunc); // 以 trunc 模式打开文件，文件存在时会清空
    if (outFile.is_open())
    {
        outFile << "Install time: " << getCurrentTime() << std::endl;
        outFile.close();
        kmlogger.info("File created/cleared and time written.");
        return true;
    }
    else
    {
        std::cerr << "Failed to create or open the file." << std::endl;
        return false;
    }
}

/**
 * isEmptyDirectory:
 * @brief : 判断路径是否空目录，使用前先判断是否是目录
 * @param : [in] path，路径
 * @return: true-空目录或不存在; false-不是空目录或不是目录
 */
bool KMFileUtils::isEmptyDirectory(const std::string &path)
{
    fs::path p(path);
    if (!fs::exists(p))
    {
        return true;
    }

    if (!fs::is_directory(p))
    {
        return false;
    }

    bool ret = true;
    for (auto const &dirEntry : fs::directory_iterator{p})
    {
        std::string filename = dirEntry.path().filename();
        if (filename != "." && filename != "..")
        {
            ret = false;
            break;
        }
    }

    return ret;
}
bool KMFileUtils::isEmptyDir(const std::string &path)
{
    for (auto &dirEntry : fs::directory_iterator(path))
    {
        if (fs::is_directory(dirEntry))
        {
            if (!isEmptyDir(dirEntry.path()))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

/**
 * removeEmptyDirectory:
 * @brief : 递归删除空子目录
 * @param : [in] path，路径
 * @return: true-操作成功; false-操作失败
 */
bool KMFileUtils::removeEmptyDirectory(const std::string &path, std::error_code &ec)
{
    fs::path p(path);
    if (!fs::exists(p))
    {
        return true;
    }

    if (!fs::is_directory(p))
    {
        return false;
    }

    for (auto const &dirEntry : fs::directory_iterator{p})
    {
        fs::path subdir = dirEntry.path();
        std::string filename = subdir.filename().string();

        if (filename == "." || filename == "..")
        {
            continue;
        }

        if (!fs::is_directory(subdir))
        {
            continue;
        }

        if (KMFileUtils::isEmptyDirectory(subdir.string()))
        {
            bool result = fs::remove(subdir, ec);
            if (!result)
            {
                return false;
            }
        }
        else
        {
            bool result = KMFileUtils::removeEmptyDirectory(subdir.string(), ec);
            if (!result)
            {
                return false;
            }

            if (KMFileUtils::isEmptyDirectory(subdir.string()))
            {
                result = fs::remove(subdir, ec);
                if (!result)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

/**
 * @brief : 目录或文件是否存在
 * @param : [in] path, 待目录或文件
 * @return : true-存在；false-不存在
 */
bool KMFileUtils::pathExists(const std::string &path)
{
    fs::path p(path);
    return fs::exists(p);
}

/**
 * @brief : 获取文件或目录中文件大小
 */
uint64_t KMFileUtils::getDirectorySize(const fs::path &dir)
{
    if (!fs::exists(dir) || !fs::is_directory(dir))
    {
        return 0;
    }

    // 不具备访问权限的目录略过
    if (::access(dir.string().c_str(), R_OK) != 0)
    {
        return 0;
    }

    std::string result;
    if (KMProcessUtils::spawn("du", {"-sb", dir}, result))
    {
        if (KMStringUtils::contains(result, " "))
        {
            std::vector<std::string> sizes = KMStringUtils::splitString(result, " ");
            if (sizes.size() > 0)
            {
                std::string totalSize = sizes.at(0);
                return std::stoull(totalSize);
            }
        }
        else if (KMStringUtils::contains(result, "\t"))
        {
            std::vector<std::string> sizes = KMStringUtils::splitString(result, "\t");
            if (sizes.size() > 0)
            {
                std::string totalSize = sizes.at(0);
                return std::stoull(totalSize);
            }
        }
    }

    return 0;
}

/**
 * @brief : 递归创建目录
 * @param : [in] path, 目录
 * @param : [out] std::error_code，保存错误信息
 * @return : true-存在；false-不存在
 */
bool KMFileUtils::mkpath(const std::string &path, std::error_code &ec)
{
    bool result = fs::create_directories(path, ec);
    if (!result && 0 == ec.value())
    {
        return true;
    }
    return result;
}

/**
 * @brief : 递归创建目录，如果目录已经存在则不报告错误
 * @param : [in] path, 目录
 * @return : true-存在；false-不存在
 */
bool KMFileUtils::mkpath(const std::string &path)
{
    std::error_code ec;
    return KMFileUtils::mkpath(path, ec);
}

/**
 * @brief : 递归创建目录，如果目录已经存在则不报告错误
 * @param : [in] path, 目录
 * @param : [in] mode，路径权限，可八进制写法，如：0755
 * @return : true-存在；false-不存在
 */
bool KMFileUtils::mkpath(const std::string &path, unsigned int mode)
{
    std::error_code ec;
    if (KMFileUtils::mkpath(path, ec))
    {
        fs::permissions(path, (fs::perms)mode);
        return true;
    }

    return false;
}

/**
 * @brief : 递归删除目录或文件
 * @param : [in] path, 待目录或文件
 * @param : [out] std::error_code，保存错误信息
 * @return : true-成功；false-失败
 */
bool KMFileUtils::removeAll(const std::string &path, std::error_code &ec)
{
    bool result = fs::remove_all(path, ec);
    if (!result && 0 == ec.value())
    {
        return true;
    }
    return result;
}

/**
 * @brief : 递归删除目录或文件
 * @param : [in] path, 待目录或文件
 * @return : true-成功；false-失败
 */
bool KMFileUtils::removeAll(const std::string &path)
{
    std::error_code ec;
    return KMFileUtils::removeAll(path, ec);
}

/**
 * @brief : 强制递归删除目录或文件
 * @param : [in] path, 待目录或文件
 * @return : true-成功；false-失败
 * @note  : 有些场景KMFileUtils::removeAll有无法删除的目录，使用KMFileUtils::rmRfAll可以删除。如overlay中的workdir
 */
bool KMFileUtils::rmRfAll(const std::string &path)
{
    std::vector<std::string> args;
    args.push_back("-rf");
    args.push_back(path);

    std::string result;
    if (!KMProcessUtils::spawn("rm", args, result))
    {
        KMWarn(result);
        return false;
    }

    return true;
}

/**
 * @brief : 获取路径的真实路径
 * @param : [in] inpath，路径
 * @return: 真实路径,软链接会替换为其连接路径
 */
std::string KMFileUtils::realpath(const std::string &inpath)
{
    if (inpath.empty())
    {
        return "";
    }

    std::string path = KMFileUtils::canonicalizeFilename(inpath);

    fs::path filepath(path);
    if (!fs::exists(filepath))
    {
        return "";
    }

    if (fs::is_symlink(filepath))
    {
        fs::path link = fs::read_symlink(filepath);
        fs::path targetPath = link;
        if (!link.is_absolute())
        {
            std::string target = link.string();
            targetPath = filepath.parent_path() / target;
        }
        if (!fs::exists(targetPath))
        {
            return "";
        }

        return targetPath.string();
    }

    return filepath.string();
}

/**
 * @brief : 规范化文件路径
 * @param : [in] path，原始文件路径
 * @return: 规范化后的文件路径
 */
std::string KMFileUtils::canonicalizeFilename(const std::string &path)
{
    if (!fs::exists(path))
    {
        return path;
    }

    return fs::canonical(path);
}

/**
 * @brief : 生成临时文件名
 * @param : [in | out] tmp，临时文件名，必需如下形式的:*.XXXXXX
 * @return: 是否成功
 */
bool KMFileUtils::genTempName(std::string &tmp)
{
    if (!KMStringUtils::endsWith(tmp, "XXXXXX"))
        return false;

    static const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static const int NLETTERS = sizeof(letters) - 1;

    char XXXXXX[6 + 1] = {0};
    int i = 0;
    for (; i < 6; i++)
    {
        XXXXXX[i] = letters[g_random_int_range(0, NLETTERS)];
    }
    XXXXXX[i] = 0;

    KMStringUtils::replace(tmp, "XXXXXX", XXXXXX);

    return true;
}

std::string KMFileUtils::kmGenerateTempName(const std::string &tempName)
{
    std::string res = tempName;
    std::string endStr = "";
    std::string charAll = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    std::random_device r;
    std::default_random_engine e(r());
    std::uniform_int_distribution<int> uniformDist(0, charAll.length() - 1);

    for (int i = 0; i < 6; i++)
    {
        endStr += charAll.at(uniformDist(e));
    }

    res.replace(res.length() - 6, 6, endStr);

    return res;
}

/**
 * @brief : 获取目录所有文件md5校验和（包含子孙目录）
 * @param : [in] dir，目录路径
 * @return: 目录所有文件md5校验和
 * @note  : 此方法废弃，替换为KMDirMd5sum::md5sum
 */
std::string KMFileUtils::getDirMd5sum(const std::string &dir)
{
    std::string result;
    std::vector<std::string> files = KMFileUtils::getDirFiles(dir);

    for (auto &file : files)
    {
        result += KMFileUtils::getFileMd5sum(file);
    }

    if (!result.empty())
    {
        std::string tmpFile = KMStringUtils::buildFilename("/tmp", KMFileUtils::kmGenerateTempName(".build-package.XXXXXX"));
        KMFileUtils::syncWriteFile(tmpFile, result);

        result = KMFileUtils::getFileMd5sum(tmpFile);
        KMFileUtils::removeAll(tmpFile);
    }

    return result;
}

/**
 * @brief : 判断是否废弃的md5计算方式的结果
 * @param : md5sumFile，md5校验和文件
 * @return: true-废弃的；false-未废弃的
 */
bool KMFileUtils::isObsolete(const std::string &md5sumFile)
{
    bool ret = false;

    std::ifstream file(md5sumFile);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file : " << md5sumFile << std::endl;
        return true;
    }

    std::string line;
    std::getline(file, line);
    file.close();

    std::vector sections = KMStringUtils::splitString(line, " ");
    if (sections.size() == 1)
    {
        return true;
    }

    return ret;
}

/**
 * @brief : 获取文件sha256校验和
 * @param : [in] file，文件路径
 * @return: 文件内容校验和
  */
std::string KMFileUtils::getFileSha256sum(const std::string &file)
{
    std::string result;
    if (!fs::exists(file))
    {
        return result;
    }

    std::vector<std::string> args;
    args.push_back("-b");
    args.push_back(file);

    if (KMProcessUtils::spawn("sha256sum", args, result))
    {
        std::vector<std::string> splits = KMStringUtils::splitString(result, " ");
        if (!splits.empty())
        {
            result = splits.at(0);
        }
        return result;
    }

    KMWarn(result);
    return "";
}

/**
 * @brief : 获取文件md5校验和
 * @param : [in] file，文件路径
 * @return: 文件内容校验和
 */
std::string KMFileUtils::getFileMd5sum(const std::string &file)
{
    std::string result;
    if (!fs::exists(file))
    {
        return result;
    }

    std::vector<std::string> args;
    args.push_back("-b");
    args.push_back(file);

    if (KMProcessUtils::spawn("md5sum", args, result))
    {
        std::vector<std::string> splits = KMStringUtils::splitString(result, " ");
        if (!splits.empty())
        {
            result = splits.at(0);
        }
        return result;
    }

    KMWarn(result);
    return "";
}

/**
 * @brief 读取文件内容
 * @param fileName 文件名，包含路径
 * @param content 文件内容
 * @return bool true成功 false失败
 */
bool KMFileUtils::readFile(const std::string &fileName, std::string &content)
{
    try
    {
        content.clear();
        std::ifstream ifs(fileName);
        std::ostringstream in;
        in << ifs.rdbuf();
        content = in.str();
        return in.good();
    }
    catch (...)
    {
        return false;
    }
}

/**
 * @brief 立即写文件
 * @param fileName 文件名，包含路径
 * @param content 文件内容
 * @return bool
 */
bool KMFileUtils::syncWriteFile(const std::string &fileName, const std::string &content)
{
    std::unique_ptr<std::FILE, int (*)(std::FILE *)> fp(std::fopen(fileName.c_str(), "w"), std::fclose);

    if (!fp)
    {
        std::perror("Failed to write file");
        return false;
    }

    ::fwrite(content.c_str(), sizeof(char), content.length(), fp.get());

    ::fflush(fp.get());
    fdatasync(::fileno(fp.get()));

    return true;
}

/**
 * @brief : 获取目录下所有的文件列表（包含子目录下的文件）
 * @param : [in] dir，路径
 * @return: 文件列表
 */
std::vector<std::string> KMFileUtils::getDirFiles(const fs::path &dir)
{
    std::vector<std::string> files;

    if (!fs::exists(dir))
    {
        return files;
    }

    if (!fs::is_directory(dir))
    {
        files.push_back(dir.string());
        return files;
    }

    // 不具备访问权限的目录略过
    if (::access(dir.string().c_str(), R_OK) != 0)
    {
        return files;
    }

    for (const auto &entry : fs::directory_iterator(dir))
    {
        std::string filename = entry.path().filename();
        if (filename == "." || filename == "..")
        {
            continue;
        }

        if (::access(entry.path().string().c_str(), R_OK) != 0)
        {
            continue;
        }

        if (fs::is_regular_file(entry.status()))
        {
            files.push_back(entry.path().string());
        }
        else if (fs::is_symlink(entry.path()))
        {
            // do nothing
        }
        else if (fs::is_directory(entry.path()))
        {
            std::vector<std::string> subs = KMFileUtils::getDirFiles(entry.path());
            for (auto &item : subs)
            {
                files.push_back(item);
            }
        }
    }

    return files;
}

/**
 * findFileInTree:
 * @brief : 在目录树中查找文件是否存在
 * @param : [in] baseDir, 查找的目标目录
 * @param : [in] filename，待查找的文件名
 * @return: true-找到; false-未找到
 */
bool KMFileUtils::findFileInTree(const std::string &baseDir, const std::string &filename)
{
    fs::path basePath(baseDir);
    if (!fs::exists(basePath))
    {
        return false;
    }

    for (auto const &dirEntry : fs::directory_iterator{basePath})
    {
        fs::path p = dirEntry.path();
        if (fs::is_regular_file(p) && p.filename() == filename)
        {
            return true;
        }
        else if (fs::is_directory(p))
        {
            return findFileInTree(KMStringUtils::buildFilename(baseDir, p.filename()), filename);
        }
    }

    return false;
}

/**
 * @brief : 使用cp实现目录或文件拷贝
 * @param : [in] src, 源目录或文件
 * @param : [in] dest, 目标目录或文件
 * @return : true-成功；false-失败
 */
bool KMFileUtils::cpRfAll(const std::string &src, const std::string &dest)
{
    std::vector<std::string> args;
    args.push_back("-rf");
    args.push_back(src);
    args.push_back(dest);

    if (!KMProcessUtils::spawn("cp", args))
    {
        return false;
    }

    return true;
}

bool KMFileUtils::cpRfAllStar(const std::string &src, const std::string &dest)
{
    std::string cmd = "cp -rf --no-preserve=xattr ";
    cmd = cmd + src;
    cmd = cmd + " " + dest;
    if (std::system(cmd.c_str()) == 0)
    {
        return true;
    }
    return false;
}

/**
 * @brief : 使用rsync同步目录中的内容
 * @param : [in] src, 源目录
 * @param : [in] dest, 目标目录
 * @param : [in] excludes, 排除同步路径
 * @return : true-成功；false-失败
 */
bool KMFileUtils::rsyncDir(const std::string &src, const std::string &dest, const std::vector<std::string>& excludes)
{
    std::vector<std::string> args;
    args.push_back("-rlptH");
    args.push_back("--ignore-errors");
    for (auto& exclude : excludes)
    {
        args.push_back("--exclude=" + exclude);
    }
    args.push_back(src + "/");
    args.push_back(dest + "/");

    if (!KMProcessUtils::spawn("rsync", args, true))
    {
        return false;
    }

    return true;
}

std::vector<std::string> KMFileUtils::parseTxt(const std::string &path)
{
    std::vector<std::string> lines;
    std::ifstream file(path);
    if (!file.is_open())
    {
        kmlogger.error(("open " + path + "error").c_str());
        return lines;
    }
    std::string line;
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }
    return lines;
}

bool KMFileUtils::isImmutableSystem()
{
    std::string system_ovl_conf = "/etc/ostree-release";
    if (fs::exists(system_ovl_conf))
    {
        return true;
    }
    
    return false;
}

bool KMFileUtils::isExecutableByFileCmd(const fs::path &path)
{
    std::string cmd = "file -b --mime-type \"" + path.string() + "\"";
    FILE *fp = popen(cmd.c_str(), "r");
    if (!fp) {
        return false;
    }

    char buf[128] = {0};
    if (fgets(buf, sizeof(buf), fp) == nullptr)
    {
        pclose(fp);
        return false;
    }
    pclose(fp);
    std::string mime(buf);

    if (mime.find("x-executable") != std::string::npos ||
        mime.find("x-pie-executable") != std::string::npos ||
        mime.find("x-sharedlib") != std::string::npos ||
        mime.find("x-shellscript") != std::string::npos)
    {
        return true;
    }
    return false;
}

bool KMFileUtils::isShellScriptByFileCmd(const fs::path &path)
{
    std::string cmd = "file -b --mime-type \"" + path.string() + "\"";
    FILE *fp = popen(cmd.c_str(), "r");
    if (!fp) {
        return false;
    }

    char buf[128] = {0};
    if (fgets(buf, sizeof(buf), fp) == nullptr)
    {
        pclose(fp);
        return false;
    }
    pclose(fp);
    std::string mime(buf);

    if (mime.find("x-shellscript") != std::string::npos)
    {
        return true;
    }

    return false;
}

bool KMFileUtils::isNonExecExt(const fs::path& path) {
    std::string filename = path.filename().string();
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    static const std::vector<std::string> nonExecExt = {
        ".txt", ".pdf", ".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx",
        ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".svg", ".mp3", ".mp4",
        ".avi", ".mkv", ".zip", ".rar", ".7z", ".tar", ".gz", ".json", 
        ".xml", ".csv", ".md", ".log", ".ini", ".conf", ".html", ".css",
        ".js", ".py", ".java", ".cpp", ".h", ".hpp", ".c", ".o",
        ".a", ".dll", ".lib", ".dat", ".db", ".sqlite", ".bak",
        ".tmp", ".temp", ".swp", ".swo", ".orig", ".rej", ".patch"
    };
    if (std::find(nonExecExt.begin(), nonExecExt.end(), ext) != nonExecExt.end()) {
        return true;
    }

    size_t soPos = filename.find(".so");
    if (soPos != std::string::npos) {
        // 以 .so 结尾，或 .so. 后面跟数字
        if (filename.size() > soPos + 3 && filename[soPos + 3] == '.') {
            return true;
        }
        if (filename.substr(soPos) == ".so") {
            return true;
        }
    }

    return false;
}

std::string KMFileUtils::copyFilesByExtension(const fs::path &sourceDir, const fs::path &destDir, const std::string &extension, const std::string &fileType, std::set<std::string> &installedFiles)
{
    if (!fs::exists(sourceDir))
    {
        kmlogger.info("Source directory does not exist: %s", sourceDir.string().c_str());
        return "";
    }

    if (!fs::exists(destDir))
    {
        fs::create_directories(destDir);
    }

    std::string destfile;
    for (const auto &entry : fs::directory_iterator(sourceDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == extension)
        {
            fs::path dest = destDir / entry.path().filename();
            destfile = dest.c_str();
            try
            {
                fs::copy_file(entry.path(), dest, fs::copy_options::overwrite_existing);
                installedFiles.insert(dest);
                kmlogger.info("Copy %s file %s to %s", fileType.c_str(), entry.path().c_str(), dest.c_str());
            }
            catch (const std::exception &e)
            {
                destfile = "";
                kmlogger.error("Failed to copy %s: %s", entry.path().c_str(), e.what());
            }
        }
    }
    return destfile;
}

bool KMFileUtils::syncWithExclusions(const fs::path& source, const fs::path& destination, const std::vector<std::string>& excludePatterns) {
    try {
        if (!fs::exists(source)) {
            kmlogger.error("Source path does not exist: %s", source.string().c_str());
            return false;
        }

        std::string src = source.string();
        std::string dst = destination.string();

        if (fs::is_directory(source) && src.back() != '/') {
            src += '/';
        }

        std::string cmd = "rsync -a -H  --links ";

        for (const auto& pattern : excludePatterns) {
            cmd += "--exclude='" + pattern + "' ";
        }

        cmd += src + " " + dst;

        kmlogger.info("Executing rsync:: %s", cmd.c_str());

        int ret = std::system(cmd.c_str());
        if (ret != 0) {
            kmlogger.error("rsync failed with code: %d", ret);
            return false;
        }

        kmlogger.info("rsync completed:: %s -> %s", source.string().c_str(), destination.string().c_str());
        return true;
    }
    catch (const std::exception& e) {
        kmlogger.error("Exception during syncSinglePlugin: %s", e.what());
        return false;
    }
}
