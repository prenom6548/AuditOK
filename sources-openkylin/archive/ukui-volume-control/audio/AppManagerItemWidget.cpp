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

#include "AppManagerItemWidget.h"

#include <QDebug>
#include <QVBoxLayout>

#include "Util.h"
#include "ukui4.0/Ukui4VolumeSliderItem.h"
#include "ukui4.0/Ukui4SelectComboxItem.h"
#include "Ukui4CustomControl.h"
#include "AppManagerSelectComboxItem.h"
#include "AppManagerSliderItem.h"

AppManagerItemWidget::AppManagerItemWidget(QLabel* label, const QString& name, QWidget* parent)
    : m_pTitileLabel(label),
      m_name(name),
      QWidget(parent)
{
    createItems();
    initUi();
    initSlots();
}

void AppManagerItemWidget::createItems()
{
    for (const auto& [k, v] : m_volumeKeys) {
        std::shared_ptr<IVolumeSlider> slider = std::make_shared<Ukui4VolumeSlider>(new UkmediaVolumeSlider(this));
        IVolumeSliderItem* item = new AppManagerSliderItem(slider, new AutoOmitLabel(v, this), new QLabel(this), new QPushButton(this), k, this);

        m_volumeSliderItemMap.emplace(k, item);
    }

    for (const auto& [k, v] : m_selectKeys) {
        auto* comboBox = new QComboBox(this);
        comboBox->setAccessibleName(v.at(0));
        std::shared_ptr<ISelectBox> box = std::make_shared<Ukui4SelectCombox>(comboBox);
        ISelectComboxItem* item = nullptr;

        item = new AppSelectComboxWithDescItem(box, new AutoOmitLabel(v.at(0)), new AutoOmitLabel(v.at(1)), this);
        m_selectComboxItemMap.emplace(k, item);
    }
}

void AppManagerItemWidget::initUi()
{
    auto labelLayout = new QHBoxLayout;
    m_pTitileLabel->setFixedSize(480, 28);
    labelLayout->setSpacing(0);
    labelLayout->setContentsMargins(0, 0, 0, 0);
    labelLayout->addSpacing(41);
    labelLayout->addWidget(m_pTitileLabel);
    labelLayout->addSpacing(39);

    auto itemWidget = new QWidget();
    itemWidget->setContentsMargins(0, 0, 0, 0);
    itemWidget->setFixedSize(512, 300);

    auto itemVLayout = new QVBoxLayout(itemWidget);
    itemVLayout->setSpacing(0);
    itemVLayout->setContentsMargins(0, 0, 0, 0);
    itemVLayout->addWidget(m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_SINKINPUT]->getWidget());
    itemVLayout->addWidget(m_selectComboxItemMap[NodeType::OUTPUT]->getWidget());
    itemVLayout->addWidget(m_selectComboxItemMap[NodeType::INPUT]->getWidget());
    itemVLayout->addStretch();
    itemWidget->setLayout(itemVLayout);

    QHBoxLayout *itemHlayout = new QHBoxLayout();
    itemHlayout->setSpacing(0);
    itemHlayout->setContentsMargins(0, 0, 0, 0);
    itemHlayout->addSpacing(25);
    itemHlayout->addWidget(itemWidget);
    itemHlayout->addSpacing(23);

    m_pConfirmBtn = new QPushButton(tr("Confirm"), this);
    m_pConfirmBtn->setFixedSize(96, 36);
    m_pConfirmBtn->setProperty("isImportant", true);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addStretch();
    hLayout->addWidget(m_pConfirmBtn);
    hLayout->addSpacing(25);

    auto vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addLayout(labelLayout);
    vLayout->addSpacing(10);
    vLayout->addLayout(itemHlayout);
    vLayout->addStretch();
    vLayout->addLayout(hLayout);
    vLayout->addSpacing(40);

    setLayout(vLayout);
}

void AppManagerItemWidget::initSlots()
{
    connect(m_pConfirmBtn, &QPushButton::clicked, this, &AppManagerItemWidget::closeWindowSlots);
}

void AppManagerItemWidget::closeWindowSlots()
{
    qDebug() << QString("App manager item confirm btn clicked, close window");
    Q_EMIT closeWindowSignals();
}

void AppManagerItemWidget::setRange(const VolumeSliderType& type, int min, int max)
{
    m_volumeSliderItemMap[type]->setRange(min, max);
}

void AppManagerItemWidget::setValue(const VolumeSliderType& type, int value)
{
    m_volumeSliderItemMap[type]->setValue(value);
}

void AppManagerItemWidget::setMuted(const VolumeSliderType& type, bool muted)
{
    m_volumeSliderItemMap[type]->setMute(muted);
}

void AppManagerItemWidget::setCurrentIndex(const NodeType& type, int idx)
{
    m_selectComboxItemMap[type]->setCurrentIndex(idx);
}

int AppManagerItemWidget::findData(const NodeType& type, const QVariant& value)
{
    return m_selectComboxItemMap[type]->findData(value);
}

void AppManagerItemWidget::insertItem(const NodeType& type, const QString& text, const QVariant& userData)
{
    m_selectComboxItemMap[type]->insertItem(text, userData);
}

void AppManagerItemWidget::removeItem(const NodeType& type, int idx)
{
    m_selectComboxItemMap[type]->removeItem(idx);
}

void AppManagerItemWidget::setEnabled(const NodeType& type, bool enable)
{
    m_selectComboxItemMap[type]->setEnabled(enable);
}

IVolumeSliderItem* AppManagerItemWidget::getSliderItem() const
{
    return m_volumeSliderItemMap.at(VolumeSliderType::VOLUME_SLIDER_SINKINPUT);
}

std::unordered_map<NodeType, ISelectComboxItem*> AppManagerItemWidget::getSelectComboxItems() const
{
    return m_selectComboxItemMap;
}
