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
#ifndef SOUND_H
#define SOUND_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusReply>

class Sound : public QObject
{
    Q_OBJECT
public:
    enum DeviceType {
        Input,
        Output,
        Sink_Input,
        Source_Output
    };

    explicit Sound(QObject *parent = nullptr);
    ~Sound();

public:
    static Sound* self();

public:
    int getSinkVolume();
    bool getSinkMute();
    bool getSourceMute();

    void setSinkVolume(int volume);
    void setSinkMute(bool mute);
    void setSourceMute(bool mute);

    bool initSinkSuccess();
    bool initSourceSuccess();

    bool isExistSink();
    bool isExistSource();

private:
    void initVolumeDbus();
    bool isDbusValid() const;

    template <typename...Args>
    QVariant getValueByDbus(const QString &method, Args &&...args);

    template <typename...Args>
    void setValueByDbus(const QString &method, Args &&...args);

private:
    QDBusInterface* m_volumeInterface = nullptr;
};

#endif // SOUND_H
