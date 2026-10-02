/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MEDIAKEYBINDING_H
#define MEDIAKEYBINDING_H

#include <QDebug>
#include <QObject>
#include <QAction>
#include <KF5/KGlobalAccel/KGlobalAccel>
#include "media-type.h"
#include "media-key-action.h"
extern "C" {
#include "clib-syslog.h"
}

typedef QList<QKeySequence> KeySeqList;

class MediaKeyBinding : public QObject
{
    Q_OBJECT
public:

    explicit MediaKeyBinding(const QString& actionName, ActionType type, QString& shortcuts, QObject *parent = nullptr);
    explicit MediaKeyBinding(const QString& actionName, ActionType type, const KeySeqList& shortcuts, QObject *parent = nullptr);
    explicit MediaKeyBinding(const QString actionName, ActionType type, QString shortcuts, QString command, QString summary, QObject *parent = nullptr);

    ~MediaKeyBinding();

    const QString actionName() const;
    ActionType actionType() const;
    const KeySeqList shortcuts() const;

    void setShortcuts(QString shortcuts);
    void setShortcuts(const KeySeqList& shortcuts);

    void registerGlobalShortcut();
    void unregisterGlobalShortcut();

    static const KeySeqList listFromString(QString& shortcuts);
    void setActionManager(MediaKeyAction *mediaAction);
private:
    void init();
    const KeySeqList getGlobalShortcut();
private:
    ActionType m_type;
    KeySeqList m_shortcuts;
    QAction *m_action = nullptr;

    QString m_actionName = "";
    QString m_command = "";
    QString m_summary = "";
};

#endif // MEDIAKEYBINDING_H
