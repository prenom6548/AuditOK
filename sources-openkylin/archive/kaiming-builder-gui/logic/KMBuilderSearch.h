#ifndef KMBUILDERSEARCH_H
#define KMBUILDERSEARCH_H
#include <QString>
#include <QFile>

#include "KMRef.h"
#include "KMBuilderStringUtils.h"
#include "KMParseSummaryJson.h"
#include "KMStructData.h"



/**
 * 使用说明
 * @brief 调用KMBuilderSearch::search(kind,base,runtime)。其中base和runtime当depend时传入即可，若无可传""。
 */

class KMBuilderSearch
{
public:
    /**
     * @brief 以Ref结构体形式返回查询到的base/runtime
     * @param channel
     * @param arch
     * @param kind
     * @return 查询到的base/runtime/depend的Ref结构体
     */
    static QVector<KMRef> getRef(const QString &channel, const QString &arch, const QString &kind, const QString &base, const QString &runtime);
    static QVector<KMRef> getRefFromLocal(const QString &channel, const QString &arch, const QString &kind, const QString &base, const QString &runtime);
    static bool isRefFromLocal(const QString &kind, const QString &baseStr);
    static QVector<KMRef> getRefFromSummary(const QString &arch, const QString &kind, const QString &base, const QString &runtime);

    /**
     * @brief 获取格式为{channel}:{id}/{version}的base/runtime列表
     * @param channel
     * @param arch
     * @param kind
     * @return 格式为{channel}:{id}/{version}的base/runtime列表
     */
    static QVector<QString> getValueList(const QString &channel, const QString &arch, const QString &kind);

    /**
     * @brief 获取格式为{id}/{version}的depend集合。
     * @param channel
     * @param arch
     * @param kind
     * @param base 如果选择depend之前已经选择了base，需要进行校验
     * @param runtime 同base
     * @return 格式为{id}/{version}的depend集合。
     */
    static QVector<KMRef> getValueListForDepend(const QString &channel, const QString &arch, const QString &kind, const QString &base, const QString &runtime);

public:

    /**
     * @brief 递归查询当前目录下所有info.json文件
     * @param info.json的绝对路径
     * @param 待查询目录
     */
    static void findInfoJson(QVector<QString> &infojsonList,const QString &path);

    /**
     * @brief 获取base/runtime/depend本地安装家目录
     * @return
     */
    static QString getRefPrePath();

    /**
     * @brief 去重和排序
     * @param 数组列表
     * @return
     */
    static QVector<KMRef> diffAndSort(const QVector<KMRef> &values);

    /**
     * @brief 检查本地summary是否存在以及是否正确
     * @param channel
     * @param arch
     * @return true:校验合格，false:校验不合格
     */
    static bool checkSummary(const QString & filepath);

    /**
     * @brief 从远程下载summary.json
     * @param savePath: 目标路径
     */
    static void downloadSummary(const QString &savePath);

    /**
     * @brief 解析config/repo获取summary.json所在url
     * @param path
     * @return summary.json所在url
     */
    static QString getSummaryUrl(const QString &path);

    /**
     * @brief 下载文件
     * @param 文件在线路径
     * @param savePath:保存路径
     * @return true:下载成功,false:下载失败
     */
    static bool downloadFile(const QString &url, const QString &savePath);
    /**
     * @brief 查询base/runtime/depend并以结构体ref数组形式返回
     * @param kind
     * @param base
     * @param runtime
     * @return 查询到的ref结构体数组
     */
    static QVector<KMRef> search(const QString &kind,const QString &base = "", const QString &runtime = "");

    /**
     * @brief 获取本地channels
     * @return
     */
    static QVector<QString> getChannels();

    /**
     * @brief 查询软件包列表
     * @param preName
     * @param kind
     * @param channel
     * @param id
     * @param version
     * @return 软件包列表
     */
    static QVector<InstalledPackage> searchPackage(const QString &name_part, const QString &kind,const QString &channel, const QString &id, const QString &version);

};


#endif // KMBUILDERSEARCH_H
