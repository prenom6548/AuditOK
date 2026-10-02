/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOABElf.h"

#include <unistd.h>
#include <fcntl.h>
#include <filesystem>

#include "common/KMStringUtils.h"
#include "common/KMLogger.h"
#include "KMOABUtils.h"
#include "squashfuse/hl.h"

namespace fs = std::filesystem;

class KMOABElf::Private
{
public:
    Private(const std::string &elf);
    ~Private();

    void close();

public:
    std::string m_elfFile;
    int m_fd;
    Elf * m_elf;

    std::string m_metaContent;
    KMOABMetadata m_metadata;
    Elf_Scn * m_metaScn;

    bool m_isAutoUmount;
    std::string m_mountPoint;
    std::string m_cacheDirPrefix;

    std::string m_lastErrorMsg;
};

KMOABElf::Private::Private(const std::string &elf)
    : m_elfFile(elf),
      m_fd(-1),
      m_elf(nullptr),
      m_metaScn(nullptr),
      m_isAutoUmount(true),
      m_cacheDirPrefix("/tmp/.ok/package/")
{
}

void KMOABElf::Private::close()
{
    if (m_elf)
    {
        elf_end(m_elf);
        m_elf = nullptr;
    }

    if (m_fd >= 3)
    {
        ::close(m_fd);
    }
    m_fd = -1;
    m_metaScn = nullptr;
}

KMOABElf::Private::~Private()
{
    close();
}

KMOABElf::KMOABElf(const std::string &elf)
    : d(std::make_shared<Private>(elf))
{}

KMOABElf::~KMOABElf()
{
    try {
        if (d && d->m_isAutoUmount) {
            umountBundle();
        }
    } catch (...) {
        // Do not throw uncaught exceptions in a destructor
        kmlogger.errorLog("Exception during umount in destructor");
    }
}

std::string KMOABElf::metaContent() const
{
    return d->m_metaContent;
}

KMOABMetadata KMOABElf::metadata() const
{
    return d->m_metadata;
}

std::string KMOABElf::appDataDir() const
{
    fs::path app(d->m_elfFile);
    std::string baseName = app.filename().string();
    std::size_t pos = baseName.find_last_of('.');
    if (pos != std::string::npos)
    {
        baseName = baseName.substr(0, pos);
    }

    const std::vector<KMInfoJson>& layers = d->m_metadata.layers;
    for (const KMInfoJson& layer : layers)
    {
        return KMStringUtils::buildFilename(baseName, "layers", layer.id, layer.module);
    }

    return std::string("");
}

std::string KMOABElf::errorMsg() const
{
    return d->m_lastErrorMsg;
}

bool KMOABElf::open()
{
    // 初始化 libelf 库。如果库版本不匹配，程序将返回错误。
    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        d->m_lastErrorMsg = std::string("ELF library initialization failed: ") + elf_errmsg(-1);
        return false;
    }

    d->m_fd = ::open(d->m_elfFile.c_str(), O_RDONLY);
    if (d->m_fd < 0)
    {
        d->m_lastErrorMsg = "Failed to open file : " + d->m_elfFile;
        return false;
    }

    // 打开 ELF 文件进行读取。
    d->m_elf = elf_begin(d->m_fd, ELF_C_READ, nullptr);
    if (!d->m_elf)
    {
        d->m_lastErrorMsg = std::string("Failed to elf_begin : ") + elf_errmsg(-1);
        return false;
    }

    d->m_metaScn = getSection(KMOABMETADATA_PREFIX ".meta");
    if (!d->m_metaScn)
    {
        return false;
    }
    if (!getSectionData(d->m_metaScn, d->m_metaContent))
    {
        return false;
    }
    d->m_metadata.parse(d->m_metaContent);

    return true;
}

