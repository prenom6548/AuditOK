/*
 * libkysdk-shortcut's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "keybind.h"
#include <QProcess>
#include <QSettings>

Keybind::Keybind(QString name, QString key, QString action, QString settingsPath, QString componentName, QObject *parent)
    : QObject(parent),
      mActionName(name),
      mBindKey(key),
      mActionCmd(action),
      mSettingsPath(settingsPath),
      mComponentName(componentName)
{
    mAction = new QAction(this);
    mAction->setObjectName(mActionName);
    mAction->setProperty("componentName", mComponentName);
    setUp();
}

void Keybind::reConnectAction()
{
    disconnect(mAction, &QAction::triggered, this, &Keybind::onActionTriggered);
    setShortcut();
}

void Keybind::setUp()
{
    setShortcut();
}

void Keybind::setShortcut()
{
    QList<QKeySequence> seq = listFromString();
    KGlobalAccel::self()->setDefaultShortcut(mAction, seq);
    KGlobalAccel::self()->setShortcut(mAction, seq);
    connect(mAction, &QAction::triggered, this, &Keybind::onActionTriggered);
}

void Keybind::parsingDesktop(QString exec)
{
    QStringList exelist;
    if (exec.contains(".desktop"))
    {
        QSettings setting(exec, QSettings::IniFormat);
        setting.beginGroup("Desktop Entry");
        QString exec = setting.value("Exec").toString();
        exelist = exec.split(0x20);
        setting.endGroup();
        if (0 == exelist.count())
        {
            qDebug() << "error %s can't find Exec!" << exec.toLatin1().data();
            return;
        }
    }
    else
    {
        exelist = exec.split(0x20);
    }
    qDebug() << exelist[0];
    QProcess process;
    process.startDetached(exelist[0]);
}

QList<QKeySequence> Keybind::listFromString()
{
    mBindKey.replace("<", "");
    mBindKey.replace(">", "+");
    if (mBindKey.contains("Win"))
    {
        mBindKey.replace("Win", "Meta");
    }
    return QKeySequence::listFromString(mBindKey);
}

void Keybind::onActionTriggered()
{
    qDebug() << "shortcut action name " << mActionCmd.toLatin1().data();
    parsingDesktop(mActionCmd);
}
