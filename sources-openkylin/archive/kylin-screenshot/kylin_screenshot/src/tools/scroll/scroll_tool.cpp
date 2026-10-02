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
#include "scroll_tool.h"
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QtX11Extras/QX11Info>
#include <X11/extensions/shape.h>

ScrollTool::ScrollTool(QObject *parent) : AbstractActionTool(parent)
{
    setObjectName("Kylin-screenshot_QPushButton_scrollTool");
}

bool ScrollTool::closeOnButtonPressed() const
{
    return false;
}

QIcon ScrollTool::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "scrollshot.svg")
        : QIcon(QStringLiteral(":/img/material/white/") + "scrollshot.png");
}

#ifdef SUPPORT_UKUI
QIcon ScrollTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    if ((context.style_name.compare("ukui-dark") == 0)
        || (context.style_name.compare("ukui-black") == 0)) {
        return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "scrollshot.svg")
               : QIcon(QStringLiteral(":/img/material/black/") + "scrollshot.svg");
    }

    return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "scrollshot.svg")
        : QIcon(QStringLiteral(":/img/material/white/") + "scrollshot.png");
}
#endif

QString ScrollTool::name() const
{
    return tr("scrollshot");
}

QString ScrollTool::nameID()
{
    return QLatin1String("");
}

QString ScrollTool::description() const
{
    return tr("Click to activate scroll screenshot mode");
}

CaptureTool *ScrollTool::copy(QObject *parent)
{
    return new ScrollTool(parent);
}

void ScrollTool::pressed(const CaptureContext &context)
{
    AbstractActionTool::pressed(context);
    return;
}