Elf_Scn * KMOABElf::getSection(const std::string &sectionName)
{
    if (!d->m_elf)
    {
        d->m_lastErrorMsg = "Please KMOABElf::open first.";
        return nullptr;
    }

    // 获取 section header string table 的索引。
    size_t shstrndx;
    elf_getshdrstrndx(d->m_elf, &shstrndx);

    // 遍历 ELF 文件中的所有 sections。
    Elf_Scn *section = nullptr;
    GElf_Shdr shdr;
    while ((section = elf_nextscn(d->m_elf, section)) != nullptr)
    {
        // 读取每个节的头信息,例如节的大小、类型和偏移位置等。
        if (gelf_getshdr(section, &shdr) != &shdr)
        {
            continue ;
        }

        // 获取 section 名称。
        const char *name = elf_strptr(d->m_elf, shstrndx, shdr.sh_name);
        if (name && sectionName == name)
        {
            return section;
        }
    }

    d->m_lastErrorMsg = "Section '" + sectionName + "' not found.";
    return nullptr;
}

/**
 * @brief : 根据名称获取段/节头信息
 * @param : [in] section, 段/节
 * @return: GElf_Shdr, 节头信息
 */
GElf_Shdr KMOABElf::getSectionHeader(Elf_Scn *section)
{
    GElf_Shdr shdr;
    gelf_getshdr(section, &shdr);

    return shdr;
}

/**
 * @brief : 根据名称获取段/节字符串类型的数据
 * @param : [in] sectionName, 段名
 * @param : [out] data, 段数据
 * @return: true-成功获取；false-失败
 */
// bool KMOABElf::getSectionData(const std::string &sectionName, std::string &data)
// {
//     Elf_Scn *section = getSection(sectionName);
//     if (!section)
//     {
//         return false;
//     }

//     GElf_Shdr shdr;
//     gelf_getshdr(section, &shdr);
//     if (gelf_getshdr(section, &shdr) != &shdr)
//     {
//         d->m_lastErrorMsg = std::string("Failed to gelf_getshdr() : ") + elf_errmsg(-1);
//         return false;
//     }

//     if (shdr.sh_type == SHT_NOBITS)
//     {
//         return false;
//     }

//     Elf_Data *pdata = elf_getdata(section, nullptr);
//     if (!pdata || !pdata->d_buf)
//     {
//         d->m_lastErrorMsg = "Failed to read section data.";
//         return false;
//     }

//     data = std::string(static_cast<char *>(pdata->d_buf), pdata->d_size);
//     return true;
// }

bool KMOABElf::getSectionData(Elf_Scn * section, std::string &data)
{
    GElf_Shdr shdr;
    gelf_getshdr(section, &shdr);
    if (gelf_getshdr(section, &shdr) != &shdr)
    {
        d->m_lastErrorMsg = std::string("Failed to gelf_getshdr() : ") + elf_errmsg(-1);
        return false;
    }

    if (shdr.sh_type == SHT_NOBITS)
    {
        return false;
    }

    Elf_Data *pdata = elf_getdata(section, nullptr);
    if (!pdata || !pdata->d_buf)
    {
        d->m_lastErrorMsg = "Failed to read section data.";
        return false;
    }

    data = std::string(static_cast<char *>(pdata->d_buf), pdata->d_size);
    return true;
}

/**
 * @brief : 解压bundle段的数据到目录dest
 * @param : [in] dest，解压到的目标目录
 * @return: true-解压成功; false-解压失败
 */ 
/*
bool KMOABElf::extractBundle(const std::string &dest)
{
    std::error_code ec;
    if (!KMOABUtils::mkpath(dest, ec))
    {
        d->m_lastErrorMsg = ec.message();
        return false;
    }

    std::string destPath = KMOABUtils::canonical(dest);

    if (!KMOABUtils::isEmptyDirectory(destPath))
    {
        d->m_lastErrorMsg = destPath + " isn't empty";
        return false;
    }

    std::string mountPoint = d->m_cacheDirPrefix + "mountpoint/" + d->m_metadata.uuid;
    if (!mountBundle(mountPoint))
    {
        return false;
    }

    std::vector<std::string> args;
    args.push_back("-rlptv");
    args.push_back("--ignore-errors");
    args.push_back(mountPoint + "/");
    args.push_back(destPath + "/");

    if (!KMOABUtils::spawn("rsync", args, true))
    {
        d->m_lastErrorMsg = "Failed to rsync bundle to " + destPath;
        return false;
    }

    return true;
}
*/

