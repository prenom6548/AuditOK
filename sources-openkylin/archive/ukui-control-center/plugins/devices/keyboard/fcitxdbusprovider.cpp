/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "fcitxdbusprovider.h"

#include <QDebug>

FcitxDbusProvider::FcitxDbusProvider(QObject *parent) : QObject(parent)
{
  registerFcitxQtDBusTypes();
  initServiceWatcher();
  initFcitxControlerProxy();
  connectSignals();
}

void FcitxDbusProvider::initFcitxControlerProxy()
{
  m_proxy = new FcitxControllerProxy(m_service, m_path,
                                     QDBusConnection::sessionBus(), this);
}

void FcitxDbusProvider::destroyFcitxControlerProxy()
{
  if (m_proxy == nullptr)
  {
    return;
  }

  delete m_proxy;
  m_proxy = nullptr;
}

void FcitxDbusProvider::initServiceWatcher()
{
  m_watcher =
      new QDBusServiceWatcher(m_service, QDBusConnection::sessionBus(),
                              QDBusServiceWatcher::WatchForRegistration |
                                  QDBusServiceWatcher::WatchForUnregistration,
                              this);
}

void FcitxDbusProvider::connectSignals()
{
  QObject::connect(m_watcher, &QDBusServiceWatcher::serviceRegistered, this,
                   [this]()
                   {
                     qDebug() << "FcitxDbusProvider serviceRegistered";

                     initFcitxControlerProxy();
                     emit fcitxDbusServiceEnabled();
                   });

  QObject::connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this,
                   [this]()
                   {
                     qDebug() << "FcitxDbusProvider serviceUnregistered";

                     destroyFcitxControlerProxy();
                     emit fcitxDbusServiceDisabled();
                   });
}

FcitxControllerProxy *FcitxDbusProvider::dbusProxy() const { return m_proxy; }
