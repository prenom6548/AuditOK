#include "KMBuilderApi.h"
#include "../globalinfo.h"
#include <QProcess>

KMBuilderManifest* KMBuilderApi::loadFromYaml(const QString &path)
{
    KMBuilderManifest* manifest = GlobelInfo::instance().getManifest();
    manifest->loadFromYaml(path);
    return manifest;
}

QVector<KMRef> KMBuilderApi::filter(const QVector<KMRef>& refs, const QString &id)
{
    if (id.isEmpty())
    {
        return refs;
    }
    QVector<KMRef> refs_result;
    QStringList ids = id.split(",");
    for (auto item: ids)
    {
        item = item.trimmed();
        if (item.isEmpty())
        {
            continue;
        }
        QVector<KMRef> refs_filtered;
        for (auto &ref: refs)
        {
            if (ref.m_id.contains(item))
            {
                refs_filtered.push_back(ref);
            }
        }
        KMBuilderStringUtils::appendVector(refs_result, refs_filtered);
    }
    return refs_result;
}

std::pair<QVector<KMRef>, QStringList> KMBuilderApi::filterDepends(const QVector<KMRef>& refs, const QString &id)
{
    QVector<KMRef> matchedRefs;
    QStringList matchedIds;
    if (id.isEmpty())
    {
        return {refs, matchedIds};
    }

    QStringList ids = id.split(",", Qt::SkipEmptyParts);
    for (const QString &item: ids)
    {
        const QString trimmedItem = item.trimmed();
        if (trimmedItem.isEmpty()) continue;

        bool isMatched = false;
        for (const KMRef &ref: refs)
        {
            if (ref.m_id.contains(trimmedItem))
            {
                matchedRefs.append(ref);
                isMatched = true;
            }
        }

        if(isMatched)
        {
            matchedIds.append(trimmedItem);
        }
    }
    return {matchedRefs, matchedIds};
}

std::pair<QVector<InstalledPackage>, QStringList> KMBuilderApi::findDepsFromRuntimeOrBase(const QVector<InstalledPackage>& packages, const QString &id)
{
    QVector<InstalledPackage> matchedPackages;
    QStringList matchedIds;
    if(id.isEmpty())
    {
        return {QVector<InstalledPackage>(), matchedIds};
    }

    QStringList ids = id.split(",", Qt::SkipEmptyParts);
    for (const QString &item: ids)
    {
        const QString trimmedItem = item.trimmed();
        if(trimmedItem.isEmpty()) continue;

        bool isMatched = false;
        for (const InstalledPackage &package: packages)
        {
            if (package.p_name.contains(trimmedItem))
            {
                matchedPackages.append(package);
                isMatched = true;
            }
        }

        if(isMatched)
        {
            matchedIds.append(trimmedItem);
        }
    }
    return {matchedPackages, matchedIds};
}

std::pair<QStringList, QStringList> KMBuilderApi::findDepsFromAPT(const QString &id)
{
    QStringList foundPackages;
    QStringList unmatchedIds;
    if(id.isEmpty())
    {
        return {foundPackages, unmatchedIds};
    }
    QStringList ids = id.split(",", Qt::SkipEmptyParts);
    for (const QString &item: ids)
    {
        const QString trimmedItem = item.trimmed();
        if(trimmedItem.isEmpty()) continue;
        // 仅包含字母、数字、连字符、下划线
        if (!QRegularExpression("^[\\w-]+$").match(trimmedItem).hasMatch())
        {
            qWarning() << "Invalid ID format:" << trimmedItem;
            unmatchedIds.append(trimmedItem);
            continue;
        }

        QString command = QString("apt search %1 | grep '^%2'").arg(trimmedItem).arg(trimmedItem);
        QProcess process;
        process.start("sh", QStringList() << "-c" << command);
        if (!process.waitForFinished(5000)) { // 超时5秒
            unmatchedIds.append(trimmedItem);
            continue;
        }

        if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        {
            qWarning() << "Command failed for:" << trimmedItem << "Exit code:" << process.exitCode();
            unmatchedIds.append(trimmedItem);
            continue;
        }


        QString output = process.readAllStandardOutput();
        if (output.isEmpty()) {
            qDebug() << "No results for:" << trimmedItem;
            unmatchedIds.append(trimmedItem);
            continue;
        }

        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            // 跳过描述行（以空格开头）
            if (line.startsWith(' ')) {
                continue;
            }
            foundPackages << line.trimmed();
        }
    }
    return {foundPackages, unmatchedIds};
}


QStringList KMBuilderApi::unionOfStringLists(const QString &id, const QStringList &list1, const QStringList &list2)
{
    if(id.isEmpty())
    {
        return QStringList();
    }

    QStringList ids = id.split(",", Qt::SkipEmptyParts);

    QSet<QString> unionSet(list1.begin(), list1.end());
    unionSet.unite(QSet<QString>(list2.begin(), list2.end()));

    QStringList unmatched;
    for (const QString &item : ids)
    {
        const QString trimmedItem = item.trimmed();
        if(trimmedItem.isEmpty()) continue;
        if (!unionSet.contains(trimmedItem))
        {
            unmatched.append(trimmedItem);
        }
    }
    return unmatched;
}

