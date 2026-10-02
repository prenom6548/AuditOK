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
#include "start_record_tool.h"
#include "globalvalues.h"
#include <QPushButton>

StartRecordTool::StartRecordTool(QObject *parent) : AbstractActionTool(parent)
{
    QPushButton *parent_b = dynamic_cast<QPushButton*>(parent);
    parent_b->resize(GlobalValues::buttonBaseSize(), GlobalValues::buttonBaseSize());
    parent_b->setMask(QRegion(QRect(-1,-1, GlobalValues::buttonBaseSize()+2,
                          GlobalValues::buttonBaseSize()+2),
                    QRegion::Rectangle));
    parent_b->setStyleSheet("QPushButton{font-family:'宋体';font-size:40px;color:rgb(255,255,255,255);}");
    parent_b->setFlat(false);
    //setStyleSheet(styleSheet());
    //font.setStyle()
    QFont font ( "Noto Sans CJK Regular", 12, 20);
    parent_b->setFont(font);
    parent_b->setText("开始录制");
}

bool StartRecordTool::closeOnButtonPressed() const
{
    return false;
}

QString StartRecordTool::name() const
{
    return tr("StartRecord");
}

QIcon StartRecordTool::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
    return QIcon();
}
#ifdef SUPPORT_UKUI
QIcon StartRecordTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
    return QIcon();
}
#endif
QString StartRecordTool::description() const
{
    return tr("StartRecord");
}

CaptureTool *StartRecordTool::copy(QObject *parent)
{
    return new StartRecordTool(parent);
}

void StartRecordTool::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_START_RECORD);
}
