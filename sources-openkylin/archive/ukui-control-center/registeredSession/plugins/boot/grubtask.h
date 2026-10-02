/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef GRUBTASK_H
#define GRUBTASK_H

#include <QObject>
#include <QRunnable>
#include <QDBusInterface>
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <QDBusReply>

enum TaskType {
    GRUBVERIFY,
    BOOTDELAY,
    DEFAULTBOOT,
};

class GrubTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit GrubTask(QString pwd, bool isOpen, int type);
    explicit GrubTask(int order, int type);
    ~GrubTask();

    void run() override;

    int getTaskType();

private:
    bool setGrubPasswordSlot(QString pwd, bool isOpen);
    bool changeBootDelay(int delay);
    bool setDefaultBoot(int order);

private:
    QDBusInterface *m_pUkccSystemDbus = nullptr;

    QString m_pwd;
    bool m_isOpen;
    int m_order;
    int m_delay;
    int m_type;

Q_SIGNALS:
    void taskFinished(bool success);
};

#endif // GRUBTASK_H
