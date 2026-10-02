/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "Ukui2AppVolumeWidget.h"

#include <QScrollBar>
#include <QSlider>
#include <QVariant>
#include <QDebug>

#include "IMainWindow.h"

Ukui2AppVolumeWidget::Ukui2AppVolumeWidget(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& map,
                                           std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& appMap,
                                           QPushButton* switchBtn, QPushButton* devVolumeBtn, QPushButton* appVolumeBtn,
                                           IMainWindow* window,
                                           QWidget* parent) :
    m_sysVolumeSliderItemMap(map),
    m_appVolumeSliderItemMap(appMap),
    m_pSwitchModuleBtn(switchBtn),
    m_pDeviceVolumeBtn(devVolumeBtn),
    m_pAppVolumeBtn(appVolumeBtn),
    m_pMainWid(std::move(window)),
    QWidget(parent)
{
    initUi();
}

Ukui2AppVolumeWidget::~Ukui2AppVolumeWidget()
{
//    delete m_pMainWid;
}

void Ukui2AppVolumeWidget::initUi()
{
    m_pSysVolumeLabel = new QLabel(tr("System Volume"));
    m_pAppLabel = new QLabel(tr("Application Volume"));

    m_pBtnWidget = new QWidget(this);
    m_pBtnWidget->setFixedSize(38, 320);

    // 应用音量widget
    m_pAppArea = new QScrollArea(this);
    m_pAppArea->setFixedSize(412, 245);
    m_pAppArea->setFrameShape(QFrame::NoFrame);
    m_pAppArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pAppArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false); // 滚动条背景透明

    QPalette pal = m_pAppArea->palette();
    pal.setColor(QPalette::Window, QColor(0x00, 0xff, 0x00, 0x00));
    m_pAppArea->setPalette(pal);

    m_pDisplayAppVolumeWidget = new QWidget(m_pAppArea);
    m_pDisplayAppVolumeWidget->setFixedWidth(412);
    m_pAppArea->setWidget(m_pDisplayAppVolumeWidget);

    m_pVlayout = new QVBoxLayout(m_pDisplayAppVolumeWidget);
    m_pDisplayAppVolumeWidget->setLayout(m_pVlayout);

    // 整体布局
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->addWidget(m_pAppLabel);
    vLayout->addSpacing(3);
    vLayout->addWidget(m_sysVolumeSliderItemMap[3]->getWidget());
    vLayout->addSpacing(13);
    vLayout->addWidget(m_pSysVolumeLabel);
    vLayout->addSpacing(3);
    vLayout->addWidget(m_pAppArea);
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(8, 12, 0, 0);

    QVBoxLayout* vLayout1 = new QVBoxLayout();
    vLayout1->addWidget(m_pDeviceVolumeBtn);
    vLayout1->addWidget(m_pAppVolumeBtn);
//    vLayout1->addSpacerItem(new QSpacerItem(10, 30, QSizePolicy::Expanding));
    vLayout1->setSpacing(3);
    vLayout1->setContentsMargins(0, 0, 0, 200);
    m_pBtnWidget->setLayout(vLayout1);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(m_pBtnWidget);
    hLayout->addLayout(vLayout);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);
}

void Ukui2AppVolumeWidget::showWidget()
{
    this->show();
}

QWidget* Ukui2AppVolumeWidget::getWidget()
{
    return this;
}

void Ukui2AppVolumeWidget::addAppWidget(int idx)
{
    m_pVlayout->addWidget(m_appVolumeSliderItemMap[idx]->getWidget());
    m_pVlayout->setSpacing(2);
    m_pVlayout->setContentsMargins(0, 0, 0, 0);
//    m_pDisplayAppVolumeWidget->adjustSize();
    m_pDisplayAppVolumeWidget->resize(358, 14+m_pVlayout->count()*68);
    m_pVlayout->update();

    auto it = m_appVolumeSliderItemMap.find(idx);

    if (it != m_appVolumeSliderItemMap.end()) {
        connect(static_cast<QSlider*>(it->second->getSlider()->getWidget()), &QSlider::valueChanged, this, [this](int value) {
            m_pMainWid->setVolume(2, value);
            qDebug() << "AppVolumeWidgetUkui2 value changed , value: " << value;
        });
    }
    else {
        qDebug() << "m_appVolumeSliderItemMap not found such as :" << idx;
    }
    qDebug() << "add wid..., displayVolumeWidge.size" << m_pDisplayAppVolumeWidget->rect();
}

void Ukui2AppVolumeWidget::removeAppWidget(int idx)
{
    auto it = m_appVolumeSliderItemMap.find(idx);

    if (it != m_appVolumeSliderItemMap.end()) {
        m_appVolumeSliderItemMap.erase(it);
    } else {
        qDebug() << "not found " << idx << " in m_appVolumeSliderItemMap.";
    }

    m_pVlayout->setSpacing(2);
    m_pVlayout->setContentsMargins(0, 0, 0, 0);
    m_pDisplayAppVolumeWidget->resize(404, m_appVolumeSliderItemMap.size()*2);
    m_pVlayout->update();
}

void Ukui2AppVolumeWidget::addAppToWidget(const QString& appName, const QString& iconName)
{
    m_pVlayout->addWidget(m_appVolumeSliderItemMap[0]->getWidget());
    m_pVlayout->setSpacing(2);
    m_pVlayout->setContentsMargins(0, 0, 0, 0);
//    m_pDisplayAppVolumeWidget->adjustSize();
    m_pDisplayAppVolumeWidget->resize(358, 14+m_pVlayout->count()*68);
//    m_pDisplayAppVolumeWidget->resize(412, m_pVlayout->count()*50);
    m_pVlayout->update();
}

void Ukui2AppVolumeWidget::removeAppfromWidget(const QString& appName)
{
    auto index = 1;
    if (-1 == index) {
        qDebug() << "can not found " << appName << " in output stream";
        return;
    }

    if (m_pVlayout->takeAt(index) != 0) {
        auto wid = m_pVlayout->takeAt(index)->widget();
        m_pVlayout->removeWidget(wid);
        wid->setParent(nullptr);
        delete wid;
    }

    m_pVlayout->setSpacing(2);
    m_pVlayout->setContentsMargins(0, 0, 0, 0);
//    m_pDisplayAppVolumeWidget->adjustSize();
    m_pDisplayAppVolumeWidget->resize(404, m_pVlayout->count()*50);
    m_pVlayout->update();
    qDebug() << "remove wid..., displayVolumeWidge.size" << m_pDisplayAppVolumeWidget->rect();
}
