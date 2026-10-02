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

#include "widget-container.h"

#include <QJsonArray>

#include "widget.h"
#include "island.h"

namespace UkuiQuick {

class WidgetContainerPrivate
{
public:
    WidgetContainerPrivate(WidgetContainer *q) : q(q){};
    bool containActiveWidgets();
    void setDisableInstancesId(const QList<int>& instanceIds, bool removeDumplicated = true);
    void loadConfig(Config* config);
    void removeDisableInstances();

    // prop
    int radius {0};
    QRect geometry;
    QScreen *screen {nullptr};
    Margin *margin {nullptr};
    Margin *padding {nullptr};
    Types::Pos position {Types::NoPosition};

    // widgets
    QVector<Widget *> widgets;
    Island *island {nullptr};
    WidgetMetadata::Host host = WidgetMetadata::All;
    WidgetMetadata::Hosts hosts = WidgetMetadata::All;
    bool configLoaded = false;
    QList<int> disableInstances;
    WidgetContainer *q = nullptr;
};

bool WidgetContainerPrivate::containActiveWidgets()
{
    return std::any_of(widgets.constBegin(), widgets.constEnd(), [] (Widget *widget) {
        return widget->active();
    });
}

void WidgetContainerPrivate::setDisableInstancesId(const QList<int>& instanceIds, bool removeDumplicated)
{
    //去重
    if(removeDumplicated) {
        QHash<int, bool> uniqueElements;
        foreach(int element, instanceIds)
        {
            uniqueElements[element] = true;
        }
        disableInstances.clear();
        disableInstances.reserve(uniqueElements.size());
        for (auto it = uniqueElements.begin(); it != uniqueElements.end(); ++it)
        {
            disableInstances.append(it.key());
        }
    } else {
        disableInstances = instanceIds;
    }

    QJsonArray jsonArray;
    for (int value : disableInstances) {
        jsonArray.append(value);
    }
    q->config()->setValue(QStringLiteral("disabledInstances"), jsonArray);
}

void WidgetContainerPrivate::loadConfig(Config* config)
{
    if (config && !configLoaded) {
        auto list = config->children(QStringLiteral("widgets"));
        auto array = config->getValue(QStringLiteral("disabledInstances")).toJsonArray();
        disableInstances.clear();
        for(auto value : array) {
            disableInstances.append(value.toInt());
        }
        for (const auto widgetConfig: list) {
            if(disableInstances.contains(widgetConfig->getValue(QStringLiteral("instanceId")).toInt())) {
                continue;
            }
            q->addWidget(widgetConfig->getValue(QStringLiteral("id")).toString(),
                      widgetConfig->getValue(QStringLiteral("instanceId")).toInt());
        }
        configLoaded = true;
    }
}

void WidgetContainerPrivate::removeDisableInstances()
{
    for (int i = widgets.size() - 1; i >= 0; --i) {
        if (disableInstances.contains(widgets.at(i)->instanceId())) {
            q->removeWidget(widgets.at(i));
        }
    }
}

/*!
    \class UkuiQuick::WidgetContainer
    \inmodule UkuiQuickFramework
    \inheaderfile widget-container.h
    \brief 插件容器.

    \c WidgetContainer 继承自 Widget,在其基础上增加了一些用于插件容器的属性,如屏幕信息,边距等,实现了插件加载和配置文件自动加载功能.
    一般情况下，\c WidgetContainer 用于加载 Widget，但一个 \c WidgetContainer 还可能与 \c Island 有关联（作为 \c island 的主视图）.
*/

/*!
    \brief 默认构造函数，创建一个空的插件容器.
*/
WidgetContainer::WidgetContainer(QObject *parent)
    : WidgetContainer({}, parent) {}

/*!
    \brief 带元数据的构造函数，创建一个指定元数据的插件容器.
*/
WidgetContainer::WidgetContainer(const WidgetMetadata &metaData, QObject *parent)
    : Widget(metaData, parent), d(new WidgetContainerPrivate(this))
{
    d->margin = new Margin(this);
    d->padding = new Margin(this);

    qRegisterMetaType<UkuiQuick::WidgetContainer *>();
}

WidgetContainer::~WidgetContainer()
{
    QVector<Widget*> widgets = d->widgets;
    for(auto w : widgets) {
        removeWidget(w);
    }
    if(d) {
        delete d;
        d = nullptr;
    }
}

/*!
    \brief 获取容器部件的类型标识.

    Returns Types::Container，表示这是一个容器类型的部件.
    \sa Types::WidgetType
 */
Types::WidgetType WidgetContainer::type() const
{
    return Types::Container;
}

/*!
    \property WidgetContainer::appid
    容器所属应用
*/
QString WidgetContainer::appid() const
{
    if (d->island) {
        return d->island->app();
    }

    return {};
}

/*!
    \brief 添加一个插件部件 \a widget到容器中.
*/
void WidgetContainer::addWidget(Widget *widget)
{
    if (!widget || d->widgets.contains(widget) || d->disableInstances.contains(widget->instanceId())) {
        return;
    }

    // 删除重复instanceId的widget
    auto it = std::find_if(d->widgets.constBegin(), d->widgets.constEnd(), [widget] (const Widget *w) {
        return w->instanceId() == widget->instanceId();
    });

    if (it != d->widgets.constEnd()) {
        removeWidget(*it);
    }
    connect(widget, &Widget::activeChanged, this, &Widget::activeChanged);
    connect(widget, &Widget::hostFocusRequested, this, &WidgetContainer::widgetHostFocusRequested);
    d->widgets.append(widget);
    if (config() && !config()->child(QStringLiteral("widgets"), widget->instanceId())) {
        QVariantMap wData = widget->config()? widget->config()->data(): QVariantMap();
        wData.insert(QStringLiteral("id"), widget->id());
        wData.insert(QStringLiteral("instanceId"), widget->instanceId());
        config()->addChild(QStringLiteral("widgets"), wData);
    }

    widget->setContainer(this);
    Q_EMIT widgetAdded(widget);
}

/*!
    \brief 根据插件 \a id 和 \a instanceId 向容器中添加插件部件

    \sa addWidget(Widget *widget)
*/
void WidgetContainer::addWidget(const QString &id, const int instanceId)
{
    if (instanceId < 0 || d->disableInstances.contains(instanceId)) {
        return;
    }

    Widget *widget = WidgetContainer::widgetLoader().loadWidget(id);
    if (widget) {
        widget->setInstanceId(instanceId);
        addWidget(widget);
    }
}

/*!
    \brief 获取全局插件加载器实例

    返回WidgetLoader的单例对象，用于加载和管理插件.
    \sa WidgetLoader::globalLoader()
*/
WidgetLoader &WidgetContainer::widgetLoader()
{
    return WidgetLoader::globalLoader();
}

/*!
    \brief 根据插件 \a id 从容器中移除部件

    \sa removeWidget(int index)
*/
void WidgetContainer::removeWidget(QString id)
{
    for (int i = d->widgets.size() - 1; i >= 0; --i) {
        if (d->widgets.at(i)->id() == id) {
            removeWidget(i);
        }
    }
}

/*!
    \brief 从容器中移除指定的部件 \a widget.

    \sa removeWidget(int index)
*/
void WidgetContainer::removeWidget(Widget *widget)
{
    if (!widget) {
        return;
    }

    removeWidget(d->widgets.indexOf(widget));
}

/*!
    \brief 根据索引从容器中移除部件.

    \sa removeWidget(QString id), removeWidget(Widget *widget)
*/
void WidgetContainer::removeWidget(int index)
{
    if (index < 0 || index >= d->widgets.size()) {
        return;
    }

    Widget *widget = d->widgets.takeAt(index);
    // 通知ContainerItem卸载WidgetItem,Widget即将被删除
    Q_EMIT widgetRemoved(widget);

    // 发送Widget删除信号，这个操作会从全局的缓存中移除widget对应的WidgetItem并delete掉它
    Q_EMIT widget->aboutToDeleted();
    widget->disconnect(this);
    widget->deleteLater();
}

/*!
    \property WidgetContainer::geometry
    容器位置和大小.
*/
QRect WidgetContainer::geometry() const
{
    return d->geometry;
}

void WidgetContainer::setGeometry(QRect geometry)
{
    if (d->geometry == geometry) {
        return;
    }

    d->geometry = geometry;
    Q_EMIT geometryChanged();
}

/*!
    \property WidgetContainer::margin
    容器外边距.
*/
Margin* WidgetContainer::margin() const
{
    return d->margin;
}

/*!
    \property WidgetContainer::padding
    容器内边距.
*/
Margin *WidgetContainer::padding() const
{
    return d->padding;
}

/*!
    \property WidgetContainer::screen
    容器所在屏幕.
*/
QScreen *WidgetContainer::screen() const
{
    return d->screen;
}

void WidgetContainer::setScreen(QScreen *screen)
{
    if(d->screen == screen) {
        return;
    }
    d->screen = screen;
    Q_EMIT screenChanged();
}

/*!
    \property WidgetContainer::radius
    圆角半径.
*/
int WidgetContainer::radius() const
{
    return d->radius;
}

void WidgetContainer::setRadius(int radius)
{
    if (d->radius == radius) {
        return;
    }

    d->radius = radius;
    Q_EMIT radiusChanged();
}

/*!
    \property WidgetContainer::position
    容器位置.

    \sa Types::Pos
*/
Types::Pos WidgetContainer::position() const
{
    return d->position;
}

void WidgetContainer::setPosition(Types::Pos position)
{
    if (d->position == position) {
        return;
    }

    d->position = position;
    Q_EMIT positionChanged();
}

/*!
    \brief 获取容器中的所有部件列表.
*/
QVector<Widget *> WidgetContainer::widgets() const
{
    return d->widgets;
}

/*!
    \brief 根据 \a instanceId 获取指定部件.
*/
Widget* WidgetContainer::widget(int instanceId) const
{
    for(const auto w : d->widgets) {
        if(w->instanceId() == instanceId) {
            return w;
        }
    }
    return nullptr;
}

/*!
    \overload
*/
bool WidgetContainer::active() const
{
    if(d->containActiveWidgets()) {
        return true;
    }
    return Widget::active();
}

/*!
    \deprecated
*/
WidgetMetadata::Host WidgetContainer::host() const
{
    return d->host;
}

/*!
    \deprecated
*/
void WidgetContainer::setHost(WidgetMetadata::Host host)
{
    if(d->host == host) {
        return;
    }
    d->host = host;
    widgetLoader().setShowInFilter(d->host);
    Q_EMIT hostChanged();
}

/*!
    \brief 获取容器支持的类型列表.
 */
WidgetMetadata::Hosts WidgetContainer::hosts() const
{
    return d->hosts;
}

/*!
    \brief  设置容器支持的 \a hosts 类型列表

    \sa WidgetMetadata::Host
 */
void WidgetContainer::setHosts(WidgetMetadata::Hosts hosts)
{
    if(d->hosts == hosts) {
        return;
    }
    d->hosts = hosts;
    widgetLoader().setShowInFilter(d->hosts);
    Q_EMIT hostsChanged();
}

/*!
    \brief 获取关联的Island对象
*/
const Island *WidgetContainer::island() const
{
    return d->island;
}

/*!
    \brief 设置关联的 Island对象 \a island.
*/
void WidgetContainer::setIsland(Island *island)
{
    if (d->island == island) {
        return;
    }

    d->island = island;
    auto config = island->config()->child(UkuiQuick::Island::viewGroupName(), instanceId());
    if (config) {
        setConfig(config);
        d->loadConfig(this->config());
    }
}

void WidgetContainer::setContainer(WidgetContainer* container)
{
    Widget::setContainer(container);
    setScreen(container->screen());
    connect(container, &UkuiQuick::WidgetContainer::screenChanged, this, [&, container](){
        setScreen(container->screen());
    });

    setPosition(container->position());
    connect(container, &UkuiQuick::WidgetContainer::positionChanged, this, [&, container](){
        setPosition(container->position());
    });

    setConfig(container->config()->child(QStringLiteral("widgets"), instanceId()));
    setInstanceId(config()->getValue(QStringLiteral("instanceId")).toInt());

    d->loadConfig(config());
}

void WidgetContainer::setConfig(Config* config)
{
    Widget::setConfig(config);
    if(config) {
        config->addGroupInfo(QStringLiteral("widgets"), QStringLiteral("instanceId"));
    }
}

void WidgetContainer::setDisableInstances(const QList<int>& instanceIds)
{
    d->setDisableInstancesId(instanceIds);
    d->removeDisableInstances();
}

void WidgetContainer::addDisableInstance(int id)
{
    if(d->disableInstances.contains(id)) {
        return;
    }
    d->setDisableInstancesId(d->disableInstances << id, false);
    d->removeDisableInstances();
}

void WidgetContainer::removeDisableInstance(int id)
{
    if(!d->disableInstances.contains(id)) {
        return;
    }
    d->disableInstances.removeAll(id);
    d->setDisableInstancesId(d->disableInstances, false);
}

QList<int> WidgetContainer::disableInstances()
{
    return d->disableInstances;
}
} // UkuiQuick
