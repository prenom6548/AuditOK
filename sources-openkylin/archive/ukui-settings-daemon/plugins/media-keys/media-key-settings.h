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
#ifndef MEDIAKEYSETTINGS_H
#define MEDIAKEYSETTINGS_H

#include <QMap>
#include <QDebug>
#include <QObject>
#include <QVariant>
#include <QSharedPointer>
#include "QGSettings/qgsettings.h"
#include "media-key-settings-abstract.h"
#define UKUI_DAEMON_NAME "ukui-settings-daemon"

class MediaKeySettings : public MediaKeyAbstractSettings
{
    Q_OBJECT
public:
    explicit MediaKeySettings(QString schema, QObject *parent = nullptr);
    ~MediaKeySettings();
    static MediaKeySettings* instance(QString schema = "");
    void initSettings();

private:
    QString m_schema = "";
};
#endif // MEDIAKEYSETTINGS_H
