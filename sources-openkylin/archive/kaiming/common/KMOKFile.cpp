
/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOKFile.h"

#include <iostream>
#include <filesystem>

#include "KMLogger.h"
#include "KMUtil.h"

namespace fs = std::filesystem;

class KMOKFile::Private
{
public:
    std::shared_ptr<KMOABMetadata> m_okMetadata; // ok 文件meta节处理

    std::string m_strFilePath;                // 解析的 ok 文件名
    std::vector<std::string> m_vecMountPoint; // 挂载点，即解压目录
    int fd = -1;                              // 打开的 ok 文件句柄
    Elf *elf = nullptr;                       // Elf 指针
};

KMOKFile::KMOKFile()
    : d(std::make_unique<Private>())
{
    d->m_okMetadata = std::make_shared<KMOABMetadata>(); // ok 文件meta节处理
}

KMOKFile::~KMOKFile()
{
}

bool KMOKFile::loadOKFile(const std::string &filePath)
{
    setOKFilePath(filePath);

    // 初始化 libelf 库。如果库版本不匹配，程序将返回错误。
    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        KMError("Failed to initialize libelf.");
        return false;
    }

    d->fd = ::open(d->m_strFilePath.c_str(), O_RDONLY);
    if (d->fd < 0)
    {
        KMError("Failed to open file: " + d->m_strFilePath);
        return false;
    }

    // 打开 ELF 文件进行读取。
    d->elf = elf_begin(d->fd, ELF_C_READ, nullptr);
    if (!d->elf)
    {
        KMError("Failed to open ELF descriptor.");
        ::close(d->fd);
        return false;
    }
    return true;
}

void KMOKFile::closeOKFile()
{
    if (d->elf)
    {
        elf_end(d->elf);
        d->elf = nullptr;
    }

    if (d->fd >= 0)
    {
        ::close(d->fd);
        d->fd = -1;
    }
}

Elf_Scn *KMOKFile::getHeaderSection(const std::string &sectionName)
{
    if (!d->elf)
    {
        KMError("ELF file : " + d->m_strFilePath + " not opened.");
        return nullptr;
    }

    Elf_Scn *section = nullptr;
    GElf_Shdr shdr;
    size_t shstrndx;

    // 获取 section header string table 的索引。
    elf_getshdrstrndx(d->elf, &shstrndx);

    // 遍历 ELF 文件中的所有 sections。
    while ((section = elf_nextscn(d->elf, section)) != nullptr)
    {
        // 读取每个节的头信息,例如节的大小、类型和偏移位置等。
        gelf_getshdr(section, &shdr);

        // 获取 section 名称。
        const char *name = elf_strptr(d->elf, shstrndx, shdr.sh_name);
        if (name && sectionName == name)
        {
            return section;
        }
    }

    // KMError("Section " + sectionName + " not found.");
    return nullptr;
}

std::string KMOKFile::getMetaInfo(Elf_Scn *section)
{
    GElf_Shdr shdr;
    gelf_getshdr(section, &shdr);

    if (shdr.sh_type == SHT_NOBITS)
    {
        return "";
    }

    //  获取特定 section 的内容并打印到标准输出。
    Elf_Data *data = elf_getdata(section, nullptr);
    if (!data || !data->d_buf)
    {
        KMError("Failed to read section data.");
        return "";
    }

    return std::string(static_cast<char *>(data->d_buf), data->d_size);
}

std::string KMOKFile::getMetaInfo()
{
    Elf_Scn *scn = getHeaderSection(KMOABMETADATA_PREFIX ".meta");
    if (scn == nullptr)
    {
        scn = getHeaderSection("kaiming.meta");
        if (scn == nullptr)
        {
            KMError("Failed to get section : " KMOABMETADATA_PREFIX ".meta");
            return "";
        }
    }
    return getMetaInfo(scn);
}

bool KMOKFile::setMountPoint(const std::string &mountPoint)
{
    if (!fs::exists(mountPoint))
    {
        // 创建挂载点目录。
        {
            std::error_code ec;
            fs::create_directories(mountPoint, ec);
            if (ec)
            {
                KMError("Failed to create mount point directory: " + mountPoint);
                return false;
            }
        }
    }

    // 检查挂载点是否为空。
    if (!fs::is_empty(mountPoint))
    {
        // std::cout << "Mount point directory is not empty, need umount first: " << d->mountPoint << std::endl;

        // 若挂载点为空，则使用 mount 命令卸载挂载点
        std::string mountCmd = "umount " + KMUtil::escapePath(mountPoint);

        int ret = system(mountCmd.c_str());
        if (ret != 0)
        {
            KMError("Failed to mount ok file.");
            return false;
        }
    }

    d->m_vecMountPoint.push_back(mountPoint);
    return true;
}

