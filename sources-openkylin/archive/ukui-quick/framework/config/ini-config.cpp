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

#include "ini-config.h"

#include <QSettings>
#include <QDebug>

namespace UkuiQuick {

class IniConfigPrivate
{
public:
    explicit IniConfigPrivate(const QString &file, IniConfig *q);
    QSettings *settings {nullptr};
};

IniConfigPrivate::IniConfigPrivate(const QString &file, IniConfig *q)
{
    settings = new QSettings(file, QSettings::IniFormat, q);
}

// ===== IniConfig ===== //


/*!
    \class UkuiQuick::IniConfig
    \inmodule UkuiQuickFramework
    \inheaderfile ini-config.h

    \brief  基于 INI 格式文件的配置实现.
*/
IniConfig::IniConfig(const QString &filename, QObject *parent) : ConfigIFace(parent), d(new IniConfigPrivate(filename, this))
{

}

/*!
    \brief 根据 \a key 获取指定键的值.
*/
QVariant IniConfig::getValue(const QString &key) const
{
    if (d->settings) {
        return d->settings->value(key);
    }

    return {};
}

/*!
    \brief 设置指定键 \a key 的值 \a value.
*/
void IniConfig::setValue(const QString &key, const QVariant &value)
{
    if (d->settings) {
        d->settings->setValue(key, value);
        Q_EMIT configChanged(key);
    }
}

/*!
    \brief 调用该函数会立即将内存中的配置同步到文件中.
*/
void IniConfig::forceSync()
{
    if (d->settings) {
        d->settings->sync();
    }
}

/*!
    \brief 获取内部的 \c QSettings 对象.
*/
QSettings *IniConfig::settings() const
{
    return d->settings;
}

/*!
    \brief 获取所有配置键名.
*/
QStringList IniConfig::keys() const
{
    if (d->settings) {
        return d->settings->allKeys();
    }

    return {};
}


} // UkuiQuick
