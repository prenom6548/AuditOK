/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2022 KylinSoft Co., Ltd.
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
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */
#ifndef PROCESSSETTINGS_H
#define PROCESSSETTINGS_H

#include <QObject>
#include <QTimer>

class ProcessSettings : public QObject
{
    Q_OBJECT
public:
    static ProcessSettings* instance();
    void setLocatePointer(bool state);
    void setDisableWTypingSynaptics(bool state);
private:
    explicit ProcessSettings(QObject *parent = nullptr);
    ProcessSettings(const ProcessSettings&) = delete;
    ProcessSettings& operator =(const ProcessSettings&)=delete;
private:
    bool m_locatePointerRunning = false;
    bool m_syndaemonRunning = false;
    bool isProcessRunning(const QString &processName);
};

#endif // PROCESSSETTINGS_H
