/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMDeltas.h"

#include <sys/personality.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <unordered_set>
#include <atomic>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <memory>
#include <array>
#include <algorithm>
#include <sstream>
#include <cstdio> // 添加这个头文件

#include "common/KMError.h"
#include "common/KMLogger.h"
#include "common/KMUtil.h"
#include "syslog.h"
#include "common/KMInfoJson.h"
#include "common/KMVersion.h"

using json = nlohmann::json;

/**
 * @brief 安装子命令的参数解析
 */
class KMDeltas::Options : public KMOption::Options
{
public:
    Options() = default;
    ~Options() override = default;

    void checkUnknownOptions(int argc, char** argv);
protected:  
    /**
     * @brief 命令行参数解析后操作
     */
    void preParseHook() override;

    /**
     * @brief 命令行参数解析后操作
     */
    void postParseHook() override;

private:
    /**
     * @brief 命令行参数添加描述选项
     */
    void addOptions();
    

public:
    bool m_help;
    bool m_isOK;
    vector<string> m_vecRefs;
};

void KMDeltas::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMDeltas::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }
}

/**
 * @brief 命令行参数添加描述选项
 */
void KMDeltas::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming deltas SOURCEDIR OUTPUTDIE\n"));
    //帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_help)->defaultValue(false), _("display help information"));
    //位置选项
    addOption("ok", "", KMOption::value<bool>(&m_isOK)->defaultValue(false), _("okfile"));
    addPositionOption("REFS", KMOption::value<std::vector<std::string>>(&m_vecRefs), -1, _("sourcedir outputdir"));
}

class KMDeltas::Private
{
public:
    std::shared_ptr<Options> m_options;
    std::shared_ptr<KMPackageManager> m_packageManager;
    map<std::string,std::string> m_savePath;
    std::string m_infoPath;
};

REGISTER_SUBCOMMAND_DYNCREATE(deltas, KMDeltas)

KMDeltas::KMDeltas()
    : d(std::make_unique<Private>()),
      m_md5Checker(std::make_shared<KMDirMd5sum>("."))
{
    init();
}

KMDeltas::~KMDeltas() = default;


int KMDeltas::deltasOK(const std::string& newVersionPath, const std::string& oldVersionPath, const std::string& outPath)
{
    fs::path version1Dir(oldVersionPath);
    fs::path version2Dir(newVersionPath);
    fs::path outputDir(outPath);
    std::string ok1Name = version1Dir.string() + ".ok";
    std::string ok2Name = version2Dir.string() + ".ok";
    std::vector<std::string> resoult1 = d->m_packageManager->extractInfoFromOK(ok1Name);
    std::vector<std::string> resoult2 = d->m_packageManager->extractInfoFromOK(ok2Name);
    vector<std::string> preVersion;
    preVersion.push_back(resoult1[2]);
    std::string infoPath = version2Dir / "layers" / resoult2[0] / resoult2[4] / "info.json";
    std::string filesPath = version2Dir / "layers" / resoult2[0] / resoult2[4] / "files";
    d->m_infoPath = infoPath;
    std::string okName = version2Dir.filename().string() + ".ok";
    std::string deltasOkName = resoult2[0] + "-" + resoult2[1] + "-" + resoult1[2] + "_" + resoult2[2] + "-" + resoult2[3] + "-" + resoult2[4] + "-squashfs.ok";
    fs::path targetPath;

    if (!fs::exists(infoPath))
    {
        std::cout << "The info.json file for the latest version of the software package was not found" << std::endl;
        return 0;
    }

    if (!fs::exists(filesPath))
    {
        std::cout << "The files directory for the latest version of the software package was also not found" << std::endl;
        return 0;
    }


    targetPath = outputDir / resoult1[2];
    fs::path info = targetPath / "info.json";
    fs::create_directories(targetPath); 
    addDeltasAnnotation(infoPath, info, preVersion);

    // 检查输入目录是否存在
    if (!fs::exists(version1Dir) || !fs::exists(version2Dir))
    {
        std::cerr << "Error: Input directories do not exist\n";
        return 1;
    }
    

    std::cout << "处理增量: " << version1Dir << " -> " << version2Dir << std::endl;
    
    // 执行rsync并处理结果 - 修复变量名
    if (executeRsyncAndProcess(version2Dir.string(), version1Dir.string(), targetPath.string())) {
        std::cout << "增量处理完成: " << targetPath << std::endl;
    } else {
        std::cerr << "增量处理失败: " << version2Dir << " -> " << version1Dir << std::endl;
        return 1;
    }
    
    std::string command = "kaiming build-export " + targetPath.string() + " -d " + targetPath.parent_path().string() + "/okfile";

    int result = std::system(command.c_str());

    // clearDirectory(outputDir);
    fs::path sourcePath = outputDir / "okfile" / okName;  
    fs::path destPath = outputDir / "okfile" / deltasOkName;

    if (fs::exists(sourcePath)) {
        fs::rename(sourcePath, destPath);
    } else {
        std::cerr << "Warning: OK file not found: " << sourcePath << std::endl;
    }
    
    return 0;
}