// bool KMOABElf::extractErofsBundle(const std::string &dest)
// {
//     std::string mountPoint = d->m_cacheDirPrefix + "mountpoint/" + d->m_metadata.uuid;
//     if (!mountBundle(mountPoint))
//     {
//         return false;
//     }

//     std::vector<std::string> args;
//     args.push_back("-rlptv");
//     args.push_back("--ignore-errors");
//     args.push_back(mountPoint + "/");
//     args.push_back(dest + "/");

//     if (!KMOABUtils::spawn("rsync", args, true))
//     {
//         d->m_lastErrorMsg = "Failed to rsync bundle to " + dest;
//         return false;
//     }

//     return true;
// }

// bool KMOABElf::extractSquashfsBundle(const std::string &dest)
// {
//     Elf_Scn * bundleScn = getSection(d->m_metadata.sections.bundle);
//     if (!bundleScn)
//     {
//         d->m_lastErrorMsg = "Section '" + d->m_metadata.sections.bundle + "' not found.";
//         return false;
//     }

//     GElf_Shdr shdr = getSectionHeader(bundleScn);
//     Elf64_Off bundleOffset = shdr.sh_offset;

//     std::vector<std::string> args;
//     args.push_back("-d");
//     args.push_back(dest);
//     args.push_back("-f");
//     args.push_back("-ig");
//     args.push_back("-offset");
//     args.push_back(std::to_string(bundleOffset));
//     args.push_back(d->m_elfFile);

//     if (!KMOABUtils::spawn("unsquashfs", args))
//     {
//         d->m_lastErrorMsg = "Failed to unsquashfs bundle";
//         return false;
//     }

//     return true;
// }

/**
 * @brief : 挂载bundle段到挂载点mountPoint
 * @param : [in] mountPoint，挂载点
 * @param : [in] isAutoUmount，是否在对象解析时自动卸载
 * @return: true-挂载成功; false-挂载失败
 */ 
bool KMOABElf::mountBundle(const std::string &mountPoint, bool isAutoUmount)
{
    std::error_code ec;
    if (!KMOABUtils::mkpath(mountPoint, ec))
    {
        d->m_lastErrorMsg = ec.message();
        return false;
    }

    d->m_mountPoint = KMOABUtils::canonical(mountPoint);
    d->m_isAutoUmount = isAutoUmount;

    if (d->m_metadata.bundleFilesystem == BUNDLE_FILESYSTE_EROFS)
    {
        return mountErofsBundle();
    }
    else if (d->m_metadata.bundleFilesystem == BUNDLE_FILESYSTE_SQUASHFS)
    {
        return mountSquashfsBundle();
    }

    d->m_lastErrorMsg = "Failed to mount. Unsupported filesystem : " + d->m_metadata.bundleFilesystem;
    return false;
}

bool KMOABElf::mountErofsBundle()
{
    Elf_Scn * bundleScn = getSection(d->m_metadata.sections.bundle);
    if (!bundleScn)
    {
        d->m_lastErrorMsg = "Section '" + d->m_metadata.sections.bundle + "' not found.";
        return false;
    }

    GElf_Shdr shdr = getSectionHeader(bundleScn);
    Elf64_Off bundleOffset = shdr.sh_offset;

    std::vector<std::string> args;
    args.push_back("--offset=" + std::to_string(bundleOffset));
    args.push_back(d->m_elfFile);
    args.push_back(d->m_mountPoint);

    if (!KMOABUtils::spawn("erofsfuse", args))
    {
        if (d->m_isAutoUmount)
        {
            d->m_mountPoint = "";
        }

        d->m_lastErrorMsg = "Failed to erofsfuse mount bundle to " + d->m_mountPoint;
        return false;
    }

    return true;
}

