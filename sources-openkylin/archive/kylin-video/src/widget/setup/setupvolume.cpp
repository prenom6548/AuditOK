/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "setupvolume.h"

#include "global/global.h"
#include "../checkbox.h"

#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

using namespace Global;

SetupVolume::SetupVolume(QWidget *parent) :
    ThemeWidget(parent)
{
    initLayout();

    // 全局音量和标准化功能先隐藏
    m_title2Label->hide();
    m_globalVolumeCheckBox->hide();
    m_standerdVolumeCheckBox->hide();

    QStringList items;
    //  音频输出默认先设置这几个，具体怎么拿到硬件输出后面需要加上
    // items << "pulse" << "alsa";
    items << "pulse";
    m_audioOutputDriver->addItems(items);

    initConnect();
    connectThemeSetting();
}

SetupVolume::~SetupVolume()
{
}

void SetupVolume::initData()
{
    // 初始化输出设备
    QString audio_out = g_config->audioOut.first;
    m_audioOutputDriver->setCurrentText(audio_out);

    m_globalVolumeCheckBox->setChecked(g_config->globalVolume.first);
    m_standerdVolumeCheckBox->setChecked(g_config->standardVolume.first);
}

void SetupVolume::setBlackTheme()
{
    m_title1Label->setStyleSheet(QString("color:rgb(255,255,255);"));
    m_title2Label->setStyleSheet(QString("color:rgb(255,255,255);"));

//    m_audioOutputDriver->setStyleSheet(QString("color:rgb(249,249,249);background-color:rgb(64,64,64);"));
    m_globalVolumeCheckBox->setStyleSheet(QString("color:rgb(249,249,249);"));
    m_standerdVolumeCheckBox->setStyleSheet(QString("color:rgb(249,249,249);"));
}

void SetupVolume::setDefaultTheme()
{
    m_title1Label->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_title2Label->setStyleSheet(QString("color:rgb(38,38,38);"));

//    m_audioOutputDriver->setStyleSheet(QString("color:rgb(38,38,38);background-color:rgb(240,240,240);"));
    m_globalVolumeCheckBox->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_standerdVolumeCheckBox->setStyleSheet(QString("color:rgb(38,38,38);"));
}

void SetupVolume::setWidgetFont(QString family, int size)
{
    QFont f(family);
    f.setPointSize(size + 2);
    m_title1Label->setFont(f);
    m_title2Label->setFont(f);
    f.setPointSize(size);
    m_audioOutputDriver->setFont(f);
    m_globalVolumeCheckBox->setFont(f);
    m_standerdVolumeCheckBox->setFont(f);
}

void SetupVolume::initLayout()
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(30, 10, 8, 8);
    lay->setSpacing(12);

    m_title1Label = new QLabel(tr("Output driver"));
    m_title1Label->setAccessibleName("kylin-video_QLabel_m_titleLabel");
    m_title1Label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lay->addWidget(m_title1Label);

    {
        m_hboxLayout = new QHBoxLayout;
        m_hboxLayout->setObjectName("kylin-video_QHBoxLayout_m_hboxLayout");
        m_hboxLayout->setContentsMargins(0, 0, 15, 0);
        m_hboxLayout->setSpacing(8);

        m_audioOutputDriver = new QComboBox;
        m_audioOutputDriver->setAccessibleName("kylin-video_setupvolume_QComboBox_m_audioOutputDriver");
        m_hboxLayout->addWidget(m_audioOutputDriver);

        lay->addLayout(m_hboxLayout);
    }

    m_title2Label = new QLabel(tr("Volume contral"));
    m_title2Label->setAccessibleName("kylin-video_QLabel_m_title2Label");
    m_title2Label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lay->addWidget(m_title2Label);

    m_globalVolumeCheckBox = new CheckBox;
    m_globalVolumeCheckBox->setAccessibleName("kylin-video_CheckBox_m_globalVolumeCheckBox");
    m_globalVolumeCheckBox->setText(tr("Global volume"));
    lay->addWidget(m_globalVolumeCheckBox);

    m_standerdVolumeCheckBox = new CheckBox;
    m_standerdVolumeCheckBox->setAccessibleName("kylin-video_CheckBox_m_standerdVolumeCheckBox");
    m_standerdVolumeCheckBox->setText(tr("Default volume standardization"));
    lay->addWidget(m_standerdVolumeCheckBox);

    lay->addStretch();
}

void SetupVolume::initConnect()
{
    connect(m_audioOutputDriver, &QComboBox::currentTextChanged, [&](QString dev){g_config->audioOut.second = dev;});
    connect(m_globalVolumeCheckBox, &QCheckBox::toggled, [&](bool checked){g_config->globalVolume.second = checked;});
    connect(m_standerdVolumeCheckBox, &QCheckBox::toggled, [&](bool checked){g_config->standardVolume.second = checked;});
}
