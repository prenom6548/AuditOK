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

#include "widget-content.h"
#include "widget-metadata.h"

#include <QVariant>
#include <QDebug>

using namespace UkuiQuick;

namespace UkuiQuick {

class WidgetContentPrivate
{
    friend class WidgetContent;
public:
    explicit WidgetContentPrivate(const WidgetMetadata &m) : contentsRoot(m.root()), contents(m.contents())
    {

    }

private:
    QDir contentsRoot;
    QVariantMap contents;
};

}

/*!
    \class UkuiQuick::WidgetContent
    \inmodule UkuiQuickFramework
    \inheaderfile widget-content.h
    \brief 插件内容.

    该类负责管理插件中的各种资源文件路径，包括主界面文件、国际化文件、配置文件等.
 */

/*!
    \enum WidgetContent::ContentKey
    插件内容类型.

    \value Main 主界面文件
    \value Config 配置文件
    \value I18n 国际化文件
    \value Plugin 动态库文件
    \value PluginVersion 插件版本号
    \value PluginPreload 插件是否预加载
 */

/*!
    \enum WidgetContent::ConfigPolicy
    配置文件行为.

    \value Null 为空,将生成全局配置文件
    \value LocalOnly 插件的配置文件将记录在宿主应用中
*/

WidgetContent::WidgetContent(const WidgetMetadata &metaData) : d(new WidgetContentPrivate(metaData))
{

}

WidgetContent::~WidgetContent()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

/*!
    \brief 获取插件的主界面文件路径.
 */
QString WidgetContent::main() const
{
    if (d->contents.contains(QStringLiteral("Main"))) {
        return d->contents.value("Main").toString();
    }

    return {};
}

/*!
    \brief 获取插件的数据文件路径.
*/
QString WidgetContent::data() const
{
    if (d->contents.contains(QStringLiteral("Data"))) {
        return d->contents.value(QStringLiteral("Data")).toString();
    }

    return {};
}

/*!
    \brief 获取插件的国际化文件路径.
*/
QString WidgetContent::i18n() const
{
    QVariant i18n = d->contents.value(QStringLiteral("I18n"));
    if (i18n.isValid()) {
        if (i18n.userType() == QMetaType::QString) {
            return i18n.toString();
        }
    }

    return {};
}

/*!
    \brief 配置文件行为, \c "LoclOnly" 表示插件的配置文件将记录在宿主应用中,为空时将生成全局配置文件
*/
QString WidgetContent::config() const
{
    QVariant cfg = d->contents.value(QStringLiteral("Config"));
    if (cfg.isValid()) {
        if (cfg.userType() == QMetaType::QString) {
            return cfg.toString();
        }
    }

    return {};
}

/*!
    \brief 获取插件的动态库文件路径.
*/
QString WidgetContent::plugin() const
{
    QVariant plugin = d->contents.value(QStringLiteral("Plugin"));
    if (plugin.isValid()) {
        if (plugin.userType() == QMetaType::QString) {
            return plugin.toString();
        }
    }

    return {};
}


/*!
    \brief 获取插件版本号.
*/
QString WidgetContent::pluginVersion() const
{
    QVariant pluginVersion = d->contents.value(QStringLiteral("PluginVersion"));
    if (pluginVersion.isValid()) {
        if (pluginVersion.userType() == QMetaType::QString) {
            return pluginVersion.toString();
        }
    }

    return {};
}

/*!
    \brief 检查插件是否需要预加载.

    \c true 表示插件在加载时即加载so插件,否则会在调用插件功能时加载 so插件.

*/
bool WidgetContent::pluginPreload() const
{
    QVariant pluginPreload = d->contents.value(QStringLiteral("PluginPreload"));
    if (pluginPreload.isValid()) {
        if (pluginPreload.userType() == QMetaType::Bool) {
            return pluginPreload.toBool();
        }
    }
    return false;
}

/*!
    \brief 获取插件的配置文件行为.

    \sa config()
*/
WidgetContent::ConfigPolicy WidgetContent::configPolicy() const
{
    QString cfg = config();
    if (!cfg.isEmpty() && cfg == QStringLiteral("LocalOnly")) {
        return LocalOnly;
    }

    return Null;
}

/*!
    \brief 获取插件的根目录路径.

    返回插件安装目录的绝对路径，该路径包含了插件的所有资源文件.

*/
QString WidgetContent::rootPath() const
{
    return d->contentsRoot.absolutePath();
}

/*!
    \brief 根据内容类型 \a key 获取对应的文件路径.

    \sa ContentKey
*/
QString WidgetContent::filePath(const WidgetContent::ContentKey &key) const
{
    QString file;
    switch (key) {
        case Main:
            file = main();
            break;
        case I18n: {
            file = i18n();
            break;
        }
        case Plugin: {
            file = plugin();
            break;
        }
        default:
            break;
    }

    if (file.isEmpty()) {
        return {};
    }

    return d->contentsRoot.absoluteFilePath(file);
}

/*!
    \brief 根据内容类型 \a key 获取对应的文件 \c URL.

    特殊处理qrc资源路径，将其转换为QQmlComponent可识别的格式.

    \sa ContentKey
*/
QUrl WidgetContent::fileUrl(const WidgetContent::ContentKey &key) const
{
    if (key == Config) {
        return {};
    }

    QString path = filePath(key);
    if (path.isEmpty()) {
        return {};
    }

    // NOTE: 在此处特殊处理qrc文件路径，将qrc格式的path(如：':/'形式)输出为qrc:/格式的Url。
    // 目的是让QQmlComponent可以自动识别同目录的其他组件。
    if (path.startsWith(QStringLiteral(":/"))) {
        return QUrl(path.prepend(QStringLiteral("qrc")));
    }

    return QUrl::fromLocalFile(path);
}
