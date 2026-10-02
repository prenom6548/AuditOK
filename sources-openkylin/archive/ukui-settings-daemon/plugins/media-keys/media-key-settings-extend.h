/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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


#ifndef MEDIAKEYEXTENDSETTINGS_H
#define MEDIAKEYEXTENDSETTINGS_H

#include "media-key-settings-abstract.h"

class MediaKeyExtendSettings: public MediaKeyAbstractSettings
{
    Q_OBJECT
public:
    MediaKeyExtendSettings(QString schema, QObject *parent = nullptr);
    ~MediaKeyExtendSettings();
    MediaKeyExtendSettings *instance(QString schema);
    void initSettings();

    QString getShortkeyName() {return m_shortkeyName;}
    QString getShortkeys() {return m_shortKeys;}
    QString getShortkeyTranslator() {return m_shortkeyTranslator;}
    QString getShortkeyCommand() {return m_shortkeyCommand;}
    QString getShortkeySummary(){return m_shortkeySummary;}
private:
    MediaKeyExtendSettings& operator =(const MediaKeyExtendSettings&)=delete;

private:
    QString m_shortkeyName = "";
    QString m_shortKeys = "";
    QString m_shortkeyTranslator = "";
    QString m_shortkeyCommand = "";
    QString m_shortkeySummary = "";
    QString m_schema = "";
    QTranslator *m_pTransLator = nullptr;
};

#endif // MEDIAKEYEXTENDSETTINGS_H
