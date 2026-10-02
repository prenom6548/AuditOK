#ifndef KMYAMLNODEHELPER_H
#define KMYAMLNODEHELPER_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <yaml-cpp/yaml.h>
/**
 * @brief : 辅助YAML::Node进行标准类型值的获取
 */
class KMYAMLNodeHelper
{
public:
    static YAML::Node loadFromYAML(const QString& yamlFile);
    static YAML::Node loadFromJson(const QString& jsonFile);
    //json数据转换为YAML::Node
    static YAML::Node jsonValueToYaml(const QJsonValue& jsonValue);
public:
    KMYAMLNodeHelper(YAML::Node& node)
        : m_node(node)
    {}

    ~KMYAMLNodeHelper() = default;

    QString getStringValue(const QString &key, const QString &defaultValue = "");
    long getLongValue(const QString &key, long defaultValue = 0L);
    int getIntValue(const QString &key, int defaultValue = 0);
    bool getBoolValue(const QString &key, bool defaultValue = false);
    QVector<QString> getStringListValue(const QString &key);
    QStringList getStringListValue1(const QString &key) const ;
    QMap<QString, QString> getStringMapValue(const QString &key);

private:
    YAML::Node &m_node;
};

#endif // KMYAMLNODEHELPER_H
