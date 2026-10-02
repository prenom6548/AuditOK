/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "tipWidget.h"
#include <QTimer>

TipWidget::TipWidget()
: mpParent(nullptr)
, mbEnter(false)
, mnTransparent(200)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAlignment(Qt::AlignCenter);

    mpTimer = new QTimer(this);
    connect(mpTimer, &QTimer::timeout, this, &TipWidget::OnTimer);
}

TipWidget::~TipWidget()
{
    deleteLater();
}

void TipWidget::enterEvent(QEvent *)
{
    mbEnter       = true;
    mnTransparent = 200;
    setStyleSheet(QString("color:white;font:12px \"Microsoft YaHei\";border-radius:5px;background-color:rgba(80, 80, 80, %1);").arg(mnTransparent));
}

void TipWidget::leaveEvent(QEvent *)
{
    mbEnter = false;
}

void TipWidget::OnTimer()
{
    if (mbEnter)
    {
        return;
    }

    mnTransparent -= 3;
    if (mnTransparent > 0)
    {
        if (mpParent && parentWidget())
        {
            QPoint pt((parentWidget()->width() - width()) >> 1, (parentWidget()->height() - height())-100);
            if (pos() != pt)
            {
                move(pt);
            }
        }
        setStyleSheet(QString("color:white;font:12px \"Microsoft YaHei\";border-radius:5px;background-color:rgba(80, 80, 80, %1);").arg(mnTransparent));
    }
    else
    {
        mpTimer->stop();
        setVisible(false);
    }
}

void TipWidget::SetMesseage(const QString &strMessage, const QPoint *pPoint)
{
    if (strMessage.isEmpty())
    {
        return;
    }

    QFontMetrics fm1(font());
    setFixedSize(fm1.width(strMessage) + 30, 30);

    mpParent = parentWidget();

    if (width() > mpParent->width())
    {
        setFixedSize(mpParent->width() - 60, 60);
        setWordWrap(true);
    }
    else
    {
        setWordWrap(false);
    }

    setText(strMessage);

    if (nullptr != mpParent)
    {
        if (nullptr != pPoint)
        {
            move(mpParent->mapFromGlobal(*pPoint));
            mpParent = nullptr;
        }
        else
        {
            move((mpParent->width() - width()) >> 1, (mpParent->height() - height())-100);
        }
    }

    setVisible(true);
    mnTransparent = 200;

    mpTimer->start(30);
}

TipWidget &TipWidget::Instance()
{
    static TipWidget tipWidget;
    return tipWidget;
}