// // 获取目录中所有文件的相对路径
// std::vector<fs::path> KMDeltas::getFilesInDirectory(const fs::path& directory)
// {
//     std::vector<fs::path> files;
    
//     for (const auto& entry : fs::recursive_directory_iterator(directory))
//     {
//         if (entry.is_regular_file())
//         {
//             files.push_back(fs::relative(entry.path(), directory));
//         }
//     }
    
//     return files;
// }

// 创建目标目录结构
void KMDeltas::createDirectoryStructure(const fs::path& targetDir, const fs::path& relativePath)
{
    fs::create_directories(targetDir / relativePath.parent_path());
}

// 复制文件到目标目录
void KMDeltas::copyDifferentFile(const fs::path& sourceFile, const fs::path& targetDir, const fs::path& relativePath)
{
    fs::path targetPath = targetDir / relativePath;
    createDirectoryStructure(targetDir, relativePath);
    fs::copy_file(sourceFile, targetPath, fs::copy_options::overwrite_existing);
}

void KMDeltas::init()
{
    d->m_options = make_shared<Options>();
    d->m_packageManager = make_shared<KMPackageManager>();
}

void KMDeltas::addDeltasAnnotation(const fs::path& jsonPath, const fs::path& destPath, vector<std::string> preVersion) {
    try {
        // 1. 读取JSON文件
        std::ifstream input(jsonPath);
        json data = json::parse(input);
        input.close();

        // 2. 添加新字段
        data["annotations"]["kylin.deltas"] = true;
        data["annotations"]["kylin.preVersion"] = preVersion;
        // 3. 写回文件（保持格式）
        std::ofstream output(destPath);
        output << data.dump(4); // 4空格缩进
        output.close();

    } catch (const std::exception& e) {
        throw std::runtime_error("修改JSON失败: " + std::string(e.what()));
    }
}

