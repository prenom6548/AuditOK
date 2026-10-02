/*
 * Qt5-UKUI's Library
 *
 * 为 KDirTabBar 提供真实文件系统子目录列举（对齐 kdk::KDirTabBar 的 QDirIterator）。
 */

#include "kydirhelper.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QVariantMap>

namespace UKUISDKStyle {

KyDirHelper::KyDirHelper(QObject *parent)
    : QObject(parent)
{
}

QString KyDirHelper::homePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

QString KyDirHelper::normalizePath(const QString &path) const
{
    QString p = path;
    if (p.isEmpty())
        return p;
    while (p.length() > 1 && p.endsWith(QLatin1Char('/')))
        p.chop(1);
    return p;
}

QString KyDirHelper::folderIconName(const QString &baseName)
{
    if (baseName == QStringLiteral("图片"))
        return QStringLiteral("folder-pictures");
    if (baseName == QStringLiteral("下载"))
        return QStringLiteral("folder-download");
    if (baseName == QStringLiteral("公共的"))
        return QStringLiteral("folder-publicshare");
    if (baseName == QStringLiteral("文档"))
        return QStringLiteral("folder-documents");
    if (baseName == QStringLiteral("桌面"))
        return QStringLiteral("user-desktop");
    if (baseName == QStringLiteral("模板"))
        return QStringLiteral("folder-templates");
    if (baseName == QStringLiteral("视频"))
        return QStringLiteral("folder-videos");
    if (baseName == QStringLiteral("音乐"))
        return QStringLiteral("folder-music");
    return QStringLiteral("folder");
}

QVariantList KyDirHelper::listSubDirectories(const QString &path) const
{
    QVariantList result;
    const QString dirPath = normalizePath(path);
    if (dirPath.isEmpty())
        return result;

    QFileInfo info(dirPath);
    if (!info.exists() || !info.isDir())
        return result;

    QDirIterator it(dirPath, QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        it.next();
        const QFileInfo fileInfo = it.fileInfo();
        if (!fileInfo.isDir())
            continue;

        QVariantMap item;
        item.insert(QStringLiteral("text"), fileInfo.fileName());
        item.insert(QStringLiteral("uri"), fileInfo.absoluteFilePath());
        item.insert(QStringLiteral("icon"), folderIconName(fileInfo.baseName()));
        result.append(item);
    }
    return result;
}

} // namespace UKUISDKStyle
