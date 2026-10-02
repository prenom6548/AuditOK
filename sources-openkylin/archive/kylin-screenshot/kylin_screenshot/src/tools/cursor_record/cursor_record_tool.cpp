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
#include "cursor_record_tool.h"

CursorRecordTool::CursorRecordTool(QObject *parent) : AbstractActionTool(parent)
{
    setIsInitActive(true);
    setIsPressed(true);
}

bool CursorRecordTool::closeOnButtonPressed() const
{
    return false;
}

QString CursorRecordTool::name() const
{
    return tr("CursorRecord");
}

bool CursorRecordTool::isIsolated() const
{
    return true;
}

QIcon CursorRecordTool::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/record/") + "cursor_record_active.png") :
                      QIcon(QStringLiteral(":/img/material/black/record/") + "cursor_record_noactive.png");
}

#ifdef SUPPORT_UKUI
QIcon CursorRecordTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/record/") + "cursor_record_active.png") :
                      QIcon(QStringLiteral(":/img/material/black/record/") + "cursor_record_noactive.png");
}
#endif

QString CursorRecordTool::description() const
{
    return tr("CursorRecord");
}

CaptureTool *CursorRecordTool::copy(QObject *parent)
{
    return new CursorRecordTool(parent);
}

void CursorRecordTool::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
//    b->setIcon(b->tool()->icon(m_contrastUiColor,false));

    emit requestAction(REQ_CURSOR_RECORD);
}