int KMDeltas::dispose(int argc, char **argv)
{
    d->m_options->checkUnknownOptions(argc, argv);
    d->m_options->parseCommandLine(argc, argv);

    // if (d->m_options->m_vecRefs.size() != 2)
    // {
    //     std::cerr << "Usage: " << argv[0] << " <version_dir>  <output_dir>\n";
    //     return 1;
    // }
    std::vector<std::string> subdirectories;
    fs::path inputPath(d->m_options->m_vecRefs[0]);
    fs::path absolutePath = fs::absolute(inputPath);
    std::string path = absolutePath.string();
    // std::string path(d->m_options->m_vecRefs[0]);
    fs::path outputDir(d->m_options->m_vecRefs[1]);
    std::string extractPath = path;
    KMOKFile okFile;
    if (d->m_options->m_isOK)
    {
        for (const auto& entry : fs::directory_iterator(path))
        {
            if (entry.is_regular_file())
            {
                std::string filename = entry.path().filename().string();
                size_t pos = filename.find(".ok");
                string mountPoint = "/tmp/" + filename.substr(0, pos);
                string rsyncPath = path + "/extractfile";
                okFile.setMountPoint(mountPoint);

                if (!okFile.loadOKFile(entry.path().string()))
                {
                    std::cout << "Failed to load file: " << filename << std::endl;
                    return KM_ERROR_LOAD_FILE_FAILED;
                }
                okFile.mountOK(mountPoint);
                std::string command = "rsync -a -H --progress --no-owner --no-group " + mountPoint + " " + rsyncPath;
                int result = std::system(command.c_str());
                okFile.umountOK(mountPoint);
            }

        }
        extractPath = path + "/extractfile";
    }

    // 遍历目录
    for (const auto& entry : fs::directory_iterator(extractPath))
    {
        std::string pathStr = entry.path().string();

        if (pathStr.compare(pathStr.length() - 3, 3, ".ok") == 0)
        {
            std::cout << _("Please add the --ok parameter to the command") << std::endl; 
            return 1;
        }

        if (entry.is_directory())
        {
            subdirectories.push_back(entry.path().string());
        }
    }

    std::string maxVersion;
    int maxIndex = -1;  // 初始化为无效索引
    std::string id;
    std::string maxIdpath;
    std::vector<std::string> preVersion;  // 添加std::命名空间
    for (size_t i = 0; i < subdirectories.size(); ++i) {
        std::string dirName = subdirectories[i] + ".ok";
        auto result = d->m_packageManager->extractInfoFromOK(dirName);

        if (id == "")
        {
            id = result[0];
        }
        else if (id != result[0])
        {
            std::cout << "Please push the same id package" << std::endl;
            return 0;
        }

        if (maxVersion.empty()) {
            maxVersion = result[2];
            maxIndex = i;
        }
        else if(KMVersion::parse(result[2]) > KMVersion::parse(maxVersion))
        {
            preVersion.push_back(maxVersion);
            maxVersion = result[2];
            maxIndex = i;
        }
        else
        {
            preVersion.push_back(result[2]);
        }
    }

    maxIdpath = subdirectories[maxIndex];

    // 处理所有非最大版本的目录 - 参数顺序正确
    if (maxIndex != -1) {
        for (size_t i = 0; i < subdirectories.size(); ++i) {
            if (i != static_cast<size_t>(maxIndex)) {
                deltasOK(subdirectories[maxIndex], subdirectories[i], outputDir.string());
            }
        }
    }

    // 修正size()的调用方式，添加括号
    if (subdirectories.size() >= 3)
    {
        // 使用fs::path进行路径拼接
        fs::path allDeltasPath = outputDir / "all" / "files";
        fs::create_directories(allDeltasPath);

        // 遍历源目录
        for (const auto& entry : fs::directory_iterator(outputDir)) {
            if (entry.is_directory()) {
                std::string dirName = entry.path().filename().string();

                // 跳过okfile目录
                if (dirName == "okfile" || dirName == "all" || dirName == "extractfile") {
                    std::cout << "跳过目录: " << dirName << std::endl;
                    continue;
                }

                // 构建目标路径
                fs::path targetPath = allDeltasPath / dirName;

                // 如果目标路径已存在，先删除
                if (fs::exists(targetPath)) {
                    fs::remove_all(targetPath);
                }

                // 移动目录
                fs::rename(entry.path(), targetPath);
                std::cout << "已移动: " << dirName << " -> " << targetPath << std::endl;
            }
        }

        std::cout << "移动完成！" << std::endl;
        // 修正路径拼接和语法错误（缺少分号）
        fs::path infoPath = outputDir / "all" / "info.json";
        addDeltasAnnotation(d->m_infoPath, infoPath.string(), preVersion);

        // 修正targetPath的作用域问题，重新构建路径
        fs::path targetDir = outputDir / "all";
        std::string command = "kaiming build-export " + targetDir.string() + " -d " + (targetDir.parent_path() / "okfile").string();

        int result = std::system(command.c_str());

        fs::path maxIdPathObj(maxIdpath);
        std::string okName = maxIdPathObj.filename().string() + ".ok";
        fs::path sourcePath = outputDir / "okfile" / okName;  

        // 处理文件名，插入"_all"
        int dash_count = 0;
        size_t pos = std::string::npos;
        
        for (int i = okName.size() - 1; i >= 0; --i) {
            if (okName[i] == '-') {
                dash_count++;
                if (dash_count == 3) {
                    pos = i; // 找到倒数第三个连字符的位置
                    break;
                }
            }
        }
        
        // 如果找到倒数第三个连字符，插入 "_all"
        if (pos != std::string::npos) {
            okName.insert(pos, "_all");
        }
    
        fs::path destPath = outputDir / "okfile" / okName;

        if (fs::exists(sourcePath)) {
            fs::rename(sourcePath, destPath);
        } else {
            std::cerr << "Warning: OK file not found: " << sourcePath << std::endl;
        }

    }
    return 0;
}


