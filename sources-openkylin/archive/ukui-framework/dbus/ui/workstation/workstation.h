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

#ifndef UKUI_FRAMEWORK_WORKSTATION_H
#define UKUI_FRAMEWORK_WORKSTATION_H

#include <QObject>
#include <QDBusContext>
#include "qgsettings.h"

class ConfigWorkstation: public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.UI.Workstation")

    Q_PROPERTY(QStringList defaultApps READ getDefaultApps WRITE setDefaultApps)

public:

    explicit ConfigWorkstation(QObject *parent = nullptr);
    ~ConfigWorkstation();
public Q_SLOTS:
    /**
     * @brief getDefaultApps
     * @return
     */
    QStringList getDefaultApps();

    /**
     * @brief setDefaultApps
     * @return
     */
    bool setDefaultApps(const QStringList& defaultApps) const;


private:
    bool getConfigApps();
    UKUI::QGSettings *m_gSettings = nullptr;
    QStringList m_defaultApps;
};


#endif //UKUI_FRAMEWORK_WORKSTATION_H