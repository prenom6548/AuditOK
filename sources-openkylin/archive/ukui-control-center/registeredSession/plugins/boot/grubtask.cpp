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
#include "grubtask.h"
#include <QDebug>

GrubTask::GrubTask(QString pwd, bool isOpen, int type) :m_pwd(pwd), m_isOpen(isOpen), m_type(type)
{
    setAutoDelete(true);
    m_pUkccSystemDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                      "/",
                                      "com.control.center.interface",
                                      QDBusConnection::systemBus(), this);
    m_pUkccSystemDbus->setTimeout(2147483647); // -1 为默认的25s超时;
    if (!m_pUkccSystemDbus->isValid()){
        qCritical() << "Create Client Interface Failed:" << QDBusConnection::systemBus().lastError();
    }
}

GrubTask::GrubTask(int order, int type) : m_order(order), m_type(type)
{
    if (type == BOOTDELAY) {
        m_delay = order;
    }
    m_pUkccSystemDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                      "/",
                                      "com.control.center.interface",
                                      QDBusConnection::systemBus(), this);
    m_pUkccSystemDbus->setTimeout(2147483647); // -1 为默认的25s超时;
    if (!m_pUkccSystemDbus->isValid()){
        qCritical() << "Create Client Interface Failed:" << QDBusConnection::systemBus().lastError();
    }
}

GrubTask::~GrubTask()
{
}

void GrubTask::run()
{
    bool ret = false;
    switch (m_type) {
    case GRUBVERIFY:
        ret = setGrubPasswordSlot(m_pwd, m_isOpen);
        break;
    case BOOTDELAY:
        ret = changeBootDelay(m_delay);
        break;
    case DEFAULTBOOT:
        ret = setDefaultBoot(m_order);
        break;
    default:
        break;
    }
    Q_EMIT taskFinished(ret);
}

int GrubTask::getTaskType()
{
    return m_type;
}

bool GrubTask::setGrubPasswordSlot(QString pwd, bool isOpen)
{
    QString lang = qgetenv("LANG");

    QDBusReply<bool> reply = m_pUkccSystemDbus->call("setGrupPasswd", "root", pwd, lang, isOpen);
    if (!reply.isValid()) {
        qCritical() << "setGrupPasswd";
    }

    return reply;
}

bool GrubTask::changeBootDelay(int delay)
{
    QString lang = qgetenv("LANG");
    QDBusReply<bool> reply = m_pUkccSystemDbus->call("setGrupBootTime", delay, lang);
    if (!reply.isValid()) {
        qCritical() << "SetBootDelay" << __LINE__;
    }

    return reply;
}

bool GrubTask::setDefaultBoot(int order)
{
    // 启动顺序 grub-install 切换为当前系统grub后设置文件
    QString lang = qgetenv("LANG");
    QDBusReply<bool> reply = m_pUkccSystemDbus->call("setDefaultBootItem", order, lang);
    if (!reply.isValid()) {
        qCritical() << "setDefaultBootItem";
    }

    return reply;
}

