/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "middleframemanager.h"

#include <QGridLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QEvent>
#include <QLibrary>
#include <QStringList>
#include <QDir>
#include <QWidgetList>
#include <QKeyEvent>

namespace KInstaller {

MiddleFrameManager::MiddleFrameManager(QWidget *parent) : QWidget(parent)
{
    this->initUI();
    this->addStyleSheet();
}

MiddleFrameManager::~MiddleFrameManager()
{

}

void MiddleFrameManager::initUI()
{
    //中间整个窗体布局
    gLayout = new QGridLayout(this);
    gLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(gLayout);
    gLayout->setRowStretch(0, 1);
    gLayout->setRowStretch(1, 1);

    m_Widget = new QWidget();
    gLayout->addWidget(m_Widget, 0, 0, 1, 1);



}
void MiddleFrameManager::translateStr()
{
    m_nextBtn->setText(tr("next"));
}

void MiddleFrameManager::initNextBnt()
{
    m_nextBtn = new QPushButton;
    m_nextBtn->setObjectName("nextBtn");
    m_nextBtn->setFocusPolicy(Qt::TabFocus);
    gLayout->addWidget(m_nextBtn, 1, 0, 1, 1, Qt::AlignHCenter);
    QObject::connect(m_nextBtn, &QPushButton::clicked, this, &MiddleFrameManager::clickNextButton,Qt::UniqueConnection);
    QObject::connect(this, &MiddleFrameManager::enterpressed, this, &MiddleFrameManager::clickNextButton,Qt::UniqueConnection);
    translateStr();
}

void MiddleFrameManager::initPreBnt()
{

}

void MiddleFrameManager::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}



void MiddleFrameManager::addStyleSheet()
{
    QFile file(":/data/qss/MiddleFrameManager.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}



void MiddleFrameManager::keyPressEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO << "key pressed in middleframemanager!";
    if(event->key() == Qt::Key_Enter)
    {
        clickNextButton();
    }
    else {
        QWidget::keyPressEvent(event);
    }
}

}
