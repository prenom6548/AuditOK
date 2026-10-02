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

#include "widget.h"

#include <QPluginLoader>

#include "widget-container.h"
#include "config-loader.h"
#include "config-property-map.h"
#include "widget-interface.h"

namespace UkuiQuick {

class WidgetPrivate
{
public:
    explicit WidgetPrivate(const WidgetMetadata& m);

    int instanceId {-1};

    QString icon;
    QString name;
    QString tooltip;
    QString version;

    QString uiError;

    WidgetContent content;
    WidgetMetadata metaData;

    // actions, 用于显示当前Widget的某些操作
    QList<QAction *> actions;
    //附加action，用于Widget给container的action list或者container给widget的action list附加action
    //例如，widget可以通过widgetInterface插件形式给container的action列表中附加action，container给他的某些字widget附加一个和
    //container相关的action
    QList<QAction *> appendActions;
    bool active {false};
    bool visible {true};

    ConfigPropertyMap *qmlLocalConfig {nullptr};
    ConfigPropertyMap *qmlGlobalConfig {nullptr};

    Config *config {nullptr};
    WidgetContainer *container {nullptr};
    Types::Orientation orientation {Types::Horizontal};

    QScopedPointer<WidgetInterface> plugin;

    double ratio {1.0};
};

WidgetPrivate::WidgetPrivate(const WidgetMetadata &m)
    : metaData(m), content(m),
      icon(m.icon()),
      name(m.name()),
      tooltip(m.tooltip()),
      version(m.version())
{

}

/*!
    \class UkuiQuick::Widget
    \inmodule UkuiQuickFramework
    \inheaderfile widget.h
    \brief  一个 \c "Widget" 组件

    widget包含元数据和内容(\c Content)数据，存放插件的基础数据属性.
    widget通过元数据进行初始化，被加载到某个容器后，容器赋予其更多新属性.
 */

Widget::Widget(const WidgetMetadata& metaData, QObject *parent) : QObject(parent), d(new WidgetPrivate(metaData))
{
    qRegisterMetaType<UkuiQuick::Widget *>();
    qRegisterMetaType<UkuiQuick::Config *>();
    qRegisterMetaType<UkuiQuick::ConfigPropertyMap *>();
    if(d->content.pluginPreload()) {
        loadPlugin();
    }
}

Widget::~Widget()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

Types::WidgetType Widget::type() const
{
    return UkuiQuick::Types::Widget;
}

/*!
    \property 插件元数据的 \c id

    \sa WidgetMetadata::id()
*/
QString Widget::id() const
{
    return d->metaData.id();
}

/*!
    \property Widget::icon
    插件的图标.
 */
QString Widget::icon() const
{
    return d->icon;
}

void Widget::setIcon(const QString &icon)
{
    d->icon = icon;
    Q_EMIT iconChanged();
}

/*!
    \property Widget::name
    插件的名称.
 */
QString Widget::name() const
{
    return d->name;
}

void Widget::setName(const QString &name)
{
    d->name = name;
    Q_EMIT nameChanged();
}

/*!
    \property Widget::tooltip
    插件的提示信息.
 */
QString Widget::tooltip() const
{
    return d->tooltip;
}

void Widget::setTooltip(const QString &tooltip)
{
    d->tooltip = tooltip;
    Q_EMIT tooltipChanged();
}

/*!
    \property Widget::version
    插件的版本.
 */
QString Widget::version() const
{
    return d->version;
}

void Widget::setVersion(const QString &version)
{
    d->version = version;
    Q_EMIT versionChanged();
}

/*!
    \property Widget::website
    插件的地址.

    \sa WidgetMetadata::website()
 */
QString Widget::website() const
{
    return d->metaData.website();
}

/*!
    \property Widget::bugReport
    插件的bug提交地址.

    \sa WidgetMetadata::bugReport()
 */
QString Widget::bugReport() const
{
    return d->metaData.bugReport();
}

/*!
    \property Widget::description
    插件的描述.

    \sa WidgetMetadata::description()
 */
QString Widget::description() const
{
    return d->metaData.description();
}

/*!
    \property Widget::authors
    插件的作者列表.

    \sa WidgetMetadata::authors()
 */
QVariantList Widget::authors() const
{
    return d->metaData.authors();;
}

/*!
    \fn const WidgetMetadata &Widget::metadata() const
    插件的元数据.

    \sa WidgetMetadata
 */
const WidgetMetadata &Widget::metadata() const
{
    return d->metaData;
}

const WidgetContent &Widget::content() const
{
    return d->content;
}

void Widget::setUiError(const QString &error)
{
    d->uiError = error;
}

bool Widget::hasUiError() const
{
    return !d->uiError.isEmpty();
}

QString Widget::uiError() const
{
    return d->uiError;
}

/*!
    \property Widget::actions
    获取插件的所有 \l QAction 列表.
 */
QList<QAction *> Widget::actions() const
{
    return d->actions + d->appendActions;
}

void Widget::setActions(const QList<QAction *>& actions)
{
    d->actions = actions;
}

void Widget::setAppendActions(QList<QAction*> actions)
{
    d->appendActions = std::move(actions);
}

QQmlListProperty<QAction> Widget::qmlActions()
{
    return {this, &d->actions};
}

/*!
    \property Widget::instanceId
    插件的实例 \c id,在某个应用中是唯一的，用于确认 \c widget 身份.
 */
int Widget::instanceId() const
{
    return d->instanceId;
}

void Widget::setInstanceId(const int instanceId)
{
    if (instanceId == d->instanceId) {
        return;
    }

    d->instanceId = instanceId;
}

/*!
    \property Widget::active
    该属性表示插件激活状态， 将 \c Widget 置为激活状态，应用可能会根据这个状态设置一些UI交互，比如任务栏不会自动隐藏.
 */
bool Widget::active() const
{
    return d->active;
}

void Widget::setActive(bool active)
{
    if (d->active == active) {
        return;
    }

    d->active = active;
    Q_EMIT activeChanged();
}

void Widget::requestHostFocus()
{
    Q_EMIT hostFocusRequested(this);
}

/*!
    \property Widget::orientation
    该属性表示插件的布局方向
 */
Types::Orientation Widget::orientation() const
{
    return d->orientation;
}

void Widget::setOrientation(Types::Orientation o)
{
    if (d->orientation == o) {
        return;
    }

    d->orientation = o;
    Q_EMIT orientationChanged();
}

/*!
    \property Widget::config
    插件的局部配置接口,该配置仅在当前Container内有效.
 */
ConfigPropertyMap *Widget::qmlLocalConfig() const
{
    if (!d->qmlLocalConfig) {
        Config *config = Widget::config();
        if (config) {
            d->qmlLocalConfig = new ConfigPropertyMap(config, (QObject *) this);
        }
    }

    return d->qmlLocalConfig;
}

/*!
    \property Widget::config
    Widget的独立配置接口,可由ConfigPolicy控制,可在多个Container间共享,该配置仅在当前Container内有效.
*/
ConfigPropertyMap *Widget::qmlGlobalConfig() const
{
    if (content().configPolicy() == WidgetContent::LocalOnly) {
        return qmlLocalConfig();
    }

    if (!d->qmlGlobalConfig) {
        Config *config = globalConfig();
        if (config) {
            d->qmlGlobalConfig = new ConfigPropertyMap(config, (QObject *) this);
        }
    }

    return d->qmlGlobalConfig;
}

Config *Widget::config() const
{
    if (!d->config) {
        if (d->container) {
            d->config = d->container->config()->child(QStringLiteral("widgets"), d->instanceId);
        }
    }

    return d->config;
}

void Widget::setConfig(Config *config)
{
    if (d->config == config) {
        return;
    }

    d->config = config;
}

Config *Widget::globalConfig() const
{
    WidgetContent::ConfigPolicy policy = content().configPolicy();
    if (policy == WidgetContent::LocalOnly) {
        return config();
    }

    return ConfigLoader::getConfig(d->metaData.id(), ConfigLoader::Global);
}

/*!
    \property Widget::container
    该属性表示插件所属的容器对象.
 */
WidgetContainer *Widget::container() const
{
    return d->container;
}

/*!
    \property Widget::showIn
    插件的显示位置.

    \sa WidgetMetadata::showIn()
 */
WidgetMetadata::Hosts Widget::showIn() const
{
    return d->metaData.showIn();
}

void Widget::setContainer(WidgetContainer *container)
{
    WidgetContainer *old = d->container;
    if (old) {
        old->disconnect(this, nullptr);
    }

    d->container = container;
    Q_EMIT containerChanged(old ? old->id() : "", d->container ? d->container->id() : "");

    if (d->container) {
        setOrientation(d->container->orientation());
        connect(d->container, &WidgetContainer::orientationChanged, this, [this] {
            setOrientation(d->container->orientation());
        });
    }
}

/*!
    \property Widget::ratio
    该属性表示插件的缩放系数，一般在 \c container 里控制，\c widget 根据需要读取，例如根据缩放系数进行布局调整等.
 */
double Widget::ratio() const
{
    return d->ratio;
}

void Widget::setRatio(double ratio)
{
    if(d->ratio != ratio) {
        d->ratio = ratio;
        Q_EMIT ratioChanged();
    }
}

void Widget::loadPlugin() const
{
    if(d->plugin.isNull() && !d->content.plugin().isNull()) {
        QPluginLoader pluginLoader(d->content.plugin());
        if(d->content.pluginVersion() == WIDGET_INTERFACE_VERSION) {
            d->plugin.reset(dynamic_cast<WidgetInterface*>(pluginLoader.instance()));
            if(d->plugin.isNull()) {
                qWarning() << "Fail to load widget plugin:" << id() << d->content.plugin();
            }
        } else {
            qWarning() << "Plugin version unmatched:" << id() << d->content.plugin();
        }
    }
}

/*!
    \property Widget::visible
    插件的可见状态.
 */
bool Widget::visible() const
{
    return d->visible;
}

void Widget::setVisible(bool visible)
{
    if (d->visible == visible) {
        return;
    }

    d->visible = visible;
    Q_EMIT visibleChanged();
}

/*!
    \property Widget::plugin
    该属性表示 \c widget 中加载的以动态库形式安装的插件.

    当插件实现了 WidgetInterface 时, 它既可以通过 actions() 提供扩展动作,
    也可以通过 featureProviders() 向 WidgetBridge 暴露领域能力对象.
 */
WidgetInterface* Widget::plugin() const
{
    loadPlugin();
    return d->plugin.data();
}
} // UkuiQuick
