#include "KMRef.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

void KMRef::loadFromInfoJson(QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull())
    {
        return;
    }

    if (jsonDoc.isObject())
    {

        QJsonObject jsonObject = jsonDoc.object();

        if (jsonObject.contains("id") && jsonObject["id"].isString())
        {
            m_id = jsonObject["id"].toString();
        }
        if (jsonObject.contains("name") && jsonObject["name"].isString())
        {
            m_name = jsonObject["name"].toString();
        }
        if (jsonObject.contains("kind") && jsonObject["kind"].isString())
        {
            m_kind = jsonObject["kind"].toString();
        }
        if (jsonObject.contains("version") && jsonObject["version"].isString())
        {
            m_version = jsonObject["version"].toString();
        }

        if (jsonObject.contains("channel") && jsonObject["channel"].isString())
        {
            m_channel = jsonObject["channel"].toString();
        }

        if (jsonObject.contains("module") && jsonObject["module"].isString())
        {
            m_module = jsonObject["module"].toString();
        }
        if (jsonObject.contains("base") && jsonObject["base"].isString())
        {
            m_base = jsonObject["base"].toString();
        }
        if (jsonObject.contains("runtime") && jsonObject["runtime"].isString())
        {
            m_runtime = jsonObject["runtime"].toString();
        }
    }
}

/**
 * @brief getStr
 * @return 形式channel:id/version
 */
QString KMRef::getStandard()
{
    return m_channel + ":" + m_id + "/" + m_version;
}

/**
 * @brief 获取格式channel:id/version的base/runtime
 * @return 形式channel:id/version
 */
QString KMRef::getStardardDepend()
{
    return m_id + "/" + m_version;
}

/**
 * @brief 解析channel:id/version的base或runtime
 * @param baseStr
 */

void KMRef::parseFromInfo(const QString &baseStr)
{
    QStringList channel_id_version = baseStr.split(":");
    QStringList id_version = channel_id_version[1].split("/");
    m_channel = channel_id_version[0];
    m_id = id_version[0];
    m_version = id_version[1];
}

void KMRef::parseVersion()
{
    QStringList major_minor = m_version.split(".");
    m_major = major_minor[0].toInt();
}

bool KMRef::checkVersion(const KMRef &another)
{

    if (m_id != another.m_id || m_major != another.m_major)
    {
        return false;
    }
    return true;
}