void KMOKFile::clearMountPoint()
{
    d->m_vecMountPoint.clear();
}

void KMOKFile::setOKFilePath(const std::string &filePath)
{
    d->m_strFilePath = filePath;
}

std::string KMOKFile::getOKFilePath() {
    return d->m_strFilePath;
}

Elf64_Off KMOKFile::getSectionOffset(Elf_Scn *section)
{
    GElf_Shdr shdr;
    gelf_getshdr(section, &shdr);

    // std::cout << "Section offset: " << shdr.sh_offset << " bytes" << std::endl;

    return shdr.sh_offset;
}

bool KMOKFile::mountOK(const std::string &strMountPath)
{
    // 解析 metaInfo 并挂载 section。
    std::string metaInfo = getMetaInfo();
    if (metaInfo.empty())
    {
        KMError("Failed to get meta info.");
        return false;
    }

    // 解析 .meta 数据
    d->m_okMetadata->parse(metaInfo);
    std::string bundleName = d->m_okMetadata->sections.bundle;

    Elf_Scn *scn = nullptr;
    if (d->m_okMetadata->bundleFilesystem == BUNDLE_FILESYSTE_EROFS || d->m_okMetadata->bundleFilesystem.empty())
    {
        // 解析 bundler 并挂载 section。
        scn = getHeaderSection(bundleName);
        if (!scn)
        {
            KMError("Failed to get bundler section.");
            return false;
        }

        Elf64_Off offset = 0;
        offset = getSectionOffset(scn);

        // 拼接 erofsfuse 挂载命令。
        std::string erofsCmd = "erofsfuse --offset=" + std::to_string(offset) + " " + KMUtil::escapePath(d->m_strFilePath) + " " + KMUtil::escapePath(strMountPath);

        // 执行 erofsfuse 挂载命令。
        int ret = system(erofsCmd.c_str());
        if (ret != 0)
        {
            KMError("Failed to mount ok file.");
            return false;
        }
    }
    else if (d->m_okMetadata->bundleFilesystem == BUNDLE_FILESYSTE_SQUASHFS)
    {
        // 解析 bundler 并挂载 section。
        scn = getHeaderSection(bundleName);
        if (!scn)
        {
            KMError("Failed to get bundler section.");
            return false;
        }

        Elf64_Off offset = 0;
        offset = getSectionOffset(scn);

        std::vector<std::string> args;
        args.push_back("-o");
        args.push_back("loop,offset=" + std::to_string(offset));
        args.push_back("-t");
        args.push_back("squashfs");
        args.push_back(d->m_strFilePath);
        args.push_back(strMountPath);

        // 只能 root 权限进行挂载
        std::string result;
        if (!KMProcessUtils::spawn("mount", args, result))
        {
            /* 
                虽然mksquashfs工具能生成LZ4压缩的镜像，但内核必须开启对Squashfs和LZ4的支持（内核配置选项 CONFIG_SQUASHFS_LZ4=y 或 =m）mount才能挂载
                下面尝试使用离线包自挂载功能进行挂载
             */
            args.clear();
            args.push_back("mount");
            args.push_back(strMountPath);
            std::string exec = fs::canonical(d->m_strFilePath);
            if (!KMProcessUtils::spawn(exec, args))
            {
                KMError(result);
                return false;
            }

            KMInfo("Change mount method, " + d->m_strFilePath + " mount " + strMountPath + " success.");
        }
    }

    return true;
}

/**
 * @brief : 挂载bundle
 * @param : [in], mountPath 挂载目录
 * @return: true-挂载成功；false-挂载失败
 * @note  : 
 *          1、和mountOK稍有区别，mountOK只能root用户使用；mountBundle是用pkexec给普通用户提权，从而普通用户也可以挂载
 *          2、需要和umountBundle卸载成对使用
 */
