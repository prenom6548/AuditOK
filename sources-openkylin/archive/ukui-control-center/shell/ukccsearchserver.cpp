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
#include "ukccsearchserver.h"
#include "interface.h"

UkccSearchServer::UkccSearchServer(MainWindow* parent, bool isDbus)
    : QDBusAbstractAdaptor(parent)
    , m_autoExitTimer(new QTimer(this))
{
    // 如果为非Dbus参数启动，则直接跳过
    if (isDbus) {
        parent->loadAllModules();
        parent->loadSearchXml();
        m_autoExitTimer->setInterval(2000);
        m_autoExitTimer->setSingleShot(true);
        connect(m_autoExitTimer, &QTimer::timeout, this, [=]() {
            if(!this->parent()->isVisible()) {
                QCoreApplication::quit();
            }
        });
        m_autoExitTimer->start();
    }
}

UkccSearchServer::~UkccSearchServer()
{
}

MainWindow* UkccSearchServer::parent() const
{
    return static_cast<MainWindow *>(QObject::parent());
}

QVariantMap UkccSearchServer::getSearchItems()
{
    QVariantMap searchMap = this->parent()->getSearchItems();
    m_autoExitTimer->start();
    return searchMap;
}
