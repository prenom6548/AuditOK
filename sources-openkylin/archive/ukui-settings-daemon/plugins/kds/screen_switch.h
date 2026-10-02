/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef SCREENSWITCH_H
#define SCREENSWITCH_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusReply>

class ScreenSwitch : public QObject
{
    Q_OBJECT
public:
    explicit ScreenSwitch(QObject *parent = nullptr);
    static ScreenSwitch* instance();
    QStringList getScreenList();
    int getScreenMode();
    void setScreenMode(const QString& mode);
Q_SIGNALS:
    void sigScreenParamChanged(QString);
private:
    QDBusInterface* m_xrandrInterface = nullptr;
};

#endif // SCREENSWITCH_H