/*
bool KMOABElf::mountSquashfsBundle()
{
    Elf_Scn * bundleScn = getSection(d->m_metadata.sections.bundle);
    if (!bundleScn)
    {
        d->m_lastErrorMsg = "Section '" + d->m_metadata.sections.bundle + "' not found.";
        return false;
    }

    GElf_Shdr shdr = getSectionHeader(bundleScn);
    Elf64_Off bundleOffset = shdr.sh_offset;

    std::vector<std::string> args;
    args.push_back("-o");
    args.push_back("offset=" + std::to_string(bundleOffset));
    args.push_back(d->m_elfFile);
    args.push_back(d->m_mountPoint);

    std::string exec = KMOABUtils::searchExecutable("squashfuse");
    if (exec.empty())
    {
        d->m_lastErrorMsg = "Program 'squashfuse' is missing, please sudo apt install squashfuse";
        return false;
    }

    if (!KMOABUtils::spawn(exec, args))
    {
        if (d->m_isAutoUmount)
        {
            d->m_mountPoint = "";
        }

        d->m_lastErrorMsg = "Failed to mount bundle to " + d->m_mountPoint;
        return false;
    }

    return true;
}
*/

bool KMOABElf::mountSquashfsBundle()
{
    Elf_Scn * bundleScn = getSection(d->m_metadata.sections.bundle);
    if (!bundleScn)
    {
        d->m_lastErrorMsg = "Section '" + d->m_metadata.sections.bundle + "' not found.";
        return false;
    }

    GElf_Shdr shdr = getSectionHeader(bundleScn);
    Elf64_Off bundleOffset = shdr.sh_offset;

    std::vector<std::string> args;
    args.push_back("-o");
    args.push_back("offset=" + std::to_string(bundleOffset));
    args.push_back(d->m_elfFile);
    args.push_back(d->m_mountPoint);

    std::vector<char *> argv;
    argv.push_back(d->m_elfFile.data());
    for (std::string& arg : args)
    {
        argv.push_back(arg.data());
    }

    std::string exec = KMOABUtils::searchExecutable("squashfuse");
    if (!exec.empty())
    {
        if (!KMOABUtils::spawn(exec, args))
        {
            if (d->m_isAutoUmount)
            {
                d->m_mountPoint = "";
            }

            d->m_lastErrorMsg = "Failed to mount bundle to " + d->m_mountPoint;
            return false;
        }
    }
    else if (squashfuse_main(argv.size(), argv.data()))
    {
        if (d->m_isAutoUmount)
        {
            d->m_mountPoint = "";
        }

        d->m_lastErrorMsg = "Failed to mount bundle to " + d->m_mountPoint;
        return false;
    }

    return true;
}

bool KMOABElf::umountBundle()
{
    if (d->m_mountPoint.empty())
    {
        return true;
    }

    if (!fs::exists(d->m_mountPoint)) {
        d->m_lastErrorMsg = "Mount point does not exist: " + d->m_mountPoint;
        d->m_mountPoint.clear();
        return false;
    }

    std::vector<std::string> args;
    args.push_back(d->m_mountPoint);

    if (!KMOABUtils::spawn("umount", args))
    {
        d->m_lastErrorMsg = "Failed to umount " + d->m_mountPoint;
        return false;
    }

    d->m_mountPoint.clear();
    return true;
}

bool KMOABElf::umountBundle(const std::string &mountPoint)
{
    if (mountPoint.empty())
    {
        return true;
    }

    if (!fs::exists(mountPoint)) {
        return false;
    }

    std::vector<std::string> args;
    args.push_back(mountPoint);

    if (!KMOABUtils::spawn("umount", args))
    {
        return false;
    }

    return true;
}

/**
 * @brief : 重新打包bundle，并更新当前ok文件的bundle段
 * @param : [in] bundleDir, bundle数据
 * @param : [in] newFilesystem, 如果传入，则打包bundle时使用新的文件系统类型
 * @return: true-成功; false-失败
 */ 
