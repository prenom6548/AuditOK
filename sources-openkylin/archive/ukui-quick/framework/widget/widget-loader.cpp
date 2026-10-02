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

#include "widget-loader.h"
#include "widget.h"
#include "widget-container.h"

#include <QDir>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>

using namespace UkuiQuick;

namespace UkuiQuick {

class WidgetLoaderPrivate
{
public:
    explicit WidgetLoaderPrivate() {
        widgetSearchPath << ":/ukui/widgets"
                         << (QDir::homePath() + "/.local/share/ukui/widgets")
                         << "/usr/share/ukui/widgets"
                         << "/opt/system/resource/ukui/ukui/widgets";
    }

    WidgetMetadata findMetadata(const QString &id) const {
        QMutexLocker locker(&m_lock);
        return m_metadataCache.value(id);
    }

    void cacheMetadata(const WidgetMetadata& metaData) {
        QMutexLocker locker(&m_lock);
        m_metadataCache.insert(metaData.id(), metaData);
//        if (!m_metadataCache.contains(metaData.id())) {
//            m_metadataCache.insert(metaData.id(), metaData);
//        }
    }

    QStringList searchPaths() const {
        QMutexLocker locker(&m_lock);
        return widgetSearchPath;
    }

    void addSearchPath(const QString &path) {
        if (path.isEmpty()) {
            return;
        }

        QMutexLocker locker(&m_lock);
        if (!widgetSearchPath.contains(path)) {
            // 添加的自定义搜索路径优先级最高
            widgetSearchPath.prepend(path);
        }
    }

    WidgetMetadata::Hosts showInFilter() const {
        QMutexLocker locker(&m_lock);
        return hosts;
    }

    void setShowInFilter(WidgetMetadata::Hosts newHosts) {
        QMutexLocker locker(&m_lock);
        hosts = newHosts;
    }

public:
    WidgetMetadata loadMetaData(const QString &id);
    mutable QMutex m_lock;
    QStringList widgetSearchPath;
    WidgetMetadata::Hosts hosts = WidgetMetadata::All;

private:
    QHash<QString, WidgetMetadata> m_metadataCache;
};
}

WidgetMetadata WidgetLoaderPrivate::loadMetaData(const QString &id)
{
    WidgetMetadata metaData = findMetadata(id);
    if (metaData.isValid()) {
        return metaData;
    }

    const QStringList paths = searchPaths();
    for (const auto &path : paths) {
        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }

        QStringList subDirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if (subDirList.contains(id)) {
            qDebug() << "loadMetaData for:" << id;
            metaData = WidgetMetadata(dir.absoluteFilePath(id));
            if (metaData.isValid()) {
                cacheMetadata(metaData);
                return metaData;
            }
            break;
        }
    }

    qWarning() << "can not find widget:" << id;
    return {};
}

/*!
    \class UkuiQuick::WidgetLoader
    \inmodule UkuiQuickFramework
    \inheaderfile widget-loader.h
    \brief 插件加载器

    插件加载器,WidgetLoader用于根据插件ID加载插件,默认加载路径为:
    \code
    {
        ":/ukui/widgets"
        "/usr/share/ukui/widgets"
        "~/.local/share/ukui/widgets"
    }
    \endcode
    可在运行时临时增加查找目录,还继承了插件种类判断功能,插件元数据中有一个类型 \l (host)字段,可以通过指定插件加载器所在的应用(宿主)类型,可以自动过滤掉不属于当前类型的插件.
    例如,在某个应用的插件选择页面只显示可以被加载到当前应用的插件.
    插件加载器的扫描路径逻辑可以根据需求进行扩充,例如:不同分类的插件可以区分不同目录等

    \section1 线程安全

    WidgetLoader 对内部共享状态提供了并发访问保护。这里的“线程安全”仅指以下共享状态：
    \list
        \li 插件元数据缓存
        \li 插件搜索路径列表
        \li showIn/host 过滤条件
    \endlist

    具备上述线程安全保证的方法包括：
    \list
        \li loadMetadata()
        \li addWidgetSearchPath()
        \li setShowInFilter()
        \li widgets()
        \li widgetsMetadata()
    \endlist

    loadWidget() 在访问上述共享状态时同样受到保护；但是该函数会返回一个新的 \l QObject 派生对象
    (\l Widget 或 \l WidgetContainer)。
    返回对象的线程亲和性遵循 Qt 的 QObject 规则：对象归属于创建它的线程，后续使用、信号槽连接和生命周期管理
    需要由调用方按照 Qt 线程模型负责。
    因此，WidgetLoader 的线程安全承诺不包含返回对象本身的跨线程访问契约。
 */

