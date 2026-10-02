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

#include <QPainter>
#include <QPainterPath>
#include <QIcon>
#include <QApplication>

LoadingWidget::LoadingWidget(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_iconLabel(nullptr)
    , m_timer(nullptr)
    , m_currentFrame(0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    
    initUI();
    
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &LoadingWidget::updateLoadingIcon);
    m_timer->setInterval(100);
}

LoadingWidget::~LoadingWidget()
{
    if (m_timer && m_timer->isActive()) {
        m_timer->stop();
    }
}

void LoadingWidget::initUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(10);
    m_layout->setAlignment(Qt::AlignCenter);
    
    m_iconLabel = new QLabel(this);
    m_iconLabel->setProperty("useIconHighlightEffect", true);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(64, 64);
    
    m_layout->addWidget(m_iconLabel);
    
    setLayout(m_layout);
}

void LoadingWidget::startLoading()
{
    if (m_timer && !m_timer->isActive()) {
        m_timer->start();
        updateLoadingIcon();
    }
    show();
}

void LoadingWidget::stopLoading()
{
    if (m_timer && m_timer->isActive()) {
        m_timer->stop();
    }
    hide();
}

void LoadingWidget::updateLoadingIcon()
{
    // 使用系统主题中的loading图标，如果没有则使用默认的旋转动画
    QString iconName = QString("ukui-loading-%1-symbolic").arg(m_currentFrame);
    QIcon icon = QIcon::fromTheme(iconName);
    
    if (icon.isNull()) {
        // 如果系统主题中没有找到图标，使用默认的旋转动画
        QPixmap pixmap(64, 64);
        pixmap.fill(Qt::transparent);
        
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // 绘制一个简单的旋转加载动画
        painter.translate(32, 32);
        painter.rotate(m_currentFrame * 45); // 每次旋转45度
        
        QPen pen(QApplication::palette().highlight().color(), 4, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(pen);
        
        QRect rect(-20, -20, 40, 40);
        painter.drawArc(rect, 0, 270 * 16); // 绘制3/4圆弧
        
        m_iconLabel->setPixmap(pixmap);
    } else {
        m_iconLabel->setPixmap(icon.pixmap(64, 64));
    }
    
    m_currentFrame = (m_currentFrame + 1) % 8;
}

void LoadingWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainterPath path;
    path.addRoundedRect(rect(), 8, 8);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // 设置全透明背景
    QColor backgroundColor = QApplication::palette().window().color();
    backgroundColor.setAlpha(0.4); // 设置为完全透明
    painter.setBrush(backgroundColor);

    painter.drawPath(path);
}
