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

#ifndef _FCITXCONTROLLERPROXY_H_
#define _FCITXCONTROLLERPROXY_H_

#include <QDBusAbstractInterface>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QObject>

#include "fcitxdbustypes.h"

class FcitxControllerProxy : public QDBusAbstractInterface
{
  Q_OBJECT

public:
  FcitxControllerProxy(const QString &service, const QString &path,
                       const QDBusConnection &connection,
                       QObject *parent = nullptr)
      : QDBusAbstractInterface(service, path, staticInterfaceName(), connection,
                               parent) {}

  ~FcitxControllerProxy() override = default;

private:
  const char *staticInterfaceName()
  {
    return "org.fcitx.Fcitx.Controller1";
  }

public Q_SLOTS: // METHODS

  QDBusPendingReply<FcitxQtInputMethodEntryList>
  AvailableInputMethods()
  {
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("AvailableInputMethods"),
                                     argumentList);
  }

  QDBusPendingReply<FcitxQtLayoutInfoList> AvailableKeyboardLayouts()
  {
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("AvailableKeyboardLayouts"),
                                     argumentList);
  }

  QDBusPendingReply<bool> CheckUpdate()
  {
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("CheckUpdate"),
                                     argumentList);
  }

  QDBusPendingReply<> Configure()
  {
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("Configure"), argumentList);
  }

  QDBusPendingReply<> ConfigureIM(const QString &in0)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(in0);
    return asyncCallWithArgumentList(QStringLiteral("ConfigureIM"),
                                     argumentList);
  }

  QDBusPendingReply<QString> CurrentInputMethod()
  {
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("CurrentInputMethod"),
                                     argumentList);
  }

  QDBusPendingReply<QString> CurrentInputMethodGroup()
  {
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("CurrentInputMethodGroup"),
                                     argumentList);
  }

  QDBusPendingReply<QDBusVariant, FcitxQtConfigTypeList>
  GetConfig(const QString &in0)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(in0);
    return asyncCallWithArgumentList(QStringLiteral("GetConfig"), argumentList);
  }

  QDBusPendingReply<QString, FcitxQtStringKeyValueList>
  InputMethodGroupInfo(const QString &in0)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(in0);
    return asyncCallWithArgumentList(QStringLiteral("InputMethodGroupInfo"),
                                     argumentList);
  }

  QDBusPendingReply<QStringList> InputMethodGroups()
  {
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("InputMethodGroups"),
                                     argumentList);
  }

  QDBusPendingReply<> Refresh()
  {
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("Refresh"), argumentList);
  }

  QDBusPendingReply<> Restart()
  {
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("Restart"), argumentList);
  }

  QDBusPendingReply<QString, QString, QString, QVariantMap,
                    FcitxQtFullInputMethodEntryList>
  FullInputMethodGroupInfo(const QString &in0)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(in0);
    return asyncCallWithArgumentList(QStringLiteral("FullInputMethodGroupInfo"),
                                     argumentList);
  }

  QDBusPendingReply<> SetConfig(const QString &in0,
                                const QDBusVariant &in1)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
    return asyncCallWithArgumentList(QStringLiteral("SetConfig"), argumentList);
  }

  QDBusPendingReply<>
  SetInputMethodGroupInfo(const QString &name, const QString &layout,
                          FcitxQtStringKeyValueList entries)
  {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(name) << QVariant::fromValue(layout)
                 << QVariant::fromValue(entries);
    return asyncCallWithArgumentList(QStringLiteral("SetInputMethodGroupInfo"),
                                     argumentList);
  }

  QDBusPendingReply<FcitxQtHotkeyInfoList> GetAllHotkeys()
  {
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("GetAllHotkeys"),
                                     argumentList);
  }

Q_SIGNALS:
  void InputMethodGroupsChanged();
};

#endif
