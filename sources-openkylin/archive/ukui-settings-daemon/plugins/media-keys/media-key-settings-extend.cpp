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

#include "media-key-settings-extend.h"

MediaKeyExtendSettings::MediaKeyExtendSettings(QString schema, QObject *parent):
    MediaKeyAbstractSettings(schema, parent)
{

    m_pTransLator = new QTranslator;
    initSettings();
}

MediaKeyExtendSettings::~MediaKeyExtendSettings()
{
    clearMapData();
    delete m_pTransLator;
    m_pTransLator = nullptr;
}

void MediaKeyExtendSettings::initSettings()
{
    for (const QString& key : getGsettings()->keys()) {
        if (key.contains("shortcutkey-")) {
            m_shortkeyName = key;
            m_shortKeys = getGsettings()->get(key).toString();
            m_shortkeyTranslator = getGsettings()->get("translator").toString();

            QString temp = m_shortkeyTranslator + QLocale().name() + ".qm";
            USD_LOG_SHOW_PARAMS(temp.toLatin1().data());
            m_pTransLator->load(temp);
            QCoreApplication::installTranslator(m_pTransLator);
            m_shortkeySummary = tr(getGsettings()->getSummary(key).toLatin1().data());
            qDebug()<<m_shortkeySummary;
            m_shortkeyCommand = (getGsettings()->get("command").toString());
            break;
        }
    }

    connect(getGsettings().data(), SIGNAL(changed(const QString&)), this, SLOT(onKeyChanged(const QString&)), Qt::DirectConnection);
}

MediaKeyExtendSettings *MediaKeyExtendSettings::instance(QString schema)
{
    static MediaKeyExtendSettings mediaKeyExtendSettings(schema);
    return &mediaKeyExtendSettings;
}