WidgetLoader::WidgetLoader(QObject *parent) : QObject(parent), d(new WidgetLoaderPrivate())
{
}

/*!
    \brief 根据插件 \a id 加载并实例化插件对象.

    该函数访问 WidgetLoader 内部共享状态时是线程安全的；但返回值是新的 \l QObject 派生对象，
    其线程亲和性属于调用线程，后续使用需遵循 Qt 的线程规则。
 */
Widget *WidgetLoader::loadWidget(const QString &id)
{
    WidgetMetadata metaData = loadMetadata(id);
    if (!metaData.isValid()) {
        return nullptr;
    }
    if (!(d->showInFilter() & metaData.showIn())) {
        return nullptr;
    }

    Widget *widget = nullptr;
    if (metaData.widgetType() == Types::Container) {
        widget = new WidgetContainer(metaData);
    } else {
        widget = new Widget(metaData);
    }

    return widget;
}

/*!
    \brief 根据插件 \a id 加载插件元数据.

    该函数是线程安全的。
 */
WidgetMetadata WidgetLoader::loadMetadata(const QString &id)
{
    return d->loadMetaData(id);
}

WidgetLoader::~WidgetLoader()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

/*!
    \brief 向插件加载器添加自定义搜索路径.

    添加的路径会被插入到搜索路径列表的最前面，具有最高优先级。

    该函数是线程安全的。
 */
void WidgetLoader::addWidgetSearchPath(const QString &path)
{
    d->addSearchPath(path);
}

/*!
    \brief 获取全局插件加载器实例.

    返回的实例支持并发调用其线程安全方法。
 */
WidgetLoader &WidgetLoader::globalLoader()
{
    static WidgetLoader loader;
    return loader;
}

/*!
    获取支持当前 \l host 的并且 \c ApplicationId 包含 \a applicationId 的所有插件 \c id.

    该函数是线程安全的。
 */
QStringList WidgetLoader::widgets(const QString &applicationId)
{
    if(applicationId.isEmpty()) {
        return{};
    }
    QStringList widgets;
    const auto hosts = d->showInFilter();
    const QStringList paths = d->searchPaths();
    for (const auto &path : paths) {
        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }

        QStringList subDirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &id : subDirList) {
            WidgetMetadata metaData = d->findMetadata(id);
            if(!metaData.isValid()) {
                metaData = WidgetMetadata(dir.absoluteFilePath(id));
                if(!metaData.isValid()) {
                    continue;
                }
                d->cacheMetadata(metaData);
            }
            if (hosts & metaData.showIn()) {
                if(!metaData.applicationId().contains(applicationId)) {
                    continue;
                }

                if(!widgets.contains(id)) {
                    widgets.append(id);
                }
            }
        }
    }
    return widgets;
}

/*!
    \brief 获取所有已安装插件的元数据列表.

    该函数是线程安全的。
 */
QList<WidgetMetadata> WidgetLoader::widgetsMetadata()
{
    QList<WidgetMetadata> widgets;
    const auto hosts = d->showInFilter();
    const QStringList paths = d->searchPaths();
    for (const auto &path : paths) {
        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }

        QStringList subDirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &id : subDirList) {
            WidgetMetadata metaData = d->findMetadata(id);
            if(!metaData.isValid()) {
                metaData = WidgetMetadata(dir.absoluteFilePath(id));
                if(!metaData.isValid()) {
                    continue;
                }
            }
            if (hosts & metaData.showIn()) {
                d->cacheMetadata(metaData);
                if(!widgets.contains(metaData)) {
                    widgets.append(metaData);
                }
            }
        }
    }
    return widgets;
}

/*!
    \brief 设置插件加载器的宿主类型过滤条件.

    该函数是线程安全的。
 */
void WidgetLoader::setShowInFilter(const WidgetMetadata::Hosts hosts)
{
    d->setShowInFilter(hosts);
}
