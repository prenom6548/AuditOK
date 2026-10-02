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
 *
 */

#ifndef TEST_GLOBAL_SETTINGS_H
#define TEST_GLOBAL_SETTINGS_H

#include <QObject>
#include <QVariant>
#include <QMap>

#define TABLET_MODE "tablet-mode"

namespace Sidebar {

class GlobalSettings : public QObject
{
    Q_OBJECT
public:
    static GlobalSettings *globalInstance(QObject *parent = nullptr);
    QVariant getValue(const QString &key) const;
    void setValue(const QString &key, const QVariant &value);

Q_SIGNALS:
    void valueChanged(const QString &key);

private:
    explicit GlobalSettings(QObject *parent = nullptr);
    QMap<QString, QVariant> m_values;
};

} // namespace Sidebar

#endif
