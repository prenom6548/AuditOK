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

#include "Ukui4AppVolumeWidget.h"

#include <QHBoxLayout>
#include <QVariant>
#include <QScrollBar>
#include <QDebug>

#include "IMainWindow.h"
#include "Ukui4SystemVolumeSliderItem.h"

using namespace UkuiAudioFramwork;

Ukui4AppVolumeWidget::Ukui4AppVolumeWidget(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& map,
                                           std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& appMap,
                                           IMainWindow* window,
                                           QWidget* parent) :
    m_sysVolumeSliderItemMap(map), m_appVolumeSliderItemMap(appMap), m_pMainWid(std::move(window)), QWidget(parent)
{
}

Ukui4AppVolumeWidget::~Ukui4AppVolumeWidget()
{
//    delete m_pMainWid;
}

void Ukui4AppVolumeWidget::initUi()
{
    m_pSysVolumeLabel = new QLabel(tr("System Volume"));
    m_pAppVolumeLabel = new QLabel(tr("Application Volume"));

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
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_pSysVolumeLabel);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(m_sysVolumeSliderItemMap[EnumToInt(VolumeSliderType::VOLUME_SLIDER_APP_OUTPUT)]->getWidget());
    mainLayout->addSpacing(13);
    mainLayout->addWidget(m_pAppVolumeLabel);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(m_pAppArea);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(8, 12, 0, 0);
}

void Ukui4AppVolumeWidget::showWidget()
{
    this->show();
}

QWidget* Ukui4AppVolumeWidget::getWidget()
{
    return this;
}

void Ukui4AppVolumeWidget::addAppWidget(int idx)
{
    m_pVlayout->addWidget(m_appVolumeSliderItemMap[idx]->getWidget());
    m_pVlayout->setSpacing(2);
    m_pVlayout->setContentsMargins(0, 0, 0, 0);
//    m_pDisplayAppVolumeWidget->adjustSize();
    m_pDisplayAppVolumeWidget->resize(412, m_appVolumeSliderItemMap.size()*50);
    m_pVlayout->update();

    auto it = m_appVolumeSliderItemMap.find(idx);

    if (it != m_appVolumeSliderItemMap.end()) {
        connect(static_cast<QSlider*>(it->second->getSlider()->getWidget()), &QSlider::valueChanged, this, [this](int value) {
            m_pMainWid->setVolume(2, value);
            qDebug() << "AppVolumeWidgetUkui4 value changed , value: " << value;
        });
    }
    else {
        qDebug() << "m_appVolumeSliderItemMap not found such as :" << idx;
    }
    qDebug() << "add wid..., displayVolumeWidge.size" << m_pDisplayAppVolumeWidget->rect();
}

void Ukui4AppVolumeWidget::removeAppWidget(int idx)
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

void Ukui4AppVolumeWidget::addAppToWidget(const QString& appName, const QString& iconName)
{
    ++sinkInputCount;
    // 显示应用音量
    auto subAppwidget = new QWidget(m_pDisplayAppVolumeWidget);
    auto appVolumeLabel = new QLabel(subAppwidget);
    appVolumeLabel->setParent(subAppwidget);
    auto appIconBtn = new QPushButton(subAppwidget);
    appIconBtn->setAccessibleName(appName);
    auto appMuteBtn = new QPushButton(subAppwidget);
    appMuteBtn->setAccessibleName(tr("Mute %1").arg(appName));
    appMuteBtn->setVisible(false);
    auto appSlider = new QSlider(subAppwidget);
    appSlider->setAccessibleName(tr("%1 volume").arg(appName));
    appSlider->setOrientation(Qt::Horizontal);
    appSlider->setProperty("needTranslucent", true); // Increase translucent effect
    appIconBtn->setFixedSize(32, 32);
    appIconBtn->setFlat(true);
    appIconBtn->setFocusPolicy(Qt::NoFocus);
    QSize icon_size(32, 32);
    appIconBtn->setIconSize(icon_size);
    appIconBtn->setIcon(QIcon::fromTheme(iconName));
    appIconBtn->setAttribute(Qt::WA_TranslucentBackground);

    auto palete = appIconBtn->palette();
    palete.setColor(QPalette::Highlight, Qt::transparent);
    palete.setBrush(QPalette::Button, QBrush(QColor(1, 1, 1, 0)));
    appIconBtn->setPalette(palete);

    auto paleteAppIcon =  appIconBtn->palette();
    paleteAppIcon.setColor(QPalette::Highlight, Qt::transparent);
    paleteAppIcon.setBrush(QPalette::Button, QBrush(QColor(0, 0, 0, 0)));
    appIconBtn->setPalette(paleteAppIcon);

    appSlider->setMaximum(100);
    appSlider->setFixedSize(286, 48);
    appIconBtn->adjustSize();
    appIconBtn->setProperty("useIconHighlightEffect", true);
    appIconBtn->setProperty("iconHighlightEffectMode", true);
    appIconBtn->setStyleSheet("QPushButton{background:transparent;border:0px;"
                                                "padding-left:0px;}"
                                                "QPushButton:hover {"
                                                "background-color: #00000000;"
                                                "color: white;}");

    //音量滑动条
    auto appSliderLayout = new QHBoxLayout(subAppwidget);

    subAppwidget->setFixedSize(412, 48);
    appVolumeLabel->setText("0%");

    appSliderLayout->addWidget(appIconBtn);
    appSliderLayout->addSpacing(3);
    appSliderLayout->addWidget(appSlider);
    appSliderLayout->addSpacing(0);
    appSliderLayout->addWidget(appVolumeLabel);
    appSliderLayout->setSpacing(0);
    subAppwidget->setLayout(appSliderLayout);
    appSliderLayout->setContentsMargins(10, 0, 30, 0);

    m_pVlayout->addWidget(m_appVolumeSliderItemMap[0]->getWidget());
//    m_pVlayout->addWidget(subAppwidget);
    // 设置布局的垂直间距以及设置vlayout四周的间距
    m_pVlayout->setSpacing(2);
    m_pVlayout->setContentsMargins(0, 0, 0, 0);
//    m_pDisplayAppVolumeWidget->adjustSize();
    m_pDisplayAppVolumeWidget->resize(412, sinkInputCount*50);
    m_pVlayout->update();
    qDebug() << "add wid..., displayVolumeWidge.size" << m_pDisplayAppVolumeWidget->rect();
}

void Ukui4AppVolumeWidget::removeAppfromWidget(const QString& appName)
{
    auto index = 1;
    --sinkInputCount;
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
    m_pDisplayAppVolumeWidget->resize(404, sinkInputCount*50);
    m_pVlayout->update();
    qDebug() << "remove wid..., displayVolumeWidge.size" << m_pDisplayAppVolumeWidget->rect();
}
