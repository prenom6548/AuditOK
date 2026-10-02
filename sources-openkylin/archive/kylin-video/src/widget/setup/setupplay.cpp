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
#include "setupplay.h"

#include "global/global.h"
#include "../checkbox.h"

#include <QVBoxLayout>

using namespace Global;

SetupPlay::SetupPlay(QWidget *parent) :
    ThemeWidget(parent)
{
    initLayout();

    QFont f("Noto Sans CJK SC Regular");
    f.setPixelSize(14);
    m_autoFullScreenCheckBox->setFont(f);
    m_autoFullScreenCheckBox->hide();

    m_clearListExitCheckBox->setFont(f);
    m_lastPosPlayCheckBox->setFont(f);
    m_findAssociatedPlayCheckBox->setFont(f);

    m_findAssociatedPlayCheckBox->hide();  // 功能暂时隐藏

    initConnect();
    connectThemeSetting();
}

SetupPlay::~SetupPlay()
{
    delete layout();
}

/** **********************************************
 * 主题颜色修改
 *************************************************/
void SetupPlay::setBlackTheme()
{
    m_autoFullScreenCheckBox->setStyleSheet(QString("color:rgb(249,249,249);"));
    m_clearListExitCheckBox->setStyleSheet(QString("color:rgb(249,249,249);"));
    m_lastPosPlayCheckBox->setStyleSheet(QString("color:rgb(249,249,249);"));
    m_findAssociatedPlayCheckBox->setStyleSheet(QString("color:rgb(249,249,249);"));
}

void SetupPlay::setDefaultTheme()
{
    m_autoFullScreenCheckBox->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_clearListExitCheckBox->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_lastPosPlayCheckBox->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_findAssociatedPlayCheckBox->setStyleSheet(QString("color:rgb(38,38,38);"));
}

void SetupPlay::setWidgetFont(QString family, int size)
{
    QFont f(family);
    f.setPointSize(size);

    m_autoFullScreenCheckBox->setFont(f);
    m_clearListExitCheckBox->setFont(f);
    m_findAssociatedPlayCheckBox->setFont(f);
    m_lastPosPlayCheckBox->setFont(f);
}

void SetupPlay::initData()
{
    m_autoFullScreenCheckBox->setChecked(g_config->fullScreenWhenPlay.first);
    m_clearListExitCheckBox->setChecked(g_config->clearListWhenExit.first);
    m_lastPosPlayCheckBox->setChecked(g_config->playLastPos.first);
    m_findAssociatedPlayCheckBox->setChecked(g_config->playRelationFile.first);
}

void SetupPlay::initConnect()
{
    connect(m_autoFullScreenCheckBox, &QCheckBox::clicked, [&](bool cheched){g_config->fullScreenWhenPlay.second = cheched;});
    connect(m_clearListExitCheckBox, &QCheckBox::clicked, [&](bool cheched){g_config->clearListWhenExit.second = cheched;});
    connect(m_lastPosPlayCheckBox, &QCheckBox::clicked, [&](bool cheched){g_config->playLastPos.second = cheched;});
    connect(m_findAssociatedPlayCheckBox, &QCheckBox::clicked, [&](bool cheched){g_config->playRelationFile.second = cheched;});
}

void SetupPlay::initLayout()
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(30, 10, 8, 8);
    lay->setSpacing(0);

    m_autoFullScreenCheckBox = new CheckBox;
    m_autoFullScreenCheckBox->setAccessibleName("kylin-video_CheckBox_m_autoFullScreenCheckBox");
    m_autoFullScreenCheckBox->setText(tr("Set fullscreen when open video"));
    lay->addWidget(m_autoFullScreenCheckBox);

    m_clearListExitCheckBox = new CheckBox;
    m_clearListExitCheckBox->setAccessibleName("kylin-video_CheckBox_m_clearListExitCheckBox");
    m_clearListExitCheckBox->setText(tr("Clear play list on exit"));
    lay->addWidget(m_clearListExitCheckBox);

    m_lastPosPlayCheckBox = new CheckBox;
    m_lastPosPlayCheckBox->setAccessibleName("kylin-video_CheckBox_m_lastPosPlayCheckBox");
    m_lastPosPlayCheckBox->setText(tr("Automatically plays from where the file was last stopped"));
    lay->addWidget(m_lastPosPlayCheckBox);

    m_findAssociatedPlayCheckBox = new CheckBox;
    m_findAssociatedPlayCheckBox->setAccessibleName("kylin-video_CheckBox_m_findAssociatedPlayCheckBox");
    m_findAssociatedPlayCheckBox->setText(tr("Automatically find associated files to play"));
    lay->addWidget(m_findAssociatedPlayCheckBox);

    lay->addStretch();
}
