#include "KMBuilderStringUtils.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "common/subprocess.h"

QString KMBuilderStringUtils::buildFilename(const QString &str1, const QString &str2)
{
    if (str1.endsWith("/"))
    {
        return str1 + str2;
    }
    return str1 + "/" + str2;
}
QString KMBuilderStringUtils::buildFilename(const QString &str1, const QString &str2, const QString &str3)
{
    return str1 + "/" + str2 + "/" + str3;
}
QString KMBuilderStringUtils::buildFilename(const QString &str1, const QString &str2, const QString &str3, const QString &str4)
{
    return str1 + "/" + str2 + "/" + str3 + "/" + str4;
}
QString KMBuilderStringUtils::buildFilename(const QString &str1, const QString &str2, const QString &str3, const QString &str4, const QString &str5)
{
    return str1 + "/" + str2 + "/" + str3 + "/" + str4 + "/" + str5;
}
QString KMBuilderStringUtils::buildFilename(const QString &str1, const QString &str2, const QString &str3, const QString &str4, const QString &str5, const QString &str6, const QString &str7)
{
    return str1 + "/" + str2 + "/" + str3 + "/" + str4 + "/" + str5 + "/" + str6 + "/" + str7;
}

bool KMBuilderStringUtils::checkStandard(const QString &base, const QString &runtime, const QString &depend_base, const QString &depend_runtime)
{
    if (!base.isEmpty())
    {
        KMRef depend_baseRef = getRefFromInfo(depend_base);
        depend_baseRef.parseVersion();

        KMRef standard_base_Ref = getRefFromInfo(base);
        standard_base_Ref.parseVersion();

        if (!depend_baseRef.checkVersion(standard_base_Ref))
        {
            return false;
        }
    }
    if (!runtime.isEmpty())
    {
        if (depend_runtime.isEmpty())
        {
            return true;
        }
        KMRef depend_runtimeRef = getRefFromInfo(depend_runtime);
        depend_runtimeRef.parseVersion();

        KMRef runtimeRef = getRefFromInfo(runtime);
        runtimeRef.parseVersion();
        if (!depend_runtimeRef.checkVersion(runtimeRef))
        {
            return false;
        }
    }
    return true;
}

KMRef KMBuilderStringUtils::getRefFromInfo(const QString &infoValue)
{
    KMRef ref;
    ref.parseFromInfo(infoValue);
    ref.parseVersion();
    return ref;
}

KMRef KMBuilderStringUtils::getRefFromValue(const QString &id, const QString &kind, const QString &channel, const QString &name, const QString &version, const QString &module)
{
    KMRef ref;
    ref.m_id = id;
    ref.m_kind = kind;
    ref.m_channel = channel;
    ref.m_name = name;
    ref.m_version = version;
    ref.m_module = module;
    return ref;
}

void KMBuilderStringUtils::appendVector(QVector<KMRef> &vector1, QVector<KMRef> &vector2)
{
    for (auto &vec: vector2)
    {
        if (std::find(vector1.begin(), vector1.end(), vec) == vector1.end())
        {
            vector1.push_back(vec);
        }

    }
}

bool KMBuilderStringUtils::regexVersion(const QString &version, bool strict)
{
    const QString versionPattern = "^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)"  // 必须包含四个数字部分
                                       "(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))"    // -先行版本号必须存在 ?:非捕获组，匹配但在结果中不保存
                                       "?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$";

    const QString versionPatternThree = "^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)"    // 必须包含三个数字部分
                                            "(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))" // -先行版本号可选
                                            "?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$";   // 可选是否+

    const QString looseVersionPattern = "^v?(0|[1-9]\\d*)(?:\\.(0|[1-9]\\d*))?(?:\\.(0|[1-9]\\d*))?(?:\\.(0|[1-9]\\d*))"    // 可选前缀v，可以包含1到4个数字部分
                                            "?(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))"  // -先行版本号可选
                                            "?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$";

    const QString looseVersionPatternThree = "^v?(0|[1-9]\\d*)(?:\\.(0|[1-9]\\d*))?(?:\\.(0|[1-9]\\d*))"    // 可选前缀v，可以包含1到3个数字部分
                                                 "?(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))"
                                                 "?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$";
    QString regexMatchFour = strict ? versionPattern: looseVersionPattern;
    QString regexMatchThree = strict ? versionPatternThree: looseVersionPatternThree;
    QRegularExpression regexFour(regexMatchFour);
    QRegularExpression regexThree(regexMatchThree);
    QRegularExpressionMatch matchFour = regexFour.match(version);
    QRegularExpressionMatch matchThree = regexThree.match(version);
    if (!matchFour.hasMatch())
    {
        if (!matchThree.hasMatch())
        {
            return false;
        }
    }
    return true;
}


/**
 * @brief : 查找可执行文件路径
 * @param : [in] exe，可执行文件名
 * @return: 可执行文件绝对路径
 * @note  : 只探索下面目录，如果需要请自行添加
 *          /usr/local/sbin
 *          /usr/local/bin
 *          /usr/bin
 *          /usr/sbin
 *          /bin
 *          /sbin
 */