void KMDeltas::clearDirectory(const fs::path& dir_path) {
    std::error_code ec; // 用于捕获错误而不抛出异常

    // 遍历目录并删除每个条目
    for (const auto& entry : fs::directory_iterator(dir_path, ec)) {
        if (ec) {
            std::cerr << "遍历目录错误: " << ec.message() << std::endl;
            return;
        }

        fs::remove_all(entry.path(), ec); // 递归删除子目录和文件
        if (ec) {
            std::cerr << "删除失败: " << entry.path() << " - " << ec.message() << std::endl;
            ec.clear(); // 清除错误继续尝试删除其他文件
        }
    }
}

void KMDeltas::Options::checkUnknownOptions(int argc, char** argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help", "-h",
        "--ok",
        "--loglevel"
    };

    for (int i = 1; i < argc; ++i) 
    {
        std::string arg(argv[i]);

        if (arg.size() >= 1 && arg[0] == '-') 
        {
            std::string opt = arg;
            auto eq_pos = opt.find('=');
            if (eq_pos != std::string::npos)
            {
                opt = opt.substr(0, eq_pos);
            }

            if (validOptions.find(opt) == validOptions.end()) 
            {
                KMError(_("Unrecognized option “") + arg + "”");
                std::cerr << _("Please use ") << ("'kaiming deltas --help'") << _(" to see available options.") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}

bool KMDeltas::executeRsyncAndProcess(const std::string& sourceDir, const std::string& targetDir, 
                                    const std::string& outputBaseDir) {

    // 判断目录大小差异是否超过50%
    bool useFullCopy = shouldUseFullCopyForDirectories(sourceDir, targetDir);

    // 构建rsync命令
    std::stringstream command;
    command << "rsync -n -c -r -v -l -H --out-format='%n' "
            << "--no-perms --no-owner --no-group --ignore-times --size-only --delete "
            << "\"" << sourceDir << "/" << "\" \"" << targetDir << "/" << "\" 2>&1";

    std::cout << "执行命令: " << command.str() << std::endl;

    // 执行命令并获取输出
    auto outputLines = executeCommand(command.str());
    if (outputLines.empty()) {
        std::cerr << "rsync命令执行失败或没有输出" << std::endl;
        return false;
    }

    // 创建删除日志文件
    fs::path LogPath = fs::path(outputBaseDir) /"files";
    fs::create_directories(LogPath); 
    fs::path deleteLogPath = LogPath / "delete.txt";
    std::ofstream deleteLog(deleteLogPath);
    if (!deleteLog.is_open()) {
        std::cerr << "无法创建删除日志文件: " << deleteLogPath << std::endl;
        return false;
    }

    bool hasOperations = false;

    // 处理每一行输出
    for (const auto& line : outputLines)
    {
        if (line.empty()) continue;

        if (line.find("layers") == 0) 
        {
            // 处理layers开头的文件
            std::string relativePath = line;
            if (!relativePath.empty() && relativePath[0] == '/') {
                relativePath = relativePath.substr(1); // 移除开头的斜杠
            }

            fs::path sourceFile = fs::path(sourceDir) / relativePath;
            fs::path oldFile = fs::path(targetDir) / relativePath;
            fs::path targetFile = fs::path(outputBaseDir) / "files" / relativePath;
            fs::path targetParent = fs::path(targetFile).parent_path();

            // 使用 symlink_status 获取真实的文件类型（不解析软链接）
            auto sym_status = fs::symlink_status(sourceFile);

            // 检查是否不是普通文件（使用 sym_status 而不是直接使用 sourceFile）
            if (!fs::is_regular_file(sym_status))
            {
                // 如果是软链接
                if (fs::is_symlink(sym_status))
                {
                    if (!fs::exists(targetFile))
                    {
                        fs::path targetParent = targetFile.parent_path();
                        if (!fs::exists(targetParent))
                        {
                            fs::create_directories(targetParent);
                        }

                        fs::path linkTarget = fs::read_symlink(sourceFile);
                        fs::create_symlink(linkTarget, targetFile);
                    }
                }
                continue;
            }

            if (!targetParent.empty())
            {
                fs::create_directories(targetParent);
            }

            if (!fs::exists(oldFile))
            {
                // 如果 oldFile 不存在，直接拷贝 sourceFile 到 targetFile
                fs::copy_file(sourceFile, targetFile, fs::copy_options::overwrite_existing);
            }
            else if (useFullCopy)
            {
                // 如果差异超过50%，使用完整复制
                fs::copy_file(sourceFile, targetFile, fs::copy_options::overwrite_existing);
            }
            else
            {
                // 如果 oldFile 存在且差异在50%以内，执行 deltas 命令
                std::string command = std::string(KAIMING_DELTAS) + " -e -f -s \"" + oldFile.string() + "\" \"" + sourceFile.string() + "\" \"" + targetFile.string() + ".xdelta" + "\"";
                int result = std::system(command.c_str());
            }
        }
        else if (line.find("deleting ") == 0) {
            // 处理删除操作
            std::string deletedPath = line.substr(9); // 移除"deleting "前缀
            
            // 记录到删除日志
            deleteLog << deletedPath << std::endl;
            hasOperations = true;
        }
    }

    deleteLog.close();

    return true;
}

std::vector<std::string> KMDeltas::executeCommand(const std::string& command) {
    std::vector<std::string> outputLines;

    // 使用popen执行命令
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        std::cerr << "popen()失败!" << std::endl;
        return outputLines;
    }

    // 读取命令输出
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        std::string line(buffer);
        // 移除行尾的换行符
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        outputLines.push_back(line);
    }

    return outputLines;
}

