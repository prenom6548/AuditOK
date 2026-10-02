#include "KMYAMLNodeHelper.h"

YAML::Node KMYAMLNodeHelper::loadFromYAML(const QString& yamlFile)
{
    try
    {
        YAML::Node rootNode = YAML::LoadFile(yamlFile.toStdString());
        return rootNode;
    }
    catch (const std::exception &e)
    {
        qWarning() << "YAML::LoadFile failed for" << yamlFile << ":" << e.what();
        return YAML::Node();
    }
}

YAML::Node KMYAMLNodeHelper::jsonValueToYaml(const QJsonValue &jsonValue)
{
    YAML::Node yamlNode;

    switch (jsonValue.type()) {
        case QJsonValue::Null:
            break;

        case QJsonValue::Bool:
            yamlNode = jsonValue.toBool();
            break;

        case QJsonValue::Double:
            yamlNode = jsonValue.toDouble();
            break;

        case QJsonValue::String:
            yamlNode = jsonValue.toString().toStdString();
            break;

        case QJsonValue::Array: {
            QJsonArray jsonArray = jsonValue.toArray();
            yamlNode = YAML::Node(YAML::NodeType::Sequence);
            for (const auto& value : jsonArray) {
                yamlNode.push_back(jsonValueToYaml(value));
            }
            break;
        }

        case QJsonValue::Object: {
            QJsonObject jsonObject = jsonValue.toObject();
            yamlNode = YAML::Node(YAML::NodeType::Map);
            for (const auto& key : jsonObject.keys()) {
                yamlNode[key.toStdString()] = jsonValueToYaml(jsonObject[key]);
            }
            break;
        }
        case QJsonValue::Undefined:
            break;
    }

    return yamlNode;
}

YAML::Node KMYAMLNodeHelper::loadFromJson(const QString &jsonFile)
{
    QFile file(jsonFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file:" << jsonFile;
        return YAML::Node();
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qDebug() << "JSON parse error:" << parseError.errorString();
        return YAML::Node();
    }
    if (jsonDoc.isObject())
    {
        return jsonValueToYaml(QJsonValue(jsonDoc.object()));
    }
    else if (jsonDoc.isArray())
    {
        return jsonValueToYaml(QJsonValue(jsonDoc.array()));
    }

    return YAML::Node();
}

QString KMYAMLNodeHelper::getStringValue(const QString &key, const QString &defaultValue)
{
    YAML::Node value = m_node[key.toStdString()];
    if (!value.IsDefined())
    {
        return defaultValue;
    }

    if (value.IsScalar())
    {
        return QString::fromStdString(value.as<std::string>());
    }

    return defaultValue;
}

long KMYAMLNodeHelper::getLongValue(const QString &key, long defaultValue)
{
    YAML::Node value = m_node[key.toStdString()];
    if (!value.IsDefined())
    {
        return defaultValue;
    }

    if (value.IsScalar())
    {
        return value.as<long>();
    }

    return defaultValue;
}

int KMYAMLNodeHelper::getIntValue(const QString &key, int defaultValue)
{
    YAML::Node value = m_node[key.toStdString()];
    if (!value.IsDefined())
    {
        return defaultValue;
    }

    if (value.IsScalar())
    {
        return value.as<int>();
    }

    return defaultValue;
}

bool KMYAMLNodeHelper::getBoolValue(const QString &key, bool defaultValue)
{
    YAML::Node value = m_node[key.toStdString()];
    if (!value.IsDefined())
    {
        return defaultValue;
    }

    if (value.IsScalar())
    {
        return value.as<bool>();
    }

    return defaultValue;
}

QVector<QString> KMYAMLNodeHelper::getStringListValue(const QString &key)
{
    QVector<QString> defaultValue;

    YAML::Node value = m_node[key.toStdString()];
    if (!value.IsDefined())
    {
        return defaultValue;
    }

    if (value.IsSequence())
    {
        for (YAML::Node::const_iterator iter = value.begin(); iter != value.end(); iter++)
        {
            if ((*iter).IsScalar())
            {
                defaultValue.push_back(QString::fromStdString((*iter).as<std::string>()));
            }
        }
    }

    return defaultValue;
}

QStringList KMYAMLNodeHelper::getStringListValue1(const QString &key) const
{
    QStringList result;

    YAML::Node value = m_node[key.toStdString()];
    if (!value.IsDefined() || !value.IsSequence())
    {
        return result; // 返回空列表
    }

    for (const auto &item : value)
    {
        if (item.IsScalar())
        {
            result.append(QString::fromStdString(item.as<std::string>()));
        }
    }

    return result;
}

QMap<QString, QString> KMYAMLNodeHelper::getStringMapValue(const QString &key)
{
    QMap<QString, QString> defaultValue;

    YAML::Node node = m_node[key.toStdString()];
    if (!node.IsDefined())
    {
        return defaultValue;
    }

    if (node.IsMap())
    {
        for(YAML::Node::const_iterator iter = node.begin() ; iter != node.end() ; iter++)
        {
            QString key = QString::fromStdString(iter->first.as<std::string>());
            YAML::Node value = iter->second;
            if (value.IsScalar())
            {
                defaultValue[key] = QString::fromStdString(value.as<std::string>());
            }
        }
    }
    return defaultValue;
}
