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

#include "ocrtool.h"
#include "src/utils/screenshotsaver.h"
#include <QPainter>
#include <QStandardPaths>
#include <QVariant>
#include <QSettings>
#include <QProcess>

OcrTool::OcrTool(QObject *parent) : AbstractActionTool(parent)
{
    setObjectName("Kylin-screenshot_QPushButton_ocrTool");
}

bool OcrTool::closeOnButtonPressed() const
{
    return true;
}

QIcon OcrTool::icon(const QColor &background, bool inEditor) const
{
    // Q_UNUSED(inEditor);
    // return QIcon(iconPath(background) + "content-ocr.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "ocr.png") :
                      QIcon(QStringLiteral(":/img/material/white/") + "ocr.png");
//    return QIcon();
}

#ifdef SUPPORT_UKUI
QIcon OcrTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    // Q_UNUSED(inEditor);
    // return QIcon(iconPath(background) + "content-ocr.svg");
    Q_UNUSED(background);
    if ((context.style_name.compare("ukui-dark") == 0)
            || (context.style_name.compare("ukui-black") == 0)) {
        return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "ocr.svg")
                        : QIcon(QStringLiteral(":/img/material/dark-theme/") + "ocr.svg");
    }
    // if((context.style_name.compare("ukui-white")==0) || (context.style_name.compare("ukui-default")==0) || (context.style_name.compare("ukui-light")==0)){
    else {
        return inEditor ? QIcon(QStringLiteral(":/img/material/black/") + "ocr1.svg")
                        : QIcon(QStringLiteral(":/img/material/white/") + "ocr.svg");
    }
}

#endif
QString OcrTool::name() const
{
    return tr("ocr");
}

QString OcrTool::nameID()
{
    return QLatin1String("");
}

QString OcrTool::description() const
{
    return tr("Start Ocr");
}

CaptureTool *OcrTool::copy(QObject *parent)
{
    return new OcrTool(parent);
}

void OcrTool::pressed(const CaptureContext &context)
{
    AbstractActionTool::pressed(context);
    QGSettings *ScreenshotGsettings = new QGSettings("org.ukui.screenshot");
    QString path = ScreenshotGsettings->get("screenshot-path").toString();
    if(path.isEmpty()){
        path = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0);
    }

    bool ok = ScreenshotSaver().saveToFilesystem(
        context.selectedScreenshotArea(), path);
    if (ok) {
        emit requestAction(REQ_OCR);
    } else {
        emit captureExit(0);
    }

    if(ScreenshotGsettings) {
        delete ScreenshotGsettings;
    }
}
