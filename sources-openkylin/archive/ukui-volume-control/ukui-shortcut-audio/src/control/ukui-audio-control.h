/*
* Copyright (C) 2024, KylinSoft Co., Ltd.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
* Authors: zhouyong <zhouyong01@kylinos.cn>
*
*/
#ifndef UKUIAUDIOCONTROL_H
#define UKUIAUDIOCONTROL_H

#include <QObject>
#include "DBusClient.h"

#include <QMutex>
#include <QProcess>
#include <QDebug>

class UKUIAudioControl : public QObject
{
    Q_OBJECT
public:
    static UKUIAudioControl& getInstance();

    bool initData();
    bool initSlots();
    bool initDbusConnect();
    bool jumpControlPanelAudio();
    bool jumpAppControlPanelAudio();

public:
    Q_INVOKABLE void initMainThreadEvent();
    Q_INVOKABLE void startTask(const int taskID, QVariant var1 = QVariant()
            , QVariant var2 = QVariant(), QVariant var3 = QVariant());

    Q_INVOKABLE void setLoadedStatus() {
        m_isLoaded = true;
        qDebug() << "getLoadStatus m_isLoaded:" << m_isLoaded;
    }
    Q_INVOKABLE bool getLoadStatus() {
        qDebug() << "getLoadStatus m_isLoaded:" << m_isLoaded;
        return m_isLoaded;
    }

private Q_SLOTS:

signals:
    void taskCompleted(int taskID, QVariant var1 = QVariant());
    void taskError(int taskID);

public:
    Q_INVOKABLE void releaseData();
    Q_INVOKABLE void releaseSlots();

private:
    UKUIAudioControl() = default;
    UKUIAudioControl(const UKUIAudioControl&) = delete;
    UKUIAudioControl(UKUIAudioControl&&) = delete;
    UKUIAudioControl operator=(const UKUIAudioControl&) = delete;
    UKUIAudioControl operator=(UKUIAudioControl&&) = delete;
    ~UKUIAudioControl();

private:
    QMutex m_taskMutex;
    bool m_isLoaded = false;

};

#endif // UKUIAUDIOCONTROL_H
