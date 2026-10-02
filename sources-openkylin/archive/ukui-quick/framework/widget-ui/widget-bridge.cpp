/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include "widget-bridge.h"

#include <QHash>
#include <QPointer>
#include <QQmlContext>
#include <QQmlEngine>

#include "widget-feature-provider.h"
#include "widget-item-context.h"
#include "widget-item.h"
#include "widget.h"

namespace UkuiQuick {
class WidgetBridgePrivate
{
public:
    explicit WidgetBridgePrivate(QObject *owner)
        : owner(owner)
    {
    }

    QObject *owner {nullptr};
    QString lastError;
    mutable QPointer<Widget> cachedWidget;
    mutable bool providersDirty {true};
    mutable QList<QPointer<QObject>> cachedProviderObjects;
    mutable QStringList cachedAvailableFeatures;
};

namespace {

QHash<const Widget *, QList<QPointer<QObject>>> &hostFeatureProviders()
{
    static QHash<const Widget *, QList<QPointer<QObject>>> providers;
    return providers;
}

QHash<const Widget *, QList<QPointer<WidgetBridge>>> &bridgeInstances()
{
    static QHash<const Widget *, QList<QPointer<WidgetBridge>>> instances;
    return instances;
}

Widget *resolveWidget(QObject *object)
{
    if (!object) {
        return nullptr;
    }

    if (auto *widgetItem = qobject_cast<WidgetItem *>(object)) {
        return widgetItem->widget();
    }

    QQmlContext *context = QQmlEngine::contextForObject(object);
    while (context) {
        context = context->parentContext();
        if (auto *widgetContext = qobject_cast<WidgetItemContext *>(context)) {
            return widgetContext->widget();
        }
    }

    return nullptr;
}

QList<WidgetFeatureProvider *> resolveProviders(const Widget *widget)
{
    QList<WidgetFeatureProvider *> providers;
    if (!widget) {
        return providers;
    }

    const auto pluginProviders = widget->plugin() ? widget->plugin()->featureProviders() : QList<QObject *> {};
    for (auto it = pluginProviders.crbegin(); it != pluginProviders.crend(); ++it) {
        if (auto *provider = dynamic_cast<WidgetFeatureProvider *>(*it)) {
            providers.append(provider);
        }
    }

    auto &hostProviders = hostFeatureProviders();
    auto hostIt = hostProviders.find(widget);
    if (hostIt != hostProviders.end()) {
        QList<QPointer<QObject>> alive;
        alive.reserve(hostIt->size());
        for (auto it = hostIt->crbegin(); it != hostIt->crend(); ++it) {
            if (!*it) {
                continue;
            }

            alive.prepend(*it);
            if (auto *provider = dynamic_cast<WidgetFeatureProvider *>((*it).data())) {
                providers.append(provider);
            }
        }

        if (alive.isEmpty()) {
            hostProviders.erase(hostIt);
        } else {
            *hostIt = alive;
        }
    }

    return providers;
}

QStringList collectAvailableFeatures(const QList<QPointer<QObject>> &providerObjects)
{
    QStringList features;
    for (const auto &providerObject : providerObjects) {
        auto *provider = dynamic_cast<WidgetFeatureProvider *>(providerObject.data());
        if (!provider) {
            continue;
        }

        const auto providerFeatures = provider->availableFeatures();
        for (const auto &featureKey : providerFeatures) {
            if (!features.contains(featureKey)) {
                features.append(featureKey);
            }
        }
    }

    return features;
}

void notifyBridges(const Widget *widget)
{
    auto &instances = bridgeInstances();
    auto it = instances.find(widget);
    if (it == instances.end()) {
        return;
    }

    QList<QPointer<WidgetBridge>> alive;
    alive.reserve(it->size());
    for (const auto &bridge : std::as_const(*it)) {
        if (!bridge) {
            continue;
        }
        bridge->notifyProviderChanged();
        alive.append(bridge);
    }

    if (alive.isEmpty()) {
        instances.erase(it);
        return;
    }

    *it = alive;
}

} // namespace

/*!
    \class UkuiQuick::WidgetBridge
    \inmodule UkuiQuickFramework
    \inheaderfile widget-bridge.h
    \brief widget 与 feature provider 之间的桥接对象.

    WidgetBridge 用于聚合当前 widget 关联的全部 WidgetFeatureProvider,
    并以统一的 feature key 查询接口向 C++ 和 QML 暴露领域能力对象。

    provider 可以来自插件动态库, 也可以来自宿主应用。发生 key 冲突时:
    \list
    \li 插件侧 provider 的优先级高于宿主侧 provider
    \li 同一来源内部, 后注册或后返回的 provider 优先级更高
    \endlist

    \sa UkuiQuick::WidgetFeatureProvider, UkuiQuick::WidgetInterface
 */

/*!
    \qmltype WidgetBridge
    \inqmlmodule UkuiQuickFramework
    \brief WidgetItem 的附加功能桥接对象.

    WidgetBridge 作为附加属性暴露在每个 \c WidgetItem 上, 用于在 QML 中访问当前 widget 的 feature object。

    常见用法如下:
    \qml
    import UkuiQuickFramework 1.0

    WidgetItem {
        readonly property var searchState: WidgetBridge.feature("org.ukui.search.state")
    }
    \endqml
 */

WidgetBridge::WidgetBridge(QObject *owner)
    : QObject(owner), d_ptr(new WidgetBridgePrivate(owner))
{
    if (auto *bridgeWidget = widget()) {
        bridgeInstances()[bridgeWidget].append(this);
    }
}

WidgetBridge::~WidgetBridge()
{
    const auto *bridgeWidget = widget();
    if (!bridgeWidget) {
        return;
    }

    auto it = bridgeInstances().find(bridgeWidget);
    if (it == bridgeInstances().end()) {
        return;
    }

    it->removeAll(this);
    if (it->isEmpty()) {
        bridgeInstances().erase(it);
    }
}

WidgetBridge *WidgetBridge::qmlAttachedProperties(QObject *object)
{
    return new WidgetBridge(object);
}

/*!
    \fn static void WidgetBridge::setFeatureProvider(UkuiQuick::Widget *widget, QObject *provider)

    为 \a widget 注册一个宿主侧 provider.
    如果 \a provider 实现了 WidgetFeatureProvider 接口, 其 feature 将通过 WidgetBridge 对外可见。
 */
void WidgetBridge::setFeatureProvider(Widget *widget, QObject *provider)
{
    if (!widget || !provider) {
        return;
    }

    hostFeatureProviders()[widget].append(provider);
    connect(widget, &QObject::destroyed, [] (QObject *obj) {
        const auto *bridgeWidget = static_cast<const Widget *>(obj);
        hostFeatureProviders().remove(bridgeWidget);
        bridgeInstances().remove(bridgeWidget);
    });
    connect(provider, &QObject::destroyed, widget, [widget] {
        notifyBridges(widget);
    });
    notifyBridges(widget);
}

/*!
    \fn static void WidgetBridge::clearFeatureProvider(const UkuiQuick::Widget *widget)

    清空 \a widget 上由宿主侧注册的 provider 列表。
 */
void WidgetBridge::clearFeatureProvider(const Widget *widget)
{
    if (!widget) {
        return;
    }

    hostFeatureProviders().remove(widget);
    notifyBridges(widget);
}

/*!
    \property WidgetBridge::widget
    当前附加属性所关联的 widget 对象.
 */
Widget *WidgetBridge::widget() const
{
    Q_D(const WidgetBridge);
    if (d->cachedWidget) {
        return d->cachedWidget;
    }

    d->cachedWidget = resolveWidget(d->owner);
    return d->cachedWidget;
}

/*!
    \property WidgetBridge::pluginObject
    当前 widget 加载的插件动态库对象。

    该属性仅用于直接访问插件对象本身, 更推荐通过 feature() 获取稳定的 feature object.
 */
QObject *WidgetBridge::pluginObject() const
{
    auto *bridgeWidget = widget();
    if (!bridgeWidget) {
        return nullptr;
    }

    return dynamic_cast<QObject *>(bridgeWidget->plugin());
}

/*!
    \property WidgetBridge::lastError
    最近一次 feature 解析失败时的错误信息。

    成功解析 feature 或 provider 集合发生变化时, 该属性会被清空.
 */
QString WidgetBridge::lastError() const
{
    Q_D(const WidgetBridge);
    return d->lastError;
}

/*!
    \property WidgetBridge::availableFeatures
    当前 widget 可用的 feature key 列表.

    该列表会聚合插件侧与宿主侧 provider 的结果, 并按最终生效顺序去重。
 */

QStringList WidgetBridge::availableFeatures() const
{
    Q_D(const WidgetBridge);
    if (d->providersDirty) {
        d->cachedProviderObjects.clear();
        const auto providers = resolveProviders(widget());
        d->cachedProviderObjects.reserve(providers.size());
        for (auto *provider : providers) {
            d->cachedProviderObjects.append(dynamic_cast<QObject *>(provider));
        }
        d->cachedAvailableFeatures = collectAvailableFeatures(d->cachedProviderObjects);
        d->providersDirty = false;
    }

    return d->cachedAvailableFeatures;
}

/*!
    \fn QObject *WidgetBridge::feature(const QString &key)

    返回 \a key 对应的 feature 对象。
    如果当前 widget 没有可用 provider 或没有 provider 支持该 key, 则返回 \c nullptr,
    并更新 lastError 属性.
 */
QObject *WidgetBridge::feature(const QString &key)
{
    Q_D(WidgetBridge);
    if (d->providersDirty) {
        availableFeatures();
    }

    if (d->cachedProviderObjects.isEmpty()) {
        updateError(tr("No feature provider registered for this widget."));
        return nullptr;
    }

    for (const auto &providerObject : std::as_const(d->cachedProviderObjects)) {
        auto *provider = dynamic_cast<WidgetFeatureProvider *>(providerObject.data());
        if (!provider) {
            continue;
        }
        if (!provider->hasFeature(key)) {
            continue;
        }

        auto *resolvedFeature = provider->feature(key);
        if (!resolvedFeature) {
            continue;
        }

        updateError({});
        return resolvedFeature;
    }

    updateError(tr("Feature is unavailable: %1").arg(key));
    return nullptr;
}

/*!
    \fn bool WidgetBridge::hasFeature(const QString &key) const

    返回当前 widget 是否存在支持 \a key 的 provider.
 */
bool WidgetBridge::hasFeature(const QString &key) const
{
    Q_D(const WidgetBridge);
    if (d->providersDirty) {
        availableFeatures();
    }

    for (const auto &providerObject : std::as_const(d->cachedProviderObjects)) {
        auto *provider = dynamic_cast<WidgetFeatureProvider *>(providerObject.data());
        if (!provider) {
            continue;
        }
        if (provider->hasFeature(key)) {
            return true;
        }
    }

    return false;
}

void WidgetBridge::updateError(const QString &error)
{
    Q_D(WidgetBridge);
    if (d->lastError == error) {
        return;
    }

    d->lastError = error;
    Q_EMIT lastErrorChanged();
}

void WidgetBridge::notifyProviderChanged()
{
    Q_D(WidgetBridge);
    d->providersDirty = true;
    d->cachedProviderObjects.clear();
    d->cachedAvailableFeatures.clear();
    updateError({});
    Q_EMIT availableFeaturesChanged();
}

} // namespace UkuiQuick
