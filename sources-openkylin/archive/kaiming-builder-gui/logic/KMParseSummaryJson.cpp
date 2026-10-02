#include "KMParseSummaryJson.h"

#define MODULE "devel"

bool KMParseSummaryJson::loadFile(const QString &path)
{

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QByteArray jsonData = file.readAll();
    file.close();
    jsonDoc = QJsonDocument::fromJson(jsonData);
    return true;
}

void KMParseSummaryJson::arrayToVector(QVector<QString> &vectors, const QJsonArray &arrays)
{
    for (QJsonArray::const_iterator it = arrays.begin(); it != arrays.end(); it ++)
    {
        QJsonValue value_it = *it;
        vectors.push_back(value_it.toString());
    }

}

void KMParseSummaryJson::from_json(const QJsonObject &json, VersionInfo &versioninfo)
{
    for(QJsonObject::const_iterator it = json.begin() ;it != json.end() ; it ++)
    {
        QString key = it.key();
        QJsonValue value = it.value();
        if (key == "installed-size")
        {
            versioninfo.installedSize = value.toInt();
        }
        if (key == "base")
        {
            versioninfo.base = value.toString();
        }
        if (key == "runtime")
        {
            versioninfo.runtime = value.toString();
        }
        if (key == "extensions")
        {
            QJsonArray valueArray = value.toArray();
            if (valueArray.isEmpty())
            {
                continue;
            }
            arrayToVector(versioninfo.extensions, valueArray);
        }
    }
}

void KMParseSummaryJson::from_json(const QJsonObject &json, ModuleInfo &moduleinfo)
{
    for(QJsonObject::const_iterator it = json.begin() ; it != json.end() ; it ++)
    {
        VersionInfo version;
        from_json(it.value().toObject(), version);
        moduleinfo.versionInfo[it.key()] = version;
    }
}

void KMParseSummaryJson::from_json(const QJsonObject &json, IdInfo &idinfo)
{
    for(QJsonObject::const_iterator it = json.begin() ;it != json.end() ; it ++)
    {
        ModuleInfo module;
        from_json(it.value().toObject(), module);
        idinfo.moduleInfo[it.key()] = module;
    }
}

void KMParseSummaryJson::from_json(const QJsonObject &json, KindInfo &kindinfo)
{
    for (QJsonObject::const_iterator it = json.begin(); it != json.end(); it++)
    {
        IdInfo id;
        from_json(it.value().toObject(), id);
        kindinfo.idInfo[it.key()] = id;
    }
}

void KMParseSummaryJson::from_json(const QJsonObject &json, ArchInfo &archinfo)
{
    for (QJsonObject::const_iterator it = json.begin(); it != json.end(); it++)
    {
        KindInfo kind;
        from_json(it.value().toObject(), kind);
        archinfo.kindInfo[it.key()] = kind;
    }
}

void KMParseSummaryJson::from_json(const QJsonObject &json, ChannelInfo & channelinfo)
{
    for (QJsonObject::const_iterator it = json.begin(); it != json.end(); it++)
    {
        ArchInfo arch;
        from_json(it.value().toObject(), arch);
        channelinfo.archInfo[it.key()] = arch;
    }
}

void KMParseSummaryJson::ParseSummay(const QString &path)
{
    if (!loadFile(path))
    {
        return;
    }

    if (jsonDoc.isNull())
    {
        return;
    }

    if (jsonDoc.isObject())
    {
        QJsonObject jsonObject = jsonDoc.object();
        for(QJsonObject::const_iterator it = jsonObject.begin(); it != jsonObject.end(); it++)
        {

            ChannelInfo channel;
            from_json(it.value().toObject(), channel);
            summaryData.channelInfo[it.key()] = channel;
        }

    }
}


QVector<KMRef> KMParseSummaryJson::getRefFromSummaryJson(const QString &channel, const QString &arch, const QString &kind)
{
    QVector<KMRef> refs;
    KindInfo ids = summaryData.channelInfo[channel].archInfo[arch].kindInfo[kind];
    for (QMap<QString , IdInfo>::const_iterator it = ids.idInfo.begin(); it != ids.idInfo.end(); it++)
    {

        QString id = it.key();
        IdInfo modules = it.value();
        ModuleInfo module = modules.moduleInfo[MODULE];
        for(QMap<QString, VersionInfo>::const_iterator it = module.versionInfo.begin(); it != module.versionInfo.end(); it++)
        {
            QString version = it.key();
            if (version.startsWith("name") || version.startsWith("description"))
            {
                continue;
            }
            KMRef ref = KMBuilderStringUtils::getRefFromValue(id, kind, channel, "", version, MODULE);

            refs.push_back(ref);
        }
    }
    return refs;
}

QVector<KMRef> KMParseSummaryJson::getRefFromSummaryJsonForDepend(const QString &channel, const QString &arch, const QString &kind, const QString &base, const QString &runtime)
{
    QVector<KMRef> refs;
    KindInfo ids = summaryData.channelInfo[channel].archInfo[arch].kindInfo[kind];
    for (QMap<QString , IdInfo>::const_iterator it = ids.idInfo.begin(); it != ids.idInfo.end(); it++)
    {
        QString id = it.key();
        IdInfo modules = it.value();
        ModuleInfo module = modules.moduleInfo[MODULE];
        for(QMap<QString, VersionInfo>::const_iterator it = module.versionInfo.begin(); it != module.versionInfo.end(); it++)
        {
            QString version = it.key();
            {
                VersionInfo versioninfo = it.value();
                if (version.startsWith("name") || version.startsWith("description"))
                {
                    continue;
                }
                if (!KMBuilderStringUtils::checkStandard(base, runtime, versioninfo.base, versioninfo.runtime))
                {
                    continue;
                }
                KMRef ref = KMBuilderStringUtils::getRefFromValue(id, kind, channel, "", version, MODULE);
                refs.push_back(ref);
            }
        }
    }
    return refs;
}

QVector<QString> KMParseSummaryJson::getChannels()
{
    return summaryData.channelInfo.keys().toVector();
}
