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

#ifndef UKUI_QUICK_WIDGET_BRIDGE_H
#define UKUI_QUICK_WIDGET_BRIDGE_H

#include <QObject>
#include <QScopedPointer>
#include <qqml.h>

namespace UkuiQuick {

class Widget;
class WidgetBridgePrivate;

class WidgetBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(UkuiQuick::Widget *widget READ widget CONSTANT FINAL)
    Q_PROPERTY(QObject *pluginObject READ pluginObject FINAL)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged FINAL)
    Q_PROPERTY(QStringList availableFeatures READ availableFeatures NOTIFY availableFeaturesChanged FINAL)
public:
    explicit WidgetBridge(QObject *owner = nullptr);
    ~WidgetBridge() override;

    static WidgetBridge *qmlAttachedProperties(QObject *object);

    static void setFeatureProvider(UkuiQuick::Widget *widget, QObject *provider);
    static void clearFeatureProvider(const UkuiQuick::Widget *widget);

    Widget *widget() const;
    QObject *pluginObject() const;
    QString lastError() const;
    QStringList availableFeatures() const;

    Q_INVOKABLE QObject *feature(const QString &key);
    Q_INVOKABLE bool hasFeature(const QString &key) const;
    void notifyProviderChanged();

Q_SIGNALS:
    void lastErrorChanged();
    void availableFeaturesChanged();

private:
    void updateError(const QString &error);

private:
    Q_DECLARE_PRIVATE(WidgetBridge)
    friend class WidgetBridgePrivate;
    QScopedPointer<WidgetBridgePrivate> d_ptr;
};

} // namespace UkuiQuick

QML_DECLARE_TYPEINFO(UkuiQuick::WidgetBridge, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUI_QUICK_WIDGET_BRIDGE_H
