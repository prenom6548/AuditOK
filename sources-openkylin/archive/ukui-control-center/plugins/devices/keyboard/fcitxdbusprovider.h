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

#ifndef _FCITXDBUSPROVIDER_H_
#define _FCITXDBUSPROVIDER_H_

#include <QDBusServiceWatcher>
#include <QObject>

#include "fcitxcontrollerproxy.h"

class FcitxDbusProvider : public QObject
{
  Q_OBJECT

public:
  explicit FcitxDbusProvider(QObject *parent = nullptr);
  ~FcitxDbusProvider() override = default;

  FcitxControllerProxy *dbusProxy() const;

private:
  void initFcitxControlerProxy();
  void initServiceWatcher();
  void destroyFcitxControlerProxy();
  void connectSignals();

Q_SIGNALS:
  void fcitxDbusServiceEnabled();
  void fcitxDbusServiceDisabled();

private:
  FcitxControllerProxy *m_proxy = nullptr;
  QDBusServiceWatcher *m_watcher = nullptr;
  const QString m_service = "org.fcitx.Fcitx5";
  const QString m_path = "/controller";
};
#endif