bool KMOABElf::repackageBundle(const std::string &bundleDir, const std::string &newFilesystem)
{
    std::string oldBundleFilesystem = d->m_metadata.bundleFilesystem;
    if (!newFilesystem.empty())
    {
        d->m_metadata.bundleFilesystem = newFilesystem;
    }

    std::string cacheDir = KMStringUtils::buildFilename(d->m_cacheDirPrefix, "bundle", d->m_metadata.uuid);
    KMOABUtils::rmRfAll(cacheDir);
    std::error_code ec;
    if (!KMOABUtils::mkpath(cacheDir, ec))
    {
        d->m_lastErrorMsg = ec.message();
        return false;
    }

    d->close();

    KMInfo("Copying replica ...");
    std::string appCopy = KMStringUtils::buildFilename(cacheDir, "ok-engine");
    if (!KMOABUtils::copyAll(d->m_elfFile, appCopy))
    {
        d->m_lastErrorMsg = "Failed to create replica";
        return false;
    }
    std::string metaSection(KMOABMETADATA_PREFIX ".meta");
    removeSection(appCopy, metaSection);
    removeSection(appCopy, d->m_metadata.sections.bundle);

    // 1、生成bundle文件
    bool ret = false;
    std::string bundleFile;
    if (d->m_metadata.bundleFilesystem == BUNDLE_FILESYSTE_EROFS)
    {
        bundleFile = KMStringUtils::buildFilename(cacheDir, "bundle.ef");
        ret = generateErofsBundle(bundleFile, bundleDir);
    }
    else if (d->m_metadata.bundleFilesystem == BUNDLE_FILESYSTE_SQUASHFS)
    {
        bundleFile = KMStringUtils::buildFilename(cacheDir, "bundle.squashfs");
        ret = generateSquashfsBundle(bundleFile, bundleDir);
    }
    else
    {
        d->m_lastErrorMsg = "Failed to repackage. Unsupported filesystem : " + d->m_metadata.bundleFilesystem;
        return false;
    }
    if (!ret)
    {
        d->m_lastErrorMsg = "Failed to generate bundle file.";
        return false;
    }

    // 2、计算bundle文件的校验和, 增加meta段
    std::string digest = KMOABUtils::getFileSha256sum(bundleFile);
    if (digest.empty())
    {
        d->m_lastErrorMsg = "Failed to sha256sum " + bundleFile;
        return false;
    }
    d->m_metadata.digest = digest;

    std::string metadataFile = KMStringUtils::buildFilename(cacheDir, "metadata.json");
    d->m_metadata.saveFile(metadataFile);

    if (!addSection(appCopy, metaSection, metadataFile))
    {
        d->m_lastErrorMsg = "Failed to update section : " + metaSection;
        return false;
    }

    // 3、增加bundle段
    if (!addSection(appCopy, d->m_metadata.sections.bundle, bundleFile))
    {
        d->m_lastErrorMsg = "Failed to update section : " + d->m_metadata.sections.bundle;
        return false;
    }

    // 4、copy icon段
    // if (!d->m_metadata.sections.icon.empty())
    // {
    //     if (copySection(d->m_metadata.sections.icon, d->m_elfFile, appCopy))
    //     {
    //         return false;
    //     }
    // }

    // 5、移动新文件替代旧文件
    if (!KMOABUtils::mv(appCopy, d->m_elfFile))
    {
        d->m_lastErrorMsg = "Replacement failed";
        return false;
    }
    std::string destFile = d->m_elfFile;
    if (!newFilesystem.empty())
    {
        KMStringUtils::replace(destFile, "-" + oldBundleFilesystem + "-", "-" + newFilesystem + "-");
        if (!KMOABUtils::mv(d->m_elfFile, destFile))
        {
            d->m_lastErrorMsg = "Replacement failed";
            return false;
        }
    }

    KMOABUtils::rmRfAll(cacheDir);
    return true;
}

bool KMOABElf::generateErofsBundle(const std::string &bundleFile, const std::string &bundleDir)
{
    std::vector<std::string> args;
    args.push_back("-zlz4hc");
    // args.push_back("-b4096");
    args.push_back("-d3");
    args.push_back(bundleFile);
    args.push_back(bundleDir);

    return KMOABUtils::spawn("mkfs.erofs", args);
}

bool KMOABElf::generateSquashfsBundle(const std::string &bundleFile, const std::string &bundleDir)
{
    std::vector<std::string> args;
    args.push_back(bundleDir);
    args.push_back(bundleFile);
    args.push_back("-info");
    // args.push_back("-comp");
    // args.push_back("xz");

    return KMOABUtils::spawn("mksquashfs", args);
}

