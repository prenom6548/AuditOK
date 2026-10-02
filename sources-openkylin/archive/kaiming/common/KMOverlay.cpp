#include "KMOverlay.h"

#include <fstream>
#include <filesystem>
#include <sys/utsname.h>

#include "common/KMLogger.h"

namespace fs = std::filesystem;

int KMOverlay::indexFlag = 0; // 0: 未设置, 1: true, 2: false
int KMOverlay::major = 0;     // 内核主版本号
int KMOverlay::minor = 0;     // 内核次版本号

void KMOverlay::checkOverlayConfig()
{
    std::string outputFilePath = "/opt/kaiming-tools/config";
    std::string outputFile = outputFilePath + "/overlay_index.config";

    std::error_code ec;
    // 如果文件存在且大小 > 0，直接返回
    if (fs::exists(outputFile))
    {
        auto fileSize = fs::file_size(outputFile, ec);
        if (!ec && fileSize > 0)
        {
            KMInfo("Overlay index config already exists and is valid");
            return;
        }
    }

    // 获取 uname 信息
    struct utsname uts;
    if (uname(&uts) != 0)
    {
        KMError("Failed to run uname");
        return;
    }
    std::string unameInfo = uts.release;
    std::string configFile = "/boot/config-" + unameInfo;

    // 检查 config 文件是否存在
    if (!fs::exists(configFile))
    {
        KMDebug("Config file not found");
        return;
    }

    // 读取 config 文件，查找关键字
    bool found = false;
    {
        std::ifstream ifs(configFile);
        if (!ifs)
        {
            KMError("Failed to open config file");
            return;
        }

        std::string line;
        while (std::getline(ifs, line))
        {
            if (line == "CONFIG_OVERLAY_FS_INDEX=y")
            {
                found = true;
                break;
            }
        }
    }

    // 写入 output 文件
    {
        // 确保目录存在
        if (!fs::exists(outputFilePath))
        {
            if (!fs::create_directories(outputFilePath, ec))
            {
                KMError("Failed to create config directory");
                return;
            }
        }
        std::ofstream ofs(outputFile);
        if (!ofs)
        {
            KMError("Failed to write to file");
            return;
        }
        ofs << (found ? "CONFIG_OVERLAY_FS_INDEX=true" : "CONFIG_OVERLAY_FS_INDEX=false") << std::endl;
        KMInfo("Write overlay index config to " + outputFile);
    }
}

/**
 * @brief : 获取内核版本号
 * @param : 获取内核版号
 * @param : [out] major 内核主版本号
 * @param : [out] minor 内核次版本号
 * @return: true 成功获取内核版本号，false 获取失败
 */
bool KMOverlay::getKerVersion()
{
    if (major != 0 && minor != 0)
    {
        return true; // 如果已经设置了版本号，直接返回
    }

    struct utsname unameinfo;
    if (uname(&unameinfo) == 0)
    {
        // 提取内核主版本号和次版本号
        std::string kernelVersion = unameinfo.release;
        major = std::stoi(kernelVersion.substr(0, kernelVersion.find('.')));
        minor = std::stoi(kernelVersion.substr(kernelVersion.find('.') + 1, kernelVersion.find_last_of('.') - kernelVersion.find('.') - 1));
        KMDebug("Kernel version: " + kernelVersion);
        return true;
    }

    return false;
}

/**
 * @brief : 判断是overlay是否支持rootless挂载
 * @return: true 支持，false 不支持
 */
bool KMOverlay::isOverlayRootless()
{
    if (major == 0 && minor == 0)
    {
        if (!getKerVersion())
        {
            KMError("Failed to get kernel version");
            return false;
        }
    }
    // 检查内核版本是否大于等于5.11
    return (major > 5 || (major == 5 && minor >= 11));
}

/**
 * @brief : 获取当前用户是否支持userxattr
 * @return: true 支持，false 不支持
 */
bool KMOverlay::isOverlayUserXAttr()
{
    if (major == 0 && minor == 0)
    {
        if (!getKerVersion())
        {
            KMError("Failed to get kernel version");
            return false;
        }
    }
    // 检查内核版本是否大于等于5.19
    return (major > 5) || (major == 5 && minor >= 19);
}

/**
 * @brief 检查内核配置文件中是否设置了 CONFIG_OVERLAY_FS_INDEX=y
 */
bool KMOverlay::isOverlayFsIndexOff()
{
    // 1 代表index true
    if (indexFlag == 1)
    {
        return true;
    }

    // 2 代表index false
    if (indexFlag == 2)
    {
        return false;
    }

    // 0 代表需要从文件中读取
    std::error_code ec;
    std::string configFilePath = "/opt/kaiming-tools/config/overlay_index.config";
    if (!fs::exists(configFilePath, ec))
    {
        indexFlag = 2;
        return false;
    }

    std::ifstream configFile(configFilePath);
    std::string line;
    if (configFile.is_open())
    {
        while (std::getline(configFile, line))
        {
            if (line.find("CONFIG_OVERLAY_FS_INDEX=true") != std::string::npos)
            {
                indexFlag = 1;
                return true;
            }
        }
    }

    // 此时赋值代表已读取过文件，但是index是false
    indexFlag = 2;
    return false;
}