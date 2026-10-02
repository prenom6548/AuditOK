/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "loadingwidget.h"

#include <QApplication>
#include <QPainterPath>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>
#include <QIcon>
#include <QKeyEvent>
#include <QDateTime>
#include <ukuistylehelper/ukuistylehelper.h>

LoadingWidget::LoadingWidget(QWidget *parent, float opacity)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(0,0,0,0);
    Qt::WindowFlags flag = windowFlags();
    flag |= Qt::FramelessWindowHint;
//    setWindowFlags(flag);

    kdk::UkuiStyleHelper::self()->removeHeader(this);

    m_pLayoutMain = new QVBoxLayout();
    m_pLayoutMain->setContentsMargins(0,0,0,0);
    m_pLayoutMain->setSpacing(0);
    m_pLayoutMain->setAlignment(Qt::AlignCenter);

    m_pLoadingLabel = new KBallonTip();
    m_pLoadingLabel->setContentsMargins(14, 6, 14, 6);
    m_pLoadingLabel->setTipType(TipType::Variable);
    m_pLayoutMain->addWidget(m_pLoadingLabel);

    m_pTimerLoading = new QTimer(this);
    connect(m_pTimerLoading, SIGNAL(timeout()), this, SLOT(onLoading()));
    m_pTimerLoading->setInterval(100);
    m_pTimerLoading->start();
    m_lluInitTick = QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_transparency = opacity;
    this->setLayout(m_pLayoutMain);
}

LoadingWidget::~LoadingWidget()
{
    if (m_pTimerLoading) {
        if(m_pTimerLoading->isActive())
            m_pTimerLoading->stop();
    }
}

void LoadingWidget::onLoading()
{
    QString strFrame = QString("ukui-loading-%1-symbolic").arg(m_nCurTick);
    QIcon iconFrame = QIcon::fromTheme(strFrame);
    if (!iconFrame.isNull()) {
        m_pLoadingLabel->setVariableIcon(iconFrame);
    }
    m_nCurTick++;
    if (m_nCurTick > 7) {
        m_nCurTick = 0;
    }
}

void LoadingWidget::setLoading()
{
    if (m_pTimerLoading) {
        if(!m_pTimerLoading->isActive()) {
            m_pTimerLoading->start();
        }
    }
}

void LoadingWidget::setLoadingText(QString text)
{
    m_pLoadingLabel->setText(tr("Modifying %1").arg(text));
}

QSize LoadingWidget::labelsize()
{
    return m_pLoadingLabel->size();
}

void LoadingWidget::closeEvent(QCloseEvent *event)
{
    if (m_pTimerLoading) {
        if(m_pTimerLoading->isActive())
            m_pTimerLoading->stop();
    }
}

void LoadingWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape){
        if (QDateTime::currentDateTime().toMSecsSinceEpoch()-m_lluInitTick>=10*1000){
            //this->close();
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}

void LoadingWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void LoadingWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void LoadingWidget::paintEvent(QPaintEvent *event)
{
    QPainterPath path;
    QPainter painter(this);
    painter.setOpacity(m_transparency);
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
