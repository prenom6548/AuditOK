/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "pictureunit.h"
#include <QApplication>
#include <widgets/maskwidget.h>
#include <QTouchEvent>
#include <QGSettings>
#include <QDebug>
#include <qmath.h>

PictureUnit::PictureUnit(QWidget *parent) : QLabel(parent)
{
    _filename = "";

    hoverStyleSheet = QString("border-width: 3px;border-style: solid;border-color: palette(highlight);");
    clickedStyleSheet = QString("border-width: 4px;border-style: solid;border-color: palette(highlight);");
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *qtSettings = new QGSettings("org.ukui.style", QByteArray(), this);
        connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "themeColor" && this->clickedFlag) {
                this->setStyleSheet(clickedStyleSheet);
            }
        });
    }
    clickedFlag = false;
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(QSize(198, 136));
    setScaledContents(true);

    MaskWidget *maskWidget = new MaskWidget(this);
    maskWidget->setGeometry(0, 0, this->width(), this->height());
    isTouchControl = false;
    qApp->installEventFilter(this);
    this->setAttribute(Qt::WA_AcceptTouchEvents);
}

PictureUnit::~PictureUnit() {}

void PictureUnit::setFilenameText(QString fn)
{
    _filename = fn;
}

QString PictureUnit::filenameText()
{
    return _filename;
}

void PictureUnit::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && !isTouchControl && e->pos().x() >= 0 && e->pos().y() >= 0
        && e->pos().x() <= width() && e->pos().y() <= height()) {

        changeClickedFlag(true);
        Q_EMIT clicked(_filename);
    }
}

void PictureUnit::enterEvent(QEvent *e)
{
    if (getClickedFlag() == false && !isTouchControl) {
        setFrameShape(QFrame::Box);
        setStyleSheet(hoverStyleSheet);
    }
}
void PictureUnit::leaveEvent(QEvent *e)
{
    if (getClickedFlag() == false) {
        setStyleSheet("border-width: 0px;");
    }
}

bool PictureUnit::getClickedFlag()
{
    return clickedFlag;
}

void PictureUnit::changeClickedFlag(bool flag)
{
    clickedFlag = flag;
    if (flag) {
        this->setFrameShape(QFrame::Box);
        this->setStyleSheet(clickedStyleSheet);
    } else {
        this->setStyleSheet("border-width: 0px;");
    }
}

bool PictureUnit::eventFilter(QObject *w, QEvent *e)
{
    if (w == this) {
        if (e->type() == QEvent::TouchBegin) {
            isTouchControl = true;
            QTouchEvent *touchE = static_cast<QTouchEvent *>(e);
            touchBeginPoint = touchE->touchPoints().at(0).screenPos();
        }
    } else if (e->type() == QEvent::TouchEnd && isTouchControl) {
        isTouchControl = false;
        QTouchEvent *touchE = static_cast<QTouchEvent *>(e);
        touchEndPoint = touchE->touchPoints().at(0).screenPos();
        if (qFabs(touchEndPoint.x() - touchBeginPoint.x()) < 1.0
            && qFabs(touchEndPoint.y() - touchBeginPoint.y()) < 1.0) {
            emit clicked(_filename);
        }
    }
    return qApp->eventFilter(w, e);
}
