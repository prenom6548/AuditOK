/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 */

//
// Created by hxf on 22-7-21.
//

#include "progress-bar.h"

#include <QIcon>
#include <QDebug>
#include <QHBoxLayout>
#include <QApplication>
#include <QPixmap>

ProgressBar::ProgressBar(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(5);
    setLayout(layout);

    m_button = new QPushButton(this);
    m_button->setFixedSize(64, 64);
    m_button->setFlat(false);

    layout->addWidget(m_button);

    m_label = new QLabel(this);
    m_label->setFixedSize(QSize(24, 24));
    layout->addWidget(m_label);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setFixedHeight(64);
    layout->addWidget(m_progressBar);

    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);

    connect(m_button, &QPushButton::clicked, this, [=] {
        Q_EMIT add(m_progressBar->value() + 5);
    });
}

void ProgressBar::setIcon(const QString &icon)
{
    qDebug() << "ProgressBar::setIcon: " << icon;
    QPixmap pixmap =  QPixmap(QIcon::fromTheme(icon).pixmap(QSize(24, 24)));

    m_label->setPixmap(pixmap);
    m_label->setFixedSize(pixmap.size() / pixmap.devicePixelRatio());
    m_button->setIcon(QIcon::fromTheme(icon));
}

void ProgressBar::setValue(int size)
{
    qDebug() << "ProgressBar::setValue: " << size;
    m_progressBar->setValue(size);
}
