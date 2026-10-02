/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#include "normaldisplaywidget.h"
#include "kmrewindow.h"

#include <QPainter>
#include <QIcon>
#include <syslog.h>

NormalDisplayWidget::NormalDisplayWidget(KmreWindow* window, int id, int width, int height, int orientation, QString displayName)
    : DisplayWidget(window, id, width, height, orientation, displayName)
{
    this->setAttribute(Qt::WA_OpaquePaintEvent);//要快速更新使用不透明内容（例如视频流部件），最好设置部件的Qt::WA_OpaquePaintEvent，以避免与重新绘制部件背景相关的任何不必要的开销
    this->setAutoFillBackground(false);//避免背景变黑
    this->setAttribute(Qt::WA_NoSystemBackground, false);
    this->setAttribute(Qt::WA_StaticContents);

}

NormalDisplayWidget::~NormalDisplayWidget()
{
    
}

void NormalDisplayWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    this->activateWindow();
}

void NormalDisplayWidget::initializeGL()
{
}

void NormalDisplayWidget::paintGL()
{
    if (updatesEnabled()) {//第一次显示时填充默认背景色，防止显示出桌面背景画面！
        syslog(LOG_DEBUG, "Auto update default display...");
        QRect winRect = rect();
        QPainter p(this);
        // draw background
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);//default
        p.drawRect(winRect);
        // draw icon
        QPixmap pixmap(mMainWindow->getIconPath());
        if (!pixmap.isNull()) {
            QSize widgetSize = winRect.size();
            QSize pixmapSize(80, 80);
            int x = (widgetSize.width() - pixmapSize.width()) / 2;
            int y = (widgetSize.height() - pixmapSize.height()) / 2;
            p.drawPixmap(x, y, pixmapSize.width(), pixmapSize.height(), pixmap);
        }
    }
}

void NormalDisplayWidget::resizeGL(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

void NormalDisplayWidget::setAutoUpdate(bool enable)
{
    syslog(LOG_DEBUG, "Set auto update: '%d'", enable);
    setUpdatesEnabled(enable);
}