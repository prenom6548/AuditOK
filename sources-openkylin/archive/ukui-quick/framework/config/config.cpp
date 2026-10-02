/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "config.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLockFile>
#include <utility>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QHashIterator>
#include <QEvent>
#include <QCoreApplication>
//#include <QFileSystemWatcher>
//#include <QFileSelector>

namespace UkuiQuick {

ConfigIFace::ConfigIFace(QObject *parent) : QObject(parent)
{

}

// ====== ConfigPrivate ====== //
class ConfigPrivate
{
public:
    explicit ConfigPrivate(QString file);
    explicit ConfigPrivate(QVariantMap d, Config *p);

    void loadConfigFile();
    void save();
    static QFileInfo getFileInfo(const QString &filename);

    QJsonObject toJsonObject();
    static Config *findConfig(const ConfigList &configList, const QString &key, const QVariant &value);

    // 配置文件全路径
    QString configFile;
    QString configLockFile;
    bool requestUpdate {false};

    // 所属组
    QString group;
    // 当前配置的ID值
    QVariant id;
    // 当前节点的分组信息
    ConfigGroupInfo groupInfo;
    // 配置数据核心
    QVariantMap data;
    // 全部子节点
    QHash<QString, ConfigList> children;
    // 父节点，如果是来自于父配置的话
    Config *parentConfig {nullptr};

private:
    friend class Config;
};

ConfigPrivate::ConfigPrivate(QString file)
    : configFile(std::move(file)), configLockFile(file + QStringLiteral(".lock")), id(QStringLiteral("_root"))
{
    loadConfigFile();
}

ConfigPrivate::ConfigPrivate(QVariantMap d, Config *p)
    : data(std::move(d)), parentConfig(p)
{

}

QFileInfo ConfigPrivate::getFileInfo(const QString &filename)
{
    QFileInfo fileInfo(filename);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        bool mpb = dir.mkpath(fileInfo.path());
        qDebug() << "Config: mkPath:" << fileInfo.path() << mpb;
    }

    return fileInfo;
}

void ConfigPrivate::loadConfigFile()
{
    QByteArray jsonData("{}");
    QFileInfo fileInfo = ConfigPrivate::getFileInfo(configFile);
    if (!fileInfo.exists()) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QFile::ReadWrite | QFile::Text)) {
            file.write(jsonData);
            file.flush();
            file.close();
        }

        qDebug() << "Config: File does not exist, initializing to default values：" << jsonData << fileInfo.fileName();
        return;
    }

    if (!fileInfo.isReadable()) {
        qWarning() << "Config: Permission denied to read file：" << configFile;
        return;
    }

    QFile file(fileInfo.absoluteFilePath());
    if (file.open(QFile::ReadOnly)) {
        jsonData = file.readAll();
        file.close();
    }

    QJsonParseError parseError {};
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData, &parseError);
    if (jsonDocument.isNull()) {
        qWarning() << "Config: Failed to parse JSON：" << parseError.errorString() << configFile;
        return;
    }

    if (!jsonDocument.isObject()) {
        qWarning() << "Config: Configuration file format error." << configFile;
        return;
    }

    data = jsonDocument.object().toVariantMap();
}

void ConfigPrivate::save()
{
    if (configFile.isEmpty()) {
        return;
    }

    // 1.读取文件信息，是否只读
    QFileInfo fileInfo = getFileInfo(configFile);
    if (!fileInfo.isWritable()) {
        qWarning() << "Config: No permission to write to the file:" << fileInfo.fileName();
        return;
    }

    // 2.获取lockfile信息
    QLockFile lockFile(configLockFile);
    if (!lockFile.lock()) {
        return;
    }

    QJsonDocument jsonDocument(toJsonObject());
    if (jsonDocument.isObject()) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QFile::ReadWrite | QFile::Truncate | QFile::Text)) {
            qint64 writeLength = file.write(jsonDocument.toJson());
            if (writeLength == -1) {
                qWarning() << "Config::save: File write failure," << file.errorString() << configFile;
            } else {
                qDebug() << "Config::save: File write succeed," << writeLength;
                file.flush();
            }
            file.close();
        } else {
            qWarning() << "Config::save: File open failed," << file.errorString() << configFile;
        }
    } else {
        qWarning() << "Config::save: Failed to parse JSON";
    }

    lockFile.unlock();
}