/**
 * @brief : 根据数据文件dataFile更新appCopy的sectionName段
 * @param : [in] appCopy, 本执行文件的copy
 * @param : [in] sectionName, 待修改的段名
 * @param : [in] dataFile, 新的段内容所在文件
 * @return: true-成功; false-失败
 * @note  : 在使用 objcopy --update-section 命令时，如果新段内容的大小与原段大小不匹配，可能会导致解析错误。
 *          这是因为 ELF 文件格式对每个段的大小有严格的要求，如果段的大小发生变化，ELF 文件的结构可能会被破坏，从而导致解析错误。
            为了避免这种情况，确保新段内容的大小与原段大小一致。如果新段内容较小，可以使用填充（padding）来使其达到所需的大小；
            如果较大，可能需要重新设计你的程序逻辑以适应新的段大小。
 */ 
// bool KMOABElf::updateSection(const std::string &appCopy, const std::string &sectionName, const std::string &dataFile)
// {
//     std::vector<std::string> args;
//     args.push_back("--update-section");
//     args.push_back(sectionName + "=" + dataFile);
//     args.push_back(appCopy);
//     args.push_back(appCopy);

//     return KMOABUtils::spawn("objcopy", args);
// }

/**
 * @brief : 根据数据文件dataFile增加appCopy的sectionName段
 * @param : [in] appCopy, 本执行文件的copy
 * @param : [in] sectionName, 待增加的段名
 * @param : [in] dataFile, 新的段内容所在文件
 * @return: true-成功; false-失败
 */ 
bool KMOABElf::addSection(const std::string &appCopy, const std::string &sectionName, const std::string &dataFile)
{
    std::vector<std::string> args;
    args.push_back("--add-section");
    args.push_back(sectionName + "=" + dataFile);
    args.push_back(appCopy);
    args.push_back(appCopy);

    return KMOABUtils::spawn("objcopy", args);
}

/**
 * @brief : 根据数据文件dataFile增加appCopy的sectionName段
 * @param : [in] sectionName, 待copy的段名
 * @param : [in] srcElf，源elf文件名
 * @param : [in] destElf, 目标elf文件名
 * @return: true-成功; false-失败
 */ 
// bool KMOABElf::copySection(const std::string &sectionName, const std::string &srcElf, const std::string &destElf)
// {
//     std::string cacheDir = KMStringUtils::buildFilename(d->m_cacheDirPrefix, "bundle", d->m_metadata.uuid);
//     std::error_code ec;
//     if (!KMOABUtils::mkpath(cacheDir, ec))
//     {
//         d->m_lastErrorMsg = ec.message();
//         return false;
//     }

//     // 1. objcopy --only-section=.text file1.elf temp_text_section
//     std::string sectionFile = KMStringUtils::buildFilename(cacheDir, sectionName);
//     std::vector<std::string> args;
//     args.push_back("--only-section");
//     args.push_back(sectionName);
//     args.push_back(srcElf);
//     args.push_back(sectionFile);

//     if (!KMOABUtils::spawn("objcopy", args))
//     {
//         d->m_lastErrorMsg = "Faile to get section : " + sectionName;
//         return false;
//     }

//     // 2. objcopy --add-section .text=temp_text_section file2.elf new_file2.elf
//     if (addSection(destElf, sectionName, sectionFile))
//     {
//         d->m_lastErrorMsg = "Faile to add section : " + sectionName;
//         return false;
//     }

//     // 3. 后面会统一删除缓存目录，此处暂时不清除临时文件

//     return true;
// }

/**
 * @brief : 删除elf文件appCopy的sectionName段
 * @param : [in] appCopy, 本执行文件的copy
 * @param : [in] sectionName, 待增加的段名
 * @return: true-成功; false-失败
 */ 
bool KMOABElf::removeSection(const std::string &appCopy, const std::string &sectionName)
{
    std::vector<std::string> args;
    args.push_back("--remove-section");
    args.push_back(sectionName);
    args.push_back(appCopy);
    args.push_back(appCopy);

    return KMOABUtils::spawn("objcopy", args);
}

