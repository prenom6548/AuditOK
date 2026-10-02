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
#include "media-key-binding.h"
#include "media-key-manager.h"

const KeySeqList MediaKeyBinding::listFromString(QString& keys)
{
    keys.replace("<", "");
    keys.replace(">", "+");
    keys.replace("Win", "Meta", Qt::CaseInsensitive);
    keys.replace("Super", "Meta", Qt::CaseInsensitive);
    keys.replace("Start", "Meta", Qt::CaseInsensitive);
    keys.replace("PrtSc", "Print", Qt::CaseInsensitive);
    return QKeySequence::listFromString(keys);
}

MediaKeyBinding::MediaKeyBinding(const QString& actionName, ActionType type, QString& shortcuts, QObject *parent)
    : QObject(parent)
    , m_actionName(actionName)
    , m_type(type)
    , m_shortcuts(listFromString(shortcuts))
{
    init();
}

MediaKeyBinding::MediaKeyBinding(const QString& actionName, ActionType type, const KeySeqList& shortcuts, QObject *parent)
    : QObject(parent)
    , m_actionName(actionName)
    , m_type(type)
    , m_shortcuts(shortcuts)
{
    init();
}

MediaKeyBinding::MediaKeyBinding(QString actionName, ActionType type, QString shortcuts,  QString command,  QString summary, QObject *parent)
    : QObject(parent)
    , m_actionName(actionName)
    , m_type(type)
    , m_shortcuts(listFromString(shortcuts))
    , m_command(command)
    , m_summary(summary)
{
    init();
}

MediaKeyBinding::~MediaKeyBinding()
{

}

const QString MediaKeyBinding::actionName() const
{
    return m_actionName;
}

ActionType MediaKeyBinding::actionType() const
{
    return m_type;
}

const KeySeqList MediaKeyBinding::shortcuts() const
{
    return m_shortcuts;
}

void MediaKeyBinding::setShortcuts(QString shortcuts)
{
    m_shortcuts = listFromString(shortcuts);
}

void MediaKeyBinding::setShortcuts(const KeySeqList &shortcuts)
{
    m_shortcuts = shortcuts;
}

void MediaKeyBinding::registerGlobalShortcut()
{
    if (!KGlobalAccel::self()->setGlobalShortcut(m_action, m_shortcuts, true)) {
        USD_LOG(LOG_WARNING, "Failed to register global shortcut.");
    }
}

void MediaKeyBinding::unregisterGlobalShortcut()
{
    if (m_action) {
        KGlobalAccel::self()->removeAllShortcuts(m_action);
    }
}

void MediaKeyBinding::init()
{
    if (!m_action) {
        m_action = new QAction(this);
        m_action->setObjectName(m_actionName);
        m_action->setProperty("componentName", QStringLiteral(UKUI_DAEMON_NAME));
        connect(m_action, &QAction::triggered, [=]{
            USD_LOG(LOG_DEBUG, "on Action triggered, m_type=%d, m_actionName=%s, m_command=%s", m_type, m_actionName.toLatin1().data(), m_command.toLatin1().data());
            qobject_cast<MediaKeyManager*>(parent())->doAction(m_type, m_actionName, m_command);
        });
    }
}

const KeySeqList MediaKeyBinding::getGlobalShortcut()
{
    return KGlobalAccel::self()->globalShortcut(QStringLiteral(UKUI_DAEMON_NAME), m_actionName);
}
