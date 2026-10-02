/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#include "scrollsettingwidget.h"
#include "kmreenv.h"
#include "preferences.h"

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

ScrollSettingWidget::ScrollSettingWidget(QString pkgName, QWidget *parent)
    :QWidget(parent)
{
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    QFrame *centralWidget = new QFrame;
    centralWidget->setFixedSize(300,150);
    centralWidget->setObjectName("mainWid");
    centralWidget->setStyleSheet("#mainWid{background:palette(Base);border-radius:12px;border: 1px solid rgba(0, 0, 0, 0.14901960784313725);}");
    QLabel *m_title = new QLabel(tr("Mouse sensitivity"));
    m_title->setFixedWidth(80);
    m_title->setWordWrap(true);
    KmreConfig::Preferences *pref = KmreConfig::Preferences::getInstance();
    QString scrollstr = QString("%1").arg(pref->mScrollSensitivity);
    m_line = new QLineEdit;
    m_line->setFixedWidth(120);
    m_line->setText(scrollstr);
    m_slider = new QSlider;
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setValue(pref->mScrollSensitivity);
    m_slider->setMaximum(100);
    m_slider->setMinimum(0);
    QLabel *lab1 = new QLabel(tr("slow"));
    QLabel *lab2 = new QLabel(tr("quick"));
    QPushButton *m_okBtn = new QPushButton;
    QPushButton *m_cancelBtn = new QPushButton;
    m_okBtn->setFixedWidth(100);
    m_cancelBtn->setFixedWidth(100);
    m_okBtn->setText(tr("ok"));
    m_cancelBtn->setText(tr("cancel"));
    QHBoxLayout *linelayout = new QHBoxLayout;
    linelayout->addWidget(m_title, 0, Qt::AlignVCenter);
    linelayout->addWidget(lab1);
    linelayout->addWidget(m_slider, 0, Qt::AlignVCenter);
    linelayout->addWidget(lab2);
    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addWidget(m_okBtn, 0, Qt::AlignCenter);
    buttonlayout->addWidget(m_cancelBtn, 0, Qt::AlignCenter);
    QVBoxLayout *m_layout = new QVBoxLayout(centralWidget);
    m_layout->addLayout(linelayout);
    m_layout->addLayout(buttonlayout);
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->addWidget(centralWidget);
    this->setLayout(mainlayout);
    connect(m_cancelBtn, &QPushButton::clicked, this, [=]() {
        this->close();
    });
    connect(m_okBtn, &QPushButton::clicked, this, [=]() {
        pref->updateScrollSensitivity(m_slider->value());
        this->close();
    });
}
