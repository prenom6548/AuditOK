#ifndef KMBUILDERAPI_H
#define KMBUILDERAPI_H
#include "KMBuilderManifest.h"
#include "KMRef.h"
#include "KMBuilderSearch.h"
class KMBuilderApi
{
public:
    /**
     * @brief 加载yaml中数据
     * @param path：yaml文件所在路径
     * @return 包含yaml数据的对象
     */
    static KMBuilderManifest* loadFromYaml(const QString &path);

    static QVector<KMRef> filter(const QVector<KMRef>& refs, const QString &id);

    static std::pair<QVector<KMRef>, QStringList> filterDepends(const QVector<KMRef>& refs, const QString &id);

    static std::pair<QVector<InstalledPackage>, QStringList> findDepsFromRuntimeOrBase(const QVector<InstalledPackage>& packages, const QString &id);

    static std::pair<QStringList, QStringList> findDepsFromAPT(const QString &id);

    static QStringList unionOfStringLists(const QString &id, const QStringList &list1, const QStringList &list2);
};



#endif // KMBUILDERAPI_H