bool KMOKFile::mountBundle(const std::string& mountPath)
{
    // 解析metaInfo
    std::string metaInfo = getMetaInfo();
    if (metaInfo.empty())
    {
        KMError("Failed to get meta info.");
        return false;
    }
    d->m_okMetadata->parse(metaInfo);

    // 挂载bundle
    std::string bundleName = d->m_okMetadata->sections.bundle;
    Elf_Scn * scn = scn = getHeaderSection(bundleName);
    if (!scn)
    {
        KMError("Failed to get bundler section.");
        return false;
    }

    Elf64_Off offset = getSectionOffset(scn);
    if (d->m_okMetadata->bundleFilesystem == BUNDLE_FILESYSTE_SQUASHFS)
    {
        std::vector<std::string> args;
        args.push_back("/usr/bin/mount");
        args.push_back("-o");
        args.push_back("loop,offset=" + std::to_string(offset));
        args.push_back("-t");
        args.push_back("squashfs");
        args.push_back(d->m_strFilePath);
        args.push_back(mountPath);

        std::string result;
        if (!KMProcessUtils::spawn("pkexec", args, result))
        {
            /* 
                虽然mksquashfs工具能生成LZ4压缩的镜像，但内核必须开启对Squashfs和LZ4的支持（内核配置选项 CONFIG_SQUASHFS_LZ4=y 或 =m）mount才能挂载
                下面尝试使用离线包自挂载功能进行挂载
             */
            args.clear();
            args.push_back("mount");
            args.push_back(mountPath);
            std::string exec = fs::canonical(d->m_strFilePath);
            if (!KMProcessUtils::spawn(exec, args))
            {
                KMError(result);
                return false;
            }
        }
    }
    else // if (d->m_okMetadata->bundleFilesystem == BUNDLE_FILESYSTE_EROFS)
    {
        std::vector<std::string> args;
        args.push_back("--offset=" + std::to_string(offset));
        args.push_back(d->m_strFilePath);
        args.push_back(mountPath);

        if (!KMProcessUtils::spawn("erofsfuse", args))
        {
            KMError("Failed to erofsfuse mount bundle to " + mountPath);
            return false;
        }
    }

    return true;
}

/**
 * @brief : 卸载bundle
 * @param : [in], mountPath 卸载目录
 * @return: true-卸载成功；false-卸载失败
 * @note  : 
 *          1、和umountOK稍有区别，umountOK只能root用户使用；umountBundle是用pkexec给普通用户提权，从而普通用户也可以挂载
 *          2、需要和mountBundle成对使用
 */
bool KMOKFile::umountBundle(const std::string& mountPath)
{
    std::vector<std::string> args;
    args.push_back("/usr/bin/umount");
    args.push_back(mountPath);

    if (!KMProcessUtils::spawn("pkexec", args))
    {
        KMError("Failed to umount " + mountPath);
        return false;
    }

    return true;
}

int KMOKFile::umountOK()
{
    auto it = d->m_vecMountPoint.begin();
    while (it != d->m_vecMountPoint.end())
    {
        std::string mountPoint = *it;
        std::string umountCmd = "umount " + KMUtil::escapePath(mountPoint);

        bool success = false;
        if (system(umountCmd.c_str()) == 0)
        {
            success = true;
        }
        else
        {
            // 尝试使用懒卸载
            umountCmd = "umount -l " + KMUtil::escapePath(mountPoint);
            if (system(umountCmd.c_str()) == 0)
            {
                success = true;
            }
            else
            {
                KMError("Failed to umount ok file: " + mountPoint);
            }
        }

        if (success)
        {
            it = d->m_vecMountPoint.erase(it);
            // KMInfo("Successfully umounted and removed: " + mountPoint);
            kmlogger.info("Successfully umounted and removed: %s", mountPoint.c_str());
        }
        else
        {
            ++it;
        }
    }

    return 0;
}

int KMOKFile::umountOK(const std::string& mountPoint)
{
    std::string umountCmd = "umount " + KMUtil::escapePath(mountPoint);

    bool success = false;
    if (system(umountCmd.c_str()) == 0)
    {
        success = true;
    }
    else
    {
        // 尝试使用懒卸载
        umountCmd = "umount -l " + KMUtil::escapePath(mountPoint);
        if (system(umountCmd.c_str()) == 0)
        {
            success = true;
        }
        else
        {
            KMError("Failed to umount ok file: " + mountPoint);
        }
    }

    if (success)
    {
        // 查找要删除的元素
        auto it = std::find(d->m_vecMountPoint.begin(), d->m_vecMountPoint.end(), mountPoint);
        
        // 如果找到了就删除
        if (it != d->m_vecMountPoint.end())
        {
            d->m_vecMountPoint.erase(it);
            kmlogger.info("Successfully umounted and removed: %s", mountPoint.c_str());
        }
    }

    return 0;
}

std::shared_ptr<KMOABMetadata> KMOKFile::getMetadata()
{
    return d->m_okMetadata;
}