static QString searchExecutable(const QString &exe)
{
    char * path = std::getenv("PATH");
    QStringList binDirs;
    if (path && path[0] != '\0')
    {
        binDirs = QString(path).split(":");
    }
    else
    {
        QStringList binDirsDefault = {
            "/usr/local/sbin",
            "/usr/local/bin",
            "/usr/sbin",
            "/usr/bin",
            "/sbin",
            "/bin"
        };

        binDirs = binDirsDefault;
    }

    QString result;

    for (const QString& binDir : binDirs)
    {
        QString execPath = KMBuilderStringUtils::buildFilename(binDir, exe);
        QFileInfo fileinfo(execPath);
        if (!fileinfo.exists())
        {
            continue;
        }

        if (!fileinfo.isFile())
        {
            continue;
        }

        return execPath;
    }

    return result;
}

QString KMBuilderStringUtils::packageArch(const QString& platformArch)
{
    QString arch = platformArch;

    if (platformArch == "loongarch64")
    {
        QString exe = searchExecutable("dpkg-architecture");
        if (!exe.isEmpty())
        {
            SubProcess child;
            child.start("dpkg-architecture", { "-q", "DEB_HOST_ARCH" }, true);
            QString result = child.readOutput();
            result.replace("\n", "");
            if (!result.isEmpty())
            {
                arch = result;
            }
        }
    }
    else if (platformArch == "sw_64")
    {
        arch = "sw64";
    }

    return arch;
}

#if !defined(__i386__) && !defined(__x86_64__) && !defined(__aarch64__) && !defined(__arm__)
static QString get_kernel_arch()
{
    static struct utsname buf;
    static char *arch = nullptr;

    if (arch != nullptr)
        return arch;

    if (uname(&buf))
    {
        arch = "unknown";
        return arch;
    }

    arch = buf.machine;

    /* Override for some arches */
    QString mstr = buf.machine;
    if (mstr.length() == 4 && mstr.at(0) == 'i' && mstr.at(2) == '8' && mstr.at(3) == '6')
    {
        arch = "i386";
    }
    else if (mstr == "mips")
    {
#  if G_BYTE_ORDER == G_LITTLE_ENDIAN
        arch = "mipsel";
#  endif
    }
    else if (mstr == "mips64")
    {
#  if G_BYTE_ORDER == G_LITTLE_ENDIAN
        arch = "mips64el";
#  endif
    }

    return arch;
}
#endif

QString KMBuilderStringUtils::getArch()
{
#if defined(__i386__)
    return "i386";
#elif defined(__x86_64__)
    return "x86_64";
#elif defined(__aarch64__)
    return "arm64";
#elif defined(__arm__)
#  if G_BYTE_ORDER == G_LITTLE_ENDIAN
    return "arm";
#  else
    return "armeb";
#  endif
#else
    return packageArch(get_kernel_arch());
#endif
}

QString KMBuilderStringUtils::buildRef(const KMRef &ref, bool depend)
{
    if (ref.m_id.isEmpty())
    {
        return "";
    }
    if (depend)
    {
        return ref.m_id + "/" + ref.m_version;
    }
    return ref.m_channel + ":" + ref.m_id + "/" + ref.m_version;
}

bool KMBuilderStringUtils::DirIsExits(const QString &path)
{
    QDir dir(path);
    if (dir.exists())
    {
        return true;
    }
    return false;
}

bool KMBuilderStringUtils::fileIsExits(const QString &path)
{
    QFile file(path);
    if (file.exists())
    {
        return true;
    }
    return false;
}

QString KMBuilderStringUtils::buildInstalledPackage(const InstalledPackage &package)
{
    return package.p_name + "/" + package.p_version;
}

QStringList KMBuilderStringUtils::subList(const QStringList &originList, int start, int end)
{
    QStringList result;
    if (start < 0 || start >= originList.size() || end < start || end >= originList.size()) {
        return result;
    }
    for (int i = start; i <= end; ++i) {
        result.append(originList.at(i));
    }
    return result;
}

QDateTime KMBuilderStringUtils::getFileTimestamp(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qDebug() << "文件不存在:" << filePath;
        return QDateTime();
    }
    return fileInfo.lastModified();
}

qint64 KMBuilderStringUtils::calculateTimeDifference(const QString& filePath) {
    QDateTime fileTime = getFileTimestamp(filePath);
    if (!fileTime.isValid()) return -1;

    QDateTime currentTime = QDateTime::currentDateTime();
    return fileTime.secsTo(currentTime);
}

QString KMBuilderStringUtils::getpathParent(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.absolutePath();
}

QString KMBuilderStringUtils::getpathRelative(const QString &sourcePath, const QString &targetPath)
{
    QDir sourceDir(sourcePath);
    return sourceDir.relativeFilePath(targetPath);
}

QStringList KMBuilderStringUtils::getColumnData(QTableWidget *table, int column)
{
    QStringList data;
    int rowCount = table->rowCount();

    for (int row = 0; row < rowCount; ++row)
    {
        QTableWidgetItem* item = table->item(row, column);
        if (item)
        {
            data.append(item->text());
        } else
        {
            data.append(""); // 空单元格处理
        }
    }
    return data;
}
