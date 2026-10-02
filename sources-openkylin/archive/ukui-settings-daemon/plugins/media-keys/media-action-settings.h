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
#ifndef MEDIAACTIONSETTINGS_H
#define MEDIAACTIONSETTINGS_H

#include <QObject>
#include <QSharedPointer>
#include <QGSettings/qgsettings.h>
#include <QMap>
#include <QVariant>
#include "usd_base_class.h"
#include "media-type.h"


class MediaActionSettings : public QObject
{
    Q_OBJECT

public:
    explicit MediaActionSettings(QObject *parent = nullptr);

    static MediaActionSettings* instance();
    void initSettings();

    QVariant getPowerKeyState() const;
    int getBrightnessValue() const;
    int updateBrightnessWithAction(ActionType action);
    bool getCanSetBrightness();

    QVariant getTouchpadState() const;
    bool getSessionState();
    void setBrightnessValue(uint value);
    void setTouchpadState(bool state);
    void setLocatePointer();

    QVariant getPowerSettings(const QString& key);
    void setPowerSettings(const QString& key, const QVariant& value);

    int getMaxBrightness() const;
private:
    QGSettings* initSettings(const QByteArray& schema, const QString& key);
    QVariant getValue(const QString& key, const QVariant& value);
    QVariant getValue(const QString& key);
    void initCanSetBrightness();

    bool isEnhancementBrightness() const;

    int getMinBrightness() const;

    void rewriteBrightnessNode() const;
    bool isBrightnessOverridden(ActionType action) const;
private:
    QMap<QString, QVariant> m_settingsData;
    QGSettings *m_touchpadSettings = nullptr;
    QGSettings *m_mouseSettings = nullptr;
    QGSettings *m_powerSettings = nullptr;
    QGSettings *m_sessionSettings = nullptr;
    QGSettings *m_shotSettings = nullptr;
    const int C_STEP_BRIGHTNESS = 5;

    bool m_isEnhancementBrightness = false;
};

#endif // MEDIAACTIONSETTINGS_H
