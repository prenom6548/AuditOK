/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef LEAVELOCK_H
#define LEAVELOCK_H

#include <QObject>
#include <QProcess>
//#include <ukui-log4qt.h>
#include <QDebug>
#include "CSingleton.h"
#include <QTimer>

class LeaveLock : public QObject
{
    Q_OBJECT
public:
    explicit LeaveLock(QObject *parent = nullptr);
    ~LeaveLock();
    bool setPing(QString uuid, QString dev, bool on);

private:
    QString m_uid;
    QString m_lockdev;
    QProcess * m_process = nullptr;
    bool m_manualCancel = false;


private slots:
    void readOutput();
    void pingFinished();
    friend class SingleTon<LeaveLock>;
};

typedef SingleTon<LeaveLock>  LEAVELOCK;

#endif // LEAVELOCK_H
