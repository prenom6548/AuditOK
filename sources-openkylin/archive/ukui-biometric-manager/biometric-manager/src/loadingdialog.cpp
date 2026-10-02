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
#include "loadingdialog.h"

#include <QApplication>
#include <QPainterPath>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>
#include <QIcon>
#include <QKeyEvent>
#include <QDateTime>

LoadingDialog::LoadingDialog(QWidget *parent, float opacity)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(0,0,0,0);
    //setProperty("useStyleWindowManager", false);

    m_layoutMain = new QHBoxLayout();
    m_layoutMain->setContentsMargins(0,0,0,0);
    m_layoutMain->setSpacing(0);
    m_layoutMain->setAlignment(Qt::AlignCenter);

    m_labelLoading = new QLabel();
    m_labelLoading->setProperty("useIconHighlightEffect", 0x2);
    m_layoutMain->addWidget(m_labelLoading);

    m_timerLoading = new QTimer(this);
    connect(m_timerLoading, SIGNAL(timeout()), this, SLOT(onLoading()));
    m_timerLoading->setInterval(100);
    m_timerLoading->start();
    m_lluInitTick = QDateTime::currentDateTime().toMSecsSinceEpoch();
    transparency = opacity;
    this->setLayout(m_layoutMain);
}

LoadingDialog::~LoadingDialog()
{
    if (m_timerLoading) {
        if(m_timerLoading->isActive())
            m_timerLoading->stop();
    }
}

void LoadingDialog::paintEvent(QPaintEvent *event)
{
    QPainterPath path;
    QPainter painter(this);
    painter.setOpacity(transparency);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipping(true);
    painter.setPen(Qt::transparent);
    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(QColor("#000000"));
    painter.setPen(Qt::transparent);

    painter.drawPath(path);
    QWidget::paintEvent(event);
}

void LoadingDialog::onLoading()
{
    QString strFrame = QString("ukui-loading-%1-symbolic").arg(m_nCurTick);
    QIcon iconFrame = QIcon::fromTheme(strFrame);
    if (!iconFrame.isNull()) {
        m_labelLoading->setPixmap(iconFrame.pixmap(40,40));
    }
    m_nCurTick ++;
    if (m_nCurTick>7) {
        m_nCurTick = 0;
    }
}

void LoadingDialog::setLoading()
{
    if (m_timerLoading) {
        if(!m_timerLoading->isActive())
            m_timerLoading->start();
    }
}

void LoadingDialog::closeEvent(QCloseEvent *event)
{
    if (m_timerLoading) {
        if(m_timerLoading->isActive())
            m_timerLoading->stop();
    }
}

void LoadingDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape){
        if (QDateTime::currentDateTime().toMSecsSinceEpoch()-m_lluInitTick>=10*1000){
            //this->close();
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}

void LoadingDialog::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void LoadingDialog::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}
