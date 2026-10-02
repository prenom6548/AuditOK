/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "videowidget.h"
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QDebug>

videoWidget::videoWidget(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("border-image: none; background: green; border:none");
    m_videoLabel = new QLabel(this);
//    m_videoLabel->setScaledContents(true);
    m_movie = new QMovie;
    m_videoLabel->setMovie(m_movie);
    m_movie->setFileName(":/gif/resources/右边缘左划显示侧边栏.gif");
    qDebug() << "scaledSize：" << m_movie->scaledSize() << m_movie->frameRect();

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->addStretch();
    vlayout->addWidget(m_videoLabel);
    vlayout->addStretch();
    m_movie->start();
}

videoWidget::~videoWidget()
{
    if(m_movie) {
        m_movie->stop();
        delete m_movie;
        m_movie = nullptr;
    }
}

void videoWidget::adjustVideoSize(int width)
{
    QSize newSize(width, width / 16 * 9);
    qDebug() << __func__ << "newSize：" << newSize;
    m_movie->setScaledSize(newSize);
}

void videoWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << __func__ << event << event->size();

    QWidget::resizeEvent(event);
}
