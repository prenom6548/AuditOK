/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UKUIWAYLANDSETTINGSMANAGER_H
#define UKUIWAYLANDSETTINGSMANAGER_H

#include <QObject>
#include <QWidget>
#include "abstract-manager.h"
#include "QGSettings/qgsettings.h"
class ukuiWaylandSettingsManager :public AbstractManager
{
    Q_OBJECT
public:
    explicit ukuiWaylandSettingsManager(QObject *parent = nullptr);
    bool start() override;
    void stop() override;

public Q_SLOTS:
    void doGSettingsChanged(const QString &key);

private:
    QGSettings *m_ukuiStyleSettings = nullptr;
    QGSettings *m_gnomeSettings = nullptr;
    QGSettings *m_mouseSettings = nullptr;
    QStringList m_syncMouseSettings = {"cursor-theme","cursor-size"};
};

#endif // UKUIWAYLANDSETTINGSMANAGER_H
