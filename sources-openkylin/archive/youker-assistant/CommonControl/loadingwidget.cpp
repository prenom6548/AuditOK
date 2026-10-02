/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "loadingwidget.h"

LoadingWidget::LoadingWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void LoadingWidget::initUI()
{
    m_hLayout = new QHBoxLayout();
    m_hLayout->setContentsMargins(0,0,0,0);
    m_hLayout->setSpacing(0);
    m_vLayout = new QVBoxLayout();
    m_vLayout->setContentsMargins(0,0,0,0);
    m_vLayout->setSpacing(0);

    
    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        settings = new QGSettings("org.ukui.style");
        connect(settings,&QGSettings::changed,this,&LoadingWidget::slot_SettingsChange);
    }

    m_labelLoadIcon = new QLabel();
    QPixmap pixmap;
    QString key = settings->get("styleName").toString();
    if("" != key){
        if(key == "ukui-black" || key == "ukui-dark"){
            pixmap.load(":/imgres/img_res/ukui-occupation-map-dark.png");
        }else{
            pixmap.load(":/imgres/img_res/ukui-occupation-map.png");
        }
    }
    pixmap = pixmap.scaled(96,96,Qt::KeepAspectRatio);
    m_labelLoadIcon->setPixmap(pixmap);

    m_labelLoadText = new QLabel();
    m_labelLoadText->setText(tr("Scanning, please wait"));

    m_vLayout->addStretch(40);
    m_vLayout->addWidget(m_labelLoadIcon, 0, Qt::AlignHCenter);
    m_vLayout->addSpacing(3);
    m_vLayout->addWidget(m_labelLoadText, 0, Qt::AlignHCenter);
    m_vLayout->addStretch(55);

    m_hLayout->addLayout(m_vLayout);
    this->setLayout(m_hLayout);
}

void LoadingWidget::slot_SettingsChange(const QString &key)
{
    if(key == "styleName"){
        if(settings->get("styleName").toString() == "ukui-black" || settings->get("styleName").toString() == "ukui-dark"){
                QPixmap pixmap;
                pixmap.load(":/imgres/img_res/ukui-occupation-map-dark.png");
                pixmap = pixmap.scaled(96,96,Qt::KeepAspectRatio);
                m_labelLoadIcon->setPixmap(pixmap);
        }else{
                QPixmap pixmap;
                pixmap.load(":/imgres/img_res/ukui-occupation-map.png");
                pixmap = pixmap.scaled(96,96,Qt::KeepAspectRatio);
                m_labelLoadIcon->setPixmap(pixmap);
        }
    }
}