// bool KMDeltas::copyFileWithDirectories(const std::string& sourcePath, const std::string& targetPath) {
//     try {
//         if (!fs::exists(sourcePath)) {
//             std::cerr << "源文件不存在: " << sourcePath << std::endl;
//             return false;
//         }
        
//         // 确保目标目录存在
//         fs::path targetDir = fs::path(targetPath).parent_path();
//         if (!targetDir.empty()) {
//             fs::create_directories(targetDir);
//         }
        
//         if (fs::is_regular_file(sourcePath)) {
//             // 复制文件
//             fs::copy_file(sourcePath, targetPath, fs::copy_options::overwrite_existing);
//             return true;
//         } else if (fs::is_directory(sourcePath)) {
//             // 复制目录
//             fs::create_directories(targetPath);
//             return true;
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "复制文件时出错 (" << sourcePath << " -> " << targetPath << "): " 
//                  << e.what() << std::endl;
//     }
    
//     return false;
// }

// 判断目录大小差异是否超过50%
bool KMDeltas::shouldUseFullCopyForDirectories(const std::string& sourceDir, const std::string& targetDir) {
    auto getDirSize = [](const std::string& dir) -> uintmax_t {
        uintmax_t size = 0;
        try {
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file()) size += entry.file_size();
            }
        } catch (...) {}
        return size;
    };

    uintmax_t sourceSize = getDirSize(sourceDir);
    uintmax_t targetSize = getDirSize(targetDir);

    if (sourceSize == 0 || targetSize == 0) return true;

    uintmax_t diff = (sourceSize > targetSize) ? (sourceSize - targetSize) : (targetSize - sourceSize);
    uintmax_t maxSize = std::max(sourceSize, targetSize);

    return (static_cast<double>(diff) / maxSize) > 0.5;
}
