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

#ifndef KEYBINDENTRY_H
#define KEYBINDENTRY_H

#include <QObject>
#include <QAction>
#include <KGlobalAccel>
#include <QDebug>

class Keybind : public QObject
{
    Q_OBJECT
public:
    Keybind(QString name, QString key, QString action, QString settingsPath, QString componentName, QObject *parent = nullptr);

public:
    void reConnectAction();

public:
    void setBindKey(QString bindKey) { mBindKey = bindKey; }
    void setAction(QString execName) { mActionName = execName; }
    QAction *action() { return mAction; }
    QString settingPath() { return mSettingsPath; }
    QString sctionName() { return mActionName; }

private:
    void setUp();
    void setShortcut();
    void parsingDesktop(QString exec);
    QList<QKeySequence> listFromString();

private slots:
    void onActionTriggered();

private:
    QString mActionName;    // 名称唯一
    QString mBindKey;       // 绑定组合键
    QString mActionCmd;     // 组合键动作
    QString mSettingsPath;  // GSetting路径
    QString mComponentName; // 组名称
    QAction *mAction;
};

#endif // KEYBINDENTRY_H
