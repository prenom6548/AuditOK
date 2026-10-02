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
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */

#include "config-property-map.h"
#include <QPointer>
#include <QDebug>

using namespace UkuiQuick;

class UkuiQuick::ConfigPropertyMapPrivate
{
public:
    explicit ConfigPropertyMapPrivate(ConfigPropertyMap *map) : q(map) {};
    void loadConfig() const;

    ConfigPropertyMap *q;
    QPointer<Config> config;
};

void ConfigPropertyMapPrivate::loadConfig() const
{
    if (!config) {
        return;
    }

    for (const auto& key :  config->keys()) {
        q->insert(key, config->getValue(key));
    }
}
/*!
    \class UkuiQuick::ConfigPropertyMap
    \inmodule UkuiQuickFramework
    \inheaderfile config-property-map.h
    \brief 提供配置属性映射功能，用于将配置数据与QML属性进行绑定.

 */
ConfigPropertyMap::ConfigPropertyMap(Config *config, QObject *parent) : QQmlPropertyMap(this, parent), d(new ConfigPropertyMapPrivate(this))
{
    Q_ASSERT(config);
    d->config = config;
    d->loadConfig();

    connect(d->config, &Config::configChanged, this, [this](const QString& key) {
        insert(key, d->config->getValue(key));
    });
    connect(this, &ConfigPropertyMap::valueChanged, this, [this] (const QString &key, const QVariant &value) {
        d->config->setValue(key, value);
    });
}

ConfigPropertyMap::~ConfigPropertyMap() = default;

QVariant ConfigPropertyMap::updateValue(const QString &key, const QVariant &input)
{
    d->config->setValue(key, input);
    d->config->forceSync();
    return input;
}

#include "moc_config-property-map.cpp"
