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

#ifndef AUTOTESTS_SRC_STUB_SIDEBARADAPTOR_H
#define AUTOTESTS_SRC_STUB_SIDEBARADAPTOR_H

#include <QObject>
#include <QVariantMap>

class SidebarAdaptor : public QObject
{
    Q_OBJECT
public:
    explicit SidebarAdaptor(QObject *parent = nullptr) : QObject(parent) {}

public Q_SLOTS:
    void stateChange(const QString &module, const QVariantMap &data)
    {
        m_lastModule = module;
        m_lastData = data;
    }

    QString lastModule() const
    {
        return m_lastModule;
    }

    QVariantMap lastData() const
    {
        return m_lastData;
    }

private:
    QString m_lastModule;
    QVariantMap m_lastData;
};

#endif