QJsonObject ConfigPrivate::toJsonObject()
{
    if (children.isEmpty()) {
        return QJsonObject::fromVariantMap(data);
    }

    QJsonObject tempObj = QJsonObject::fromVariantMap(data);
    QHashIterator<QString, ConfigList> iterator(children);
    while (iterator.hasNext()) {
        iterator.next();

        QJsonArray jsonArray;
        for (const auto &config : iterator.value()) {
            jsonArray.append(config->d->toJsonObject());
        }

        tempObj.insert(iterator.key(), jsonArray);
    }

    return tempObj;
}

Config *ConfigPrivate::findConfig(const ConfigList& configList, const QString &key, const QVariant &value)
{
    auto it = std::find_if(configList.constBegin(), configList.constEnd(), [&key, &value] (const Config *config) {
        return config->getValue(key) == value;
    });

    if (it == configList.constEnd()) {
        return nullptr;
    }

    return *it;
}

/*!
    \class UkuiQuick::Config
    \inheaderfile config.h
    \inmodule UkuiQuickFramework
    \brief 配置类,该类用于加载配置文件内容.

    一个配置文件节点,如果从文件或者一个Map生成，那么就是一个根节点。
    如果由父配置构造，那么就是一个子节点，并且有一个指向父节点的指针。
    构造方式：
    \list
    \li 1.从文件加载
    \li 2.从VariantMap加载
    \li 3.从父节点获取
    \endlist
 */


/*!
    \typedef ConfigGroupInfo
    \relates QString

    同 QHash<QString, QString>
*/

/*!
    \typedef ConfigList
    \relates Config

    同 QList<Config*>
*/

// ====== Config ====== //
/*!
    \brief 从文件加载配置.
    \a file 配置文件路径
*/
Config::Config(const QString &file, QObject *parent)
    : ConfigIFace(parent), d(new ConfigPrivate(file))
{

}
/*!
    \brief 从VariantMap加载配置.
    \a data 配置数据
*/
Config::Config(const QVariantMap &data, QObject *parent)
    : ConfigIFace(parent), d(new ConfigPrivate(data, nullptr))
{

}

Config::Config(const QVariantMap &data, Config *parent)
    : ConfigIFace(parent), d(new ConfigPrivate(data, parent))
{

}

Config::~Config()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

/*!
    \brief 提取所有子节点配置

    根据当前配置的groupInfo信息，遍历所有分组并提取对应的子节点配置。
    每个分组会调用extractGroup方法处理具体的子节点提取逻辑。

    \note 如果groupInfo为空则直接返回，不做任何处理
 */
void Config::extractChildren()
{
    if (d->groupInfo.isEmpty()) {
        return;
    }

    QHashIterator<QString, QString> iterator(d->groupInfo);
    while (iterator.hasNext()) {
        iterator.next();
        extractGroup(iterator.key(), iterator.value());
    }
}

/*!
    \brief 提取指定分组配置
 */
void Config::extractGroup(const QString &group, const QString &key)
{
    if (d->data.contains(group)) {
        auto &value = d->data[group];
        QJsonArray jsonArray = value.toJsonArray();

        for (const auto &jsonObjectRef : jsonArray) {
            if (!jsonObjectRef.isObject()) {
                continue;
            }

            QVariantMap data = jsonObjectRef.toObject().toVariantMap();
            addChild(group, data);
        }

        // 清空生成子Config的节点
        value = QVariantList();
    }
}
/*!
    \brief 返回当前配置ID值
 */
const QVariant &Config::id() const
{
    return d->id;
}
/*!
    \brief 返回当前配置分组
 */
const QString &Config::group() const
{
    return d->group;
}

void Config::setId(const QVariant &id)
{
    if (d->id == id) {
        return;
    }

    d->id = id;
}

void Config::setGroup(const QString &group)
{
    if (d->group == group) {
        return;
    }

    d->group = group;
}

/*!
    \brief 返回配置分组信息
 */
const ConfigGroupInfo &Config::groupInfo() const
{
    return d->groupInfo;
}

