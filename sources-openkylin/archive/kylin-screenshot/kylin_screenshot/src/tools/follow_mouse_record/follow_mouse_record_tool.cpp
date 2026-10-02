/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "follow_mouse_record_tool.h"

FollowMouseRecordTool::FollowMouseRecordTool(QObject *parent) : AbstractActionTool(parent)
{
}

bool FollowMouseRecordTool::closeOnButtonPressed() const
{
    return false;
}

QString FollowMouseRecordTool::name() const
{
    return tr("FollowMouseRecord");
}

bool FollowMouseRecordTool::isIsolated() const
{
    return true;
}

QIcon FollowMouseRecordTool::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/record/") + "follow_mouse_record_active.png") :
                      QIcon(QStringLiteral(":/img/material/black/record/") + "follow_mouse_record_noactive.png");
    //    return QIcon(QStringLiteral(":/img/material/black/") + "luping_icon.svg");
}
#ifdef SUPPORT_UKUI
QIcon FollowMouseRecordTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/record/") + "follow_mouse_record_active.png") :
                      QIcon(QStringLiteral(":/img/material/black/record/") + "follow_mouse_record_noactive.png");
    //    return QIcon(QStringLiteral(":/img/material/black/") + "luping_icon.svg");
}
#endif
QString FollowMouseRecordTool::description() const
{
    return tr("FollowMouseRecord");
}

CaptureTool *FollowMouseRecordTool::copy(QObject *parent)
{
    return new FollowMouseRecordTool(parent);
}

void FollowMouseRecordTool::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_FOLLOW_MOUSE_RECORD);
}
