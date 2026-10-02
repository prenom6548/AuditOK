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

#include "island.h"
#include "config-loader.h"

#include <QDebug>
#include <utility>

namespace UkuiQuick {

class IslandPrivate
{
public:
    explicit IslandPrivate(QString id, QString app);

    // 当前Island的Id，不能包含路径
    QString islandId;

    // island所在的app名称, 一般是进程名称, 如: ukui-panel, ukui-menu等.
    QString appId;

    // 当前Island的主配置文件
    Config *config {nullptr};

    // island当前加载的顶级View, 有且只有一个顶级View
    WidgetContainer *view {nullptr};
};

IslandPrivate::IslandPrivate(QString id, QString app) : islandId(std::move(id)), appId(std::move(app))
{
}


/*!
    \class UkuiQuick::Island
    \inmodule UkuiQuickFramework
    \inheaderfile island.h
    \brief  岛，存放着一堆 \c view.

    一般情况下，island表示一个应用程序,只为这个应用加载配置文件,并设置基本的组信息,不负责其他业务逻辑，配置文件默认分组为：\c _views.
    一个island有多个 \c View,但是只能有一个处于实例化状态，也就是只能有一个顶级 \c View.
*/


    /*!
        \brief  构造函数
        \a id island的id, \a app 当前应用名称
    */
Island::Island(const QString &id, const QString &app, QObject *parent)
    : QObject(parent), d(new IslandPrivate(id, app))
{
    // _root
    auto config = ConfigLoader::getConfig(id, ConfigLoader::Local, app);
    if (!config) {
        config = new Config(QVariantMap(), this);
        qWarning() << "Island: can not load config for:" << id << app << ", use default.";
    }

    // 初始配置分组信息
    config->addGroupInfo(Island::viewGroupName(), Island::viewGroupKey());
    d->config = config;
}

Island::~Island()
{
    if (d->view) {
        Q_EMIT d->view->aboutToDeleted();
        d->view->deleteLater();
        d->view = nullptr;
    }
    if (d) {
        delete d;
        d = nullptr;
    }
}

/*!
    \brief  获取当前 \c island的配置
*/
Config *Island::config() const
{
    return d->config;
}

/*!
    \brief  获取当前 \c island的 \c id
*/
QString Island::id() const
{
    return d->islandId;
}

/*!
    \brief  获取当前 \c island的应用名称
*/
QString Island::app() const
{
    return d->appId;
}

/*!
    \brief  获取当前island的主view,使用前应该进行判断
*/
WidgetContainer *Island::mainView() const
{
    return d->view;
}

/*!
    \brief  加载配置文件中的一个view,默认加载
    \a id \c view的 \c ID ,\a emitChangeSignal 加载成功后是否发送信号(自动加载 \c ui)
    Returns 加载成功返回 \c true,否则返回 \c false
*/
bool Island::loadMainView(const QString &id, bool emitChangeSignal)
{
    auto list = d->config->children(viewGroupName());
    Config *config = nullptr;
    for(auto c : list) {
        if(c->getValue(QStringLiteral("id")) == id) {
            config = c;
        }
    }

    if (!config) {
        qWarning() << "Config for:" << id << "not found, generating default one";
        QVariantMap data;
        data.insert(QStringLiteral("id"), id);
        int instanceId = d->config->children(UkuiQuick::Island::viewGroupName()).count();
        data.insert(QStringLiteral("instanceId"), instanceId);
        data.insert(QStringLiteral("widgets"), QVariantList());
        d->config->addChild(viewGroupName(), data);
        config = d->config->child(viewGroupName(), instanceId);
    }

    // TODO: 从一个'widget包'进行加载
    auto widget = WidgetContainer::widgetLoader().loadWidget(id);
    auto container = qobject_cast<WidgetContainer *>(widget);
    if (!container) {
        qWarning() << "The main view is not 'Container'" << id;
        delete widget;
        return false;
    }

    container->setInstanceId(config->getValue(QStringLiteral("instanceId")).toInt());
    container->setConfig(config);
    setMainView(container, emitChangeSignal);

    return true;
}

/*!
    \brief  加载配置文件中的一个 \c view.
    \a id \c view的 \c ID ; \a instanceId \c view的 \c instanceId, 用于区分多个相同 \c id的 \c view ; \a emitChangeSignal 加载成功后是否发送信号(自动加载 \c ui)
    Returns 加载成功返回 \c true,否则返回 \c false
*/
bool Island::loadMainView(const QString& id, int instanceId, bool emitChangeSignal)
{
    auto list = d->config->children(viewGroupName());
    Config *config = nullptr;
    for(auto c : list) {
        if(c->getValue(QStringLiteral("id")) == id && c->getValue(QStringLiteral("instanceId")).toInt() == instanceId) {
            config = c;
        }
    }

    if (!config) {
        qWarning() << "Config for:" << id << "not found, generating default one！";        QVariantMap data;
        data.insert(QStringLiteral("id"), id);
        data.insert(QStringLiteral("instanceId"), instanceId);
        data.insert(QStringLiteral("widgets"), QVariantList());
        d->config->addChild(viewGroupName(), data);
        config = d->config->child(viewGroupName(), instanceId);
    }
    auto widget = WidgetContainer::widgetLoader().loadWidget(id);
    auto container = qobject_cast<WidgetContainer *>(widget);
    if (!container) {
        qWarning() << "The main view is not 'Container'" << id;
        delete widget;
        return false;
    }

    container->setInstanceId(instanceId);
    container->setConfig(config);
    setMainView(container, emitChangeSignal);

    return true;
}

/*!
    \brief  手动设置一个主view，不用loader加载.
*/
void Island::setMainView(WidgetContainer *view, bool emitChangeSignal)
{
    if (view == d->view) {
        return;
    }

    if (d->view) {
        Q_EMIT d->view->aboutToDeleted();
        d->view->deleteLater();
        d->view = nullptr;
    }

    d->view = view;
    if (view) {
        view->setIsland(this);
    }

    if(emitChangeSignal) {
        Q_EMIT mainViewChanged();
    }
}

/*!
    \brief  获取内部\c view的 \c 默认分组的组名
*/
QString Island::viewGroupName()
{
    return QStringLiteral("_views");
}

/*!
    \brief  获取\c view分组 \c key
*/
QString Island::viewGroupKey()
{
    return QStringLiteral("instanceId");
}

/*!
    \brief  获取内部\c view的 \c id 列表
*/
QStringList Island::views() const
{
    QStringList list;
    for (const auto &item : d->config->children(Island::viewGroupName())) {
        list << item->getValue(QStringLiteral("id")).toString();
    }

    return list;
}

/*!
    \brief  获取内部\c view的数量
*/
int Island::viewCount() const
{
    return d->config->numberOfChildren(Island::viewGroupName());
}

} // UkuiQuick