//void Config::setGroupInfo(const ConfigGroupInfo &groupInfo)
//{
//    if (d->groupInfo.isEmpty() && d->children.isEmpty()) {
//        d->groupInfo = groupInfo;
//        // 将子组存放到children中
//        extractChildren();
//    }
//}

/*!
    \brief 添加分组信息
 */
void Config::addGroupInfo(const QString &group, const QString &key)
{
    if (key.isEmpty() || d->groupInfo.contains(group)) {
        return;
    }

    d->groupInfo.insert(group, key);
    extractGroup(group, key);
}
/*!
    \brief 返回指定分组的子节点列表
 */
ConfigList Config::children(const QString &group) const
{
    if (d->children.contains(group)) {
        return d->children[group];
    }

    return {};
}
/*!
    \brief 返回指定分组的子节点数量
 */
int Config::numberOfChildren(const QString &group) const
{
    if (d->children.contains(group)) {
        return static_cast<int>(d->children[group].count());
    }

    return 0;
}

Config *Config::child(const QString &group, const QVariant &id) const
{
    if (!d->children.contains(group)) {
        return nullptr;
    }

    const auto key = d->groupInfo.value(group);
    const ConfigList &children =  d->children[group];
    return ConfigPrivate::findConfig(children, key, id);
}
/*!
    \brief 返回配置是否为根节点
 */
bool Config::isRoot() const
{
    return d->parentConfig == nullptr;
}

/*!
    \brief 返回配置是否为空
 */
bool Config::isNull() const
{
    return d->data.isEmpty();
}
/*!
    \brief 强制同步配置数据
 */
void Config::forceSync()
{
    requestUpdate();
}
/*!
    \brief 返回所有键值列表
 */
QStringList Config::keys() const
{
    return d->data.keys();
}
/*!
    \brief 返回配置数据
 */
const QVariantMap &Config::data() const
{
    return d->data;
}
/*!
    \brief 根据指定键值返回配置值
 */
QVariant Config::getValue(const QString &key) const
{
    return d->data.value(key);
}
/*!
    \brief 设置指定键的值
 */
void Config::setValue(const QString &key, const QVariant &value)
{
    if (d->groupInfo.contains(key)) {
        return;
    }

    d->data.insert(key, value);
    Q_EMIT configChanged(key);

    requestUpdate();
}
/*!
    \brief 添加子节点配置.
    \a group 分组名称，\a childData 子节点数据
 */
void Config::addChild(const QString &group, const QVariantMap &childData)
{
    const QString key = d->groupInfo.value(group);
    if (key.isEmpty() || !childData.contains(key)) {
        return;
    }

    const auto id = childData.value(key);
    if (id.isNull()) {
        return;
    }

    auto &children = d->children[group];
    auto config = ConfigPrivate::findConfig(children, key, id);
    if (config) {
        return;
    }

    // create childData
    config = new Config(childData, this);
    config->setGroup(group);
    config->setId(id);

    children.append(config);
    requestUpdate();

    Q_EMIT childAdded(group, id);
}
/*!
    \brief 删除指定分组的子节点配置
    \a group 分组名称，\a id 子节点ID
 */
void Config::removeChild(const QString &group, const QVariant &id)
{
    if (!d->children.contains(group)) {
        return;
    }

    auto &children = d->children[group];
    const auto config = ConfigPrivate::findConfig(children, d->groupInfo[group], id);
    if (!config) {
        return;
    }

    children.removeOne(config);
    requestUpdate();
    Q_EMIT childRemoved(group, id);
}
/*!
    \brief 删除指定键值的配置
 */
void Config::removeKey(const QString& key)
{
    d->data.remove(key);
    requestUpdate();
}

bool Config::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest) {
        d->save();
        d->requestUpdate = false;
    }

    return QObject::event(event);
}

void Config::requestUpdate()
{
    if (d->parentConfig) {
        d->parentConfig->requestUpdate();
        return;
    }

    // do sync
    if (!d->requestUpdate) {
        d->requestUpdate = true;
        auto ev = new QEvent(QEvent::UpdateRequest);
        QCoreApplication::postEvent(this, ev);
    }
}
/*!
    \brief 配置是否包含指定键值
 */
bool Config::contains(const QString &key) const
{
    return d->data.contains(key);
}

} // UkuiQuick
