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

#include "widget-metadata.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonArray>
#include <QLocale>
#include <QSharedData>

#include <QDebug>

namespace UkuiQuick {

class WidgetMetadataPrivate : public QSharedData
{
public:
    WidgetMetadataPrivate() : m_isValid(false){};
    WidgetMetadataPrivate(const QString &root);
    WidgetMetadataPrivate(const WidgetMetadataPrivate &other)
        : QSharedData(other),
          m_isValid(other.m_isValid),
          m_id(other.m_id),
          m_root(other.m_root),
          m_object(other.m_object)
    {};
    ~WidgetMetadataPrivate() {};

    static QString localeKey(const QString &key);

    bool m_isValid;
    QString m_id;
    QDir m_root;
    QJsonObject m_object;
};

WidgetMetadataPrivate::WidgetMetadataPrivate(const QString& root)
    : m_isValid(false)
{
    if(root.isEmpty()) {
        return;
    }
    m_root = QDir(root);
    if (!m_root.entryList(QDir::Files).contains("metadata.json")) {
        qWarning() << "loadWidgetPackage: can not find metadata.json, path:" << m_root.path();
        return;
    }

    QFile file(m_root.absoluteFilePath("metadata.json"));
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    QTextStream iStream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    iStream.setCodec("UTF-8");
#else
    iStream.setEncoding(QStringConverter::Utf8);
#endif

    QJsonParseError parseError {};
    QJsonDocument document = QJsonDocument::fromJson(iStream.readAll().toUtf8(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "metadata.json parse error:" << parseError.errorString();
        return;
    }

    if (!document.isObject()) {
        qWarning() << "metadata.json parse error: Is not object.";
        return;
    }

    QJsonObject object = document.object();
    QJsonValue id = object.value(QLatin1String("Id"));
    if (id.type() == QJsonValue::Undefined) {
        qWarning() << "metadata: Id is undefined.";
        return;
    }

    if ((m_id = id.toString()) != m_root.dirName()) {
        qWarning() << "metadata: dir not equal id.";
        return;
    }

    m_object = object;
    m_isValid = true;
}

QString WidgetMetadataPrivate::localeKey(const QString& key)
{
    return (key + "[" + QLocale::system().name() + "]");
}

/*!
    \class UkuiQuick::WidgetMetadata
    \inmodule UkuiQuickFramework
    \inheaderfile widget-metadata.h
    \brief 插件元数据

    WidgetMetadata表示一个插件的原始元数据信息,所谓原始元数据,就是插件在打包时安装的metadata.json文件中提供的信息.
    原始元数据在插件框架中非常重要，它用于描述一个插件，包括插件的名称，\c ID，介绍等基础信息，还有插件的类型 \l Host，国际化，\c UI 入口等信息.
    实际上,他可以类比为原生linux应用的 \c "desktop" 文件. \l WidgetMetaData在一个 \l Widget 首次加载时加载,并且在同一个插件被多次加载时共享.
    \c Widget 提供了在运行时访问全部 \c metadata 信息的功能，并可以修改部分信息，如 \c Tooltip文本等(仅在运行时生效,不会直接修改 \c json 文件).
    \sa UkuiQuick::Widget
*/

/*!
   \enum  WidgetMetadata::Host
    插件适用的容器类型.
   \value Undefined
   \value Panel 用于 \c panel 类型 \c container ，如 \c ukui-panel
   \value SideBar 用于 \c sidebar 类型 \c container ,如 \c ukui-sidebar 中的通知中心等从屏幕侧边划入的面板
   \value Desktop 用于 \c desktop 类型 \c container
   \value TaskManager 可以加载到\c taskManager 的图标上
   \value Shortcut 用于加载到快捷操作面板上，作为对应快捷操作的二级菜单
   \value All 用于所有类型 \c container
   \value Wallpaper 桌面壁纸插件
   \value DesktopView 桌面布局插件
   \value Search 全局搜索插件
 */

/*!
    \brief 构造函数
*/
WidgetMetadata::WidgetMetadata() : d(new WidgetMetadataPrivate)
{
}

/*!
    \brief 从指定路径构造插件元数据.
    \a root 插件的路径，从该路径下的 \c metadata.json 文件加载元数据
*/
WidgetMetadata::WidgetMetadata(const QString &root) : d(new WidgetMetadataPrivate(root))
{
}

/*!
    \brief 拷贝构造函数.
*/
WidgetMetadata::WidgetMetadata(const WidgetMetadata& other): d(other.d)
{
}

/*!
    \brief 获取插件的元数据文件路径.
*/
const QDir &WidgetMetadata::root() const
{
    return d->m_root;
}

/*!
    \brief 拷贝赋值运算符.
*/
WidgetMetadata& WidgetMetadata::operator=(const WidgetMetadata& other)
{
    d = other.d;
    return *this;
}

/*!
    \brief 比较运算符，通过比较 id 判断是否相同.
*/
bool WidgetMetadata::operator==(const WidgetMetadata& other) const
{
    return d->m_id == other.d->m_id;
}

WidgetMetadata::~WidgetMetadata() = default;

/*!
    \brief 判断插件元数据是否有效.
*/
bool WidgetMetadata::isValid() const
{
    return d->m_isValid;
}

/*!
    \brief 获取插件的唯一标识符.
*/
QString WidgetMetadata::id() const
{
    return d->m_id;
}

/*!
    \brief 获取插件的图标.
*/
QString WidgetMetadata::icon() const
{
    QJsonValue value = d->m_object.value(QLatin1String("Icon"));
    if (value.type() == QJsonValue::Undefined) {
        return "application-x-desktop";
    }

    return value.toString();
}

/*!
    \brief 获取插件的本地化名称.

    该方法首先尝试获取当前语言环境下的本地化名称,如果未找到本地化名称,则返回默认名称.
    如果连默认名称也未定义，则返回插件ID作为名称.
*/
QString WidgetMetadata::name() const
{
    QJsonValue value = d->m_object.value(WidgetMetadataPrivate::localeKey("Name"));
    if (value.type() != QJsonValue::Undefined) {
        return value.toString();
    }

    value = d->m_object.value(QLatin1String("Name"));
    if (value.type() == QJsonValue::Undefined) {
        return d->m_id;
    }

    return value.toString();
}

/*!
    \brief 获取插件的本地化提示文本.

    该方法首先尝试获取当前语言环境下的本地化提示文本,如果未找到本地化提示文本,则返回默认提示文本.
    如果连默认提示文本也未定义，则返回插件ID作为提示文本.
*/
QString WidgetMetadata::tooltip() const
{
    QJsonValue value = d->m_object.value(WidgetMetadataPrivate::localeKey("Tooltip"));
    if (value.type() != QJsonValue::Undefined) {
        return value.toString();
    }

    value = d->m_object.value(QLatin1String("Tooltip"));
    if (value.type() == QJsonValue::Undefined) {
        return d->m_id;
    }

    return value.toString();
}

/*!
    \brief 获取插件的版本号.
*/
QString WidgetMetadata::version() const
{
    QJsonValue value = d->m_object.value(QLatin1String("Version"));
    if (value.type() == QJsonValue::Undefined) {
        return "";
    }

    return value.toString();
}

/*!
    \brief 获取插件的网站地址.
*/
QString WidgetMetadata::website() const
{
    QJsonValue value = d->m_object.value(QLatin1String("Website"));
    if (value.type() == QJsonValue::Undefined) {
        return "";
    }

    return value.toString();
}

/*!
    \brief 获取插件的错误报告地址.

    地址用于用户提交插件使用过程中遇到的问题。
*/
QString WidgetMetadata::bugReport() const
{
    QJsonValue value = d->m_object.value(QLatin1String("BugReport"));
    if (value.type() == QJsonValue::Undefined) {
        return "https://gitee.com/openkylin/ukui-panel/issues";
    }

    return value.toString();
}

/*!
    \brief 获取插件的本地化描述文本.

    该方法首先尝试获取当前语言环境下的本地化描述文本,如果未找到本地化描述文本,则返回默认描述文本.
    如果连默认描述文本也未定义，则返回插件ID作为描述文本.
    \sa name()
 */
QString WidgetMetadata::description() const
{
    QJsonValue value = d->m_object.value(WidgetMetadataPrivate::localeKey("Description"));
    if (value.type() != QJsonValue::Undefined) {
        return value.toString();
    }

    value = d->m_object.value(QLatin1String("Description"));
    if (value.type() == QJsonValue::Undefined) {
        return d->m_id;
    }

    return value.toString();
}

/*!
    \brief 获取插件的作者列表.
*/
QVariantList WidgetMetadata::authors() const
{
    QJsonValue value = d->m_object.value(QLatin1String("Authors"));
    if (value.type() == QJsonValue::Undefined || value.type() != QJsonValue::Array) {
        return {};
    }

    return value.toArray().toVariantList();
}

/*!
    \brief 获取插件的内容配置.
*/
QVariantMap WidgetMetadata::contents() const
{
    QJsonValue value = d->m_object.value(QLatin1String("Contents"));
    if (value.type() == QJsonValue::Undefined || value.type() != QJsonValue::Object) {
        return {};
    }

    return value.toObject().toVariantMap();
}

/*!
    \brief 获取插件适用的容器类型.
    \sa Hosts
*/
WidgetMetadata::Hosts WidgetMetadata::showIn() const
{
    QJsonValue value = d->m_object.value(QLatin1String("ShowIn"));
    if (value.type() == QJsonValue::Undefined || value.type() != QJsonValue::String) {
        return All;
    }
    QStringList hostList = value.toString().split(",");
    if (hostList.isEmpty()) {
        return All;
    }
    Hosts hosts = Undefined;
    for (const QString &host : hostList) {
        if (host == QStringLiteral("Panel")) {
            hosts |= Panel;
        } else if (host == QStringLiteral("SideBar")) {
            hosts |= SideBar;
        } else if (host == QStringLiteral("Desktop")) {
            hosts |= Desktop;
        } else if (host == QStringLiteral("All")) {
            hosts |= All;
        } else if (host == QStringLiteral("TaskManager")) {
            hosts |= TaskManager;
        } else if (host == QStringLiteral("Shortcut")) {
            hosts |= Shortcut;
        } else if (host == QStringLiteral("Wallpaper")) {
            hosts |= Wallpaper;
        } else if (host == QStringLiteral("DesktopView")) {
            hosts |= DesktopView;
        } else if (host == QStringLiteral("Search")) {
            hosts |= Search;
        }
    }
    return hosts;
}

/*!
    \brief 获取插件的类型.

    \sa Types::WidgetType
*/
Types::WidgetType WidgetMetadata::widgetType() const
{
    QJsonValue value = d->m_object.value(QLatin1String("WidgetType"));
    if (value.toString() == QStringLiteral("Container")) {
        return Types::Container;
    }

    return Types::Widget;
}

/*!
    \brief 获取插件的应用程序列表.
*/
QStringList WidgetMetadata::applicationId() const
{
    const QJsonValue value = d->m_object.value(QLatin1String("ApplicationId"));
    if (value.type() == QJsonValue::Undefined || value.type() != QJsonValue::Array) {
        return {};
    }
    QStringList ids;
    for (auto id : value.toArray()) {
        ids.append(id.toString());
    }

    return ids;
}

/*!
    \brief 获取插件的缩略图.
 */
QString WidgetMetadata::thumbnail() const
{
    const QJsonValue value = d->m_object.value(QLatin1String("Thumbnail"));
    if (value.type() == QJsonValue::Undefined ) {
        return {};
    }
    return value.toString();
}
}
