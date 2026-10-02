/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#include "option_record_tool.h"

OptionRecordTool::OptionRecordTool(QObject *parent) : AbstractActionTool(parent)
{
}
bool OptionRecordTool::closeOnButtonPressed() const
{
     return false;
}

QIcon OptionRecordTool::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
   // return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-copy.svg") :
     //                 QIcon(QStringLiteral(":/img/material/white/") + "content-copy.svg");
    return QIcon();
}
#ifdef SUPPORT_UKUI
QIcon OptionRecordTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
   // return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-copy.svg") :
     //                 QIcon(QStringLiteral(":/img/material/white/") + "content-copy.svg");
    return QIcon();

}
#endif
QString OptionRecordTool::name() const
{
    return tr("OptionRecord");
}
QString OptionRecordTool::nameID()
{
     return QLatin1String("");
}
QString OptionRecordTool::description() const
{
    return tr("options record tool");
    //return tr("图片保存配置");
}

CaptureTool* OptionRecordTool::copy(QObject *parent)
{
     return new OptionRecordTool(parent);
}

void OptionRecordTool::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_OPTION_RECORD);
}
