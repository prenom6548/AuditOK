#include "KMBuilderSearch.h"

#include <QDir>
#include <QSet>
#include <QFile>
#include <algorithm>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QString>
#include <QProcess>

#define SUMMARYLOCALPATH "/opt/kaiming/summary.json"
#define REPOPATH "/opt/kaiming/repo/config"
#define MODULEDEVEL "devel"

/**
 * @brief 递归查询当前目录下所有info.json文件
 * @param info.json的绝对路径
 * @param 待查询目录
 */
void KMBuilderSearch::findInfoJson(QVector<QString> &infojsons,const QString &path)
{

    QDir dir(path);
    if (!dir.exists())
    {
        return;
    }

    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    QStringList files = dir.entryList();
    for (const QString &file : files)
    {
        if (file == "info.json")
        {
            infojsons.push_back(dir.absoluteFilePath(file));
        }
    }

    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList subDirs = dir.entryList();

    for (const QString &subDir : subDirs) {
        if (subDir == "entries" || subDir == "files")
        {
            continue;
        }
        findInfoJson(infojsons, dir.absoluteFilePath(subDir));
    }
}

/**
 * @brief 获取base/runtime/depend本地安装家目录
 * @return base/runtime/depend所在目录
 */
QString KMBuilderSearch::getRefPrePath()
{
    QDir homeDir = QDir::home();
    QString homePath = homeDir.absolutePath();
    return KMBuilderStringUtils::buildFilename(homePath,".local/share","kaiming","layers");
;
}

/**
 * @brief 去重和排序
 * @param 数组列表
 * @return
 */

QVector<KMRef> KMBuilderSearch::diffAndSort(const QVector<KMRef> &values)
{
    QVector<KMRef> valuetmp;
    for (auto &value: values)
    {
        if (!valuetmp.contains(value))
        {
            valuetmp.push_back(value);
        }
    }
    std::sort(valuetmp.begin(), valuetmp.end());
    return valuetmp;
}

/**
 * @brief 以Ref结构体形式返回查询到的base/runtime/depend
 * @param channel
 * @param arch
 * @param kind
 * @return 查询到的base/runtime/depend的Ref结构体
 */
QVector<KMRef> KMBuilderSearch::getRef(const QString &channel, const QString &arch, const QString &kind, const QString &base, const QString &runtime)
{
    QVector<KMRef> refs_summary = getRefFromSummary(arch, kind,base, runtime);
    QVector<KMRef> refs = getRefFromLocal(channel, arch, kind, base, runtime);
    KMBuilderStringUtils::appendVector(refs, refs_summary);
    return refs;
}

QVector<KMRef>KMBuilderSearch::getRefFromLocal(const QString &channel, const QString &arch, const QString &kind, const QString &base, const QString &runtime)
{
    QVector<KMRef> refs;
    QString refPrePath = getRefPrePath();
    QString PrePath = KMBuilderStringUtils::buildFilename(refPrePath, channel, arch, kind);

    QVector<QString> infoJsons;
    findInfoJson(infoJsons, PrePath);
    for (auto infojson: infoJsons)
    {
        KMRef ref;
        ref.loadFromInfoJson(infojson);
        if (kind == "depend" && !KMBuilderStringUtils::checkStandard(base, runtime, ref.m_base, ref.m_runtime))
        {
            continue;
        }
        ref.local = true;
        refs.push_back(ref);
    }
    return refs;
}

bool KMBuilderSearch::isRefFromLocal(const QString &kind, const QString &baseStr)
{
    QString refPrePath = getRefPrePath();
    QString arch = KMBuilderStringUtils::getArch();
    KMRef ref;
    ref.parseFromInfo(baseStr);
    QString targetPath = KMBuilderStringUtils::buildFilename(
            refPrePath,
            ref.m_channel,
            arch,
            kind,
            ref.m_id,
            "devel",
            ref.m_version
        );

   QFileInfo deployFile(QDir(targetPath).absoluteFilePath("deploy"));

   return deployFile.exists() && deployFile.isFile();
}

QVector<KMRef> KMBuilderSearch::getRefFromSummary(const QString &arch, const QString &kind, const QString &base, const QString &runtime)
{
    QVector<KMRef> refs;
    QString summaryPath = SUMMARYLOCALPATH;

    if (!checkSummary(summaryPath))
    {
//        QDir homeDir = QDir::home();
//        QString homePath = homeDir.absolutePath();
//        QString summaryParent = KMBuilderStringUtils::buildFilename(homePath,".local","share",".kaiming-builder");
//        if (!KMBuilderStringUtils::DirIsExits(summaryParent))
//        {
//            QDir dirCreate;
//            if (!dirCreate.mkpath(summaryParent))
//            {
//                qDebug() << "summary.json路径制作失败";
//            }
//        }
//        summaryPath =KMBuilderStringUtils::buildFilename(summaryParent, "summary.json");

//        if (!checkSummary(summaryPath))
//        {
//            downloadSummary(summaryPath);
//        }
    }

    KMParseSummaryJson summaryinfo;
    summaryinfo.ParseSummay(summaryPath);
    QVector<QString> channels = summaryinfo.getChannels();
    for (const QString &channel_value: channels)
    {
        QVector<KMRef> channel_ref;
        if (kind == "depend")
        {
            channel_ref = summaryinfo.getRefFromSummaryJsonForDepend(channel_value, arch, kind, base, runtime);
        }
        else
        {
            channel_ref = summaryinfo.getRefFromSummaryJson(channel_value, arch, kind);
        }

        KMBuilderStringUtils::appendVector(refs, channel_ref);
    }
    return refs;
}


