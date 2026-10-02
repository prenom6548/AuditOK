#ifndef KMPARSESUMMARYJSON_H
#define KMPARSESUMMARYJSON_H

#include "KMRef.h"
#include "KMBuilderStringUtils.h"

#include <QMap>
#include <QVector>
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

struct VersionInfo
{
    uint64_t installedSize = 0;         // 安装大小
    QString base = "";                   // 依赖的 base
    QString runtime = "";                // 依赖的 runtime
    QVector<QString> depend;
    QVector<QString> extensions = { "" }; // 依赖的 extensions 列表
};

struct ModuleInfo
{
    QMap<QString, VersionInfo> versionInfo;  // 0.0.0.1, 0.0.0.2
};

struct IdInfo
{
    QMap<QString, ModuleInfo> moduleInfo;  // binary/debuginfo/locale_zh_cn
};

struct KindInfo
{
    QMap<QString, IdInfo> idInfo; // top.openkylin.Music, top.openkylin.Video
};

struct ArchInfo
{
    QMap<QString, KindInfo> kindInfo; // app/runtime/base/extension
};

struct ChannelInfo
{
    QMap<QString, ArchInfo> archInfo; // x86_64/arm64/loongarch
};

struct SummaryInfo
{
    QMap<QString, ChannelInfo> channelInfo; // main/stable
};

class KMParseSummaryJson
{
private:
    SummaryInfo summaryData;
    QJsonDocument jsonDoc;

public:
    /**
     * @brief 解析summary.json
     * @param summary.json所在路径
     * @return
     */
    void ParseSummay(const QString &path);
    bool loadFile(const QString &path);

    /**
     * @brief json内容保存到结构体中
     * @param json对象
     * @param 结构体
     */
    void from_json(const QJsonObject &json, ChannelInfo & channelinfo);
    void from_json(const QJsonObject &json, ArchInfo &archinfo);
    void from_json(const QJsonObject &json, KindInfo &kindinfo);
    void from_json(const QJsonObject &json, IdInfo &idinfo);
    void from_json(const QJsonObject &json, ModuleInfo &moduleinfo);
    void from_json(const QJsonObject &json, VersionInfo & versioninfo);

    void arrayToVector(QVector<QString> &vectors, const QJsonArray &arrays);

    QVector<KMRef> getRefFromSummaryJson(const QString &channel, const QString &arch, const QString &kind);
    QVector<KMRef> getRefFromSummaryJsonForDepend(const QString &channel, const QString &arch, const QString &kind, const QString &base, const QString &runtime);


    QVector<QString> getChannels();

};

#endif // KMPARSESUMMARYJSON_H
