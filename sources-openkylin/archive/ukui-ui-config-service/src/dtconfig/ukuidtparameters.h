/*
 * Copyright © 2016-2018 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Jan Grulich <jgrulich@redhat.com>
 * Modifier:
 *       Yue Lan <lanyue@kylinos.cn>
 */

#ifndef UKUI_DT_PARAMETERS_H
#define UKUI_DT_PARAMETERS_H

#include <QDBusObjectPath>
#include <QDBusAbstractAdaptor>

#include "dtreadconfig.h"

class DTParameters : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ThemeConfig.DTParameters")
public:

    explicit DTParameters(QObject *parent);
    ~DTParameters();

public Q_SLOTS:
    QVariantMap GetDTThemeConfig(const QString key);

private:
    UKUIThemeConfigDaemon::GlobalDTConfig *m_dtconfig;
};

#endif // UKUI_DT_PARAMETERS_H