/**
 * @brief 检查本地summary是否存在以及是否正确
 * @param channel
 * @param arch
 * @return true:本地summary正确， false:本地summary错误
 */
bool KMBuilderSearch::checkSummary(const QString &filePath)
{
    if (!QFile::exists(filePath))
    {
        return false;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (jsonDoc.isNull() || parseError.error != QJsonParseError::NoError)
    {
        return false;
    }
//    if (KMBuilderStringUtils::calculateTimeDifference(SUMMARYLOCALPATH) >= 3600*24)
//    {
//        return false;
//    }
    return true;
}

/**
 * @brief 解析config/repo获取summary.json所在url
 * @param path
 * @return summary.json所在url
 */
QString KMBuilderSearch::getSummaryUrl(const QString &path)
{
    QFile repoFile(path);
    if (repoFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&repoFile);
        in.setCodec("ASCII");

        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.startsWith("url="))
            {
                return line.mid(4);
            }
        }
    }
    return "";
}

/**
 * @brief 下载文件
 * @param 文件在线路径
 * @param savePath:保存路径
 * @return true:下载成功,false:下载失败
 */
bool KMBuilderSearch::downloadFile(const QString &url, const QString &savePath)
{
    QProcess curlProcess;

    // 构建 curl 命令及其参数
    QStringList arguments;
    arguments << url;
    arguments << "-o" << savePath;

    // 启动 curl 进程
    curlProcess.start("curl", arguments);

    // 等待进程启动
    if (!curlProcess.waitForStarted()) {
        qDebug() << "Failed to start curl process:" << curlProcess.errorString();
        return false;
    }

    // 等待进程完成
    if (!curlProcess.waitForFinished()) {
        qDebug() << "Curl process failed:" << curlProcess.errorString();
        return false;
    }

    // 检查进程退出状态
    int exitCode = curlProcess.exitCode();
    if (exitCode != 0) {
        qDebug() << "Curl exited with non - zero status:" << exitCode;
        qDebug() << "Output:" << curlProcess.readAllStandardOutput();
        qDebug() << "Error:" << curlProcess.readAllStandardError();
        return false;
    }

    return true;
}

/**
 * @brief 从远程下载summary.json
 * @param savePath: 目标路径
 */
void KMBuilderSearch::downloadSummary(const QString &savePath)
{
    QString remoteUrl = getSummaryUrl(REPOPATH);
    QString summaryRemoteUrl = KMBuilderStringUtils::buildFilename(remoteUrl, "summary.json");
    if (!downloadFile(summaryRemoteUrl, savePath))
    {
        qDebug() << "下载summary.json失败";
        return;
    }
}

QVector<KMRef> KMBuilderSearch::search(const QString &kind, const QString &base, const QString &runtime)
{
    QVector<KMRef> refs;

    QString arch = KMBuilderStringUtils::getArch();
    QVector<QString> channels = getChannels();
    if (channels.empty())
    {
        return getRefFromSummary(arch, kind,base, runtime);
    }
    for (const QString &channel: channels)
    {
        QVector<KMRef> channel_ref = getRef(channel, arch, kind, base, runtime);
        KMBuilderStringUtils::appendVector(refs, channel_ref);
    }

    return diffAndSort(refs);
}

QVector<QString> KMBuilderSearch::getChannels()
{
    QString layersPath = getRefPrePath();
    QDir channelsDir(layersPath);
    channelsDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList channels = channelsDir.entryList();
    return channels.toVector();
}

QVector<InstalledPackage> KMBuilderSearch::searchPackage(const QString &name_part, const QString &kind, const QString &channel, const QString &id, const QString &version)
{
    QVector<InstalledPackage> packages;
    QString layersPath = getRefPrePath();
    QString arch = KMBuilderStringUtils::getArch();
    QString file_parent_path = KMBuilderStringUtils::buildFilename(layersPath, channel, arch, kind, id);
    QString dpkg_path =KMBuilderStringUtils::buildFilename(file_parent_path, MODULEDEVEL, version, "files", "dpkg.list");
    if (!KMBuilderStringUtils::fileIsExits(dpkg_path))
    {
        return packages;
    }

    QFile file(dpkg_path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream data(&file);
        while(!data.atEnd())
        {
            QString line = data.readLine();
            QStringList lines = line.split(QRegularExpression("\\s+"));
            if (lines.size() >= 5 &&lines[0] == "ii")
            {
                QString name = lines[1].split(":")[0];
                if (name_part == "" || name.contains(name_part))
                {
                    InstalledPackage package;
                    package.isInstalled = true;
                    package.p_name = name;
                    package.p_version = lines[2];
                    package.p_arch = lines[3];
                    QStringList description = KMBuilderStringUtils::subList(lines, 4, lines.size()-1);
                    package.p_description = description.join(" ");
                    packages.push_back(package);
                }
            }
        }
    }
    return packages;
}
