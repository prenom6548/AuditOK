/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#ifndef UKUI_QUICK_WIDGET_FEATURE_PROVIDER_H
#define UKUI_QUICK_WIDGET_FEATURE_PROVIDER_H

#include <QObject>
#include <QStringList>

namespace UkuiQuick {

/*!
    \class UkuiQuick::WidgetFeatureProvider
    \inmodule UkuiQuickFramework
    \inheaderfile widget-feature-provider.h
    \brief 为 WidgetBridge 提供领域能力对象的 provider 接口.

    WidgetFeatureProvider 用于按 feature key 暴露 \c QObject 功能对象。
    一个 provider 可以暴露一个或多个 feature，返回对象的生命周期由 provider 自身负责。

    该接口通常由插件动态库中的辅助对象或宿主应用中的桥接对象实现。
    它们分别通过 WidgetInterface::featureProviders() 与 WidgetBridge::setFeatureProvider()
    暴露给 WidgetBridge。

    \sa UkuiQuick::WidgetBridge, UkuiQuick::WidgetInterface
 */
class WidgetFeatureProvider
{
public:
    virtual ~WidgetFeatureProvider() = default;
    /*!
        \fn virtual QObject *WidgetFeatureProvider::feature(const QString &key)

        返回 \a key 对应的 feature 对象。如果当前 provider 不支持该 key, 应返回 \c nullptr.
    */
    virtual bool hasFeature(const QString &key) const = 0;

    /*!
    \fn virtual QObject *WidgetFeatureProvider::feature(const QString &key)

    返回 \a key 对应的 feature 对象。如果当前 provider 不支持该 key, 应返回 \c nullptr.
    */
    virtual QObject *feature(const QString &key) = 0;
    /*!
    \fn virtual QStringList WidgetFeatureProvider::availableFeatures() const

    返回当前 provider 对外暴露的全部 feature key 列表.
    */
    virtual QStringList availableFeatures() const = 0;
};
} // namespace UkuiQuick

#endif // UKUI_QUICK_WIDGET_FEATURE_PROVIDER_H
