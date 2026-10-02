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
#include "minimodeshade.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGSettings>
#include <QEvent>

#include "minimodebutton.h"
#include "global/global.h"

using namespace Global;

MiniModeShade::MiniModeShade(QWidget *parent) : FilletWidget(parent)
{
    setColor(QColor(0,0,0,0));
    initLayout();
    initConnect();

    // 根据主题设置样式
    if(g_gsettings->get("style-name").toString() == STYLE_UKUI_LIGHT)
        setLightTheme();
    else
        setBlackTheme();

    connect(g_gsettings, &QGSettings::changed, [&](QString key){
        // 如果不是跟随主题的话直接返回
        if(key == "styleName") {
            if(g_gsettings->get("style-name").toString() == STYLE_UKUI_LIGHT)
                setLightTheme();
            else
                setBlackTheme();
        }
    });

    // 修改播放暂停图标
    connect(g_core_signal, &GlobalCoreSignal::sigStateChange, [this](){
        if (g_playstate == Mpv::Playing) {
            btnPlayPause->resetName("suspend-mini");
            btnPlayPause->setToolTip(tr("pause"));
        }
        else {
            btnPlayPause->resetName("play-mini");
            btnPlayPause->setToolTip(tr("play"));
        }
    });
}

// 主题变化修改按钮样式
void MiniModeShade::setBlackTheme()
{
    btnClose->setBlackTheme();
    btnNormal->setBlackTheme();
    btnPlayPause->setBlackTheme();
    setColor(QColor(0,0,0,0));
    return;
}

void MiniModeShade::setLightTheme()
{
    btnClose->setLightTheme();
    btnNormal->setLightTheme();
    btnPlayPause->setLightTheme();
    setColor(QColor(0,0,0,0));
    return;
}

void MiniModeShade::initLayout()
{
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *hb_top = new QHBoxLayout;
    hb_top->setContentsMargins(7, 7, 7, 7);

    QHBoxLayout *hb_bottom = new QHBoxLayout;
    hb_bottom->setContentsMargins(30, 30, 30, 30);
    hb_bottom->setSpacing(30);

    btnClose = new MiniModeButton("close-mini", QSize(24,24), QSize(11,11));
    btnClose->setAccessibleName("kylin-video_MiniModeButton_btnClose");
    hb_top->addStretch();
    hb_top->addWidget(btnClose);
    btnClose->setToolTip(tr("close"));

    hb_bottom->addStretch();

    btnPlayPause = new MiniModeButton("suspend-mini", QSize(40,40), QSize(17,17));
    btnPlayPause->setAccessibleName("kylin-video_MiniModeButton_btnPlayPause");
    hb_bottom->addWidget(btnPlayPause);

    btnNormal = new MiniModeButton("showmode-mini", QSize(40,40), QSize(17,17));
    btnNormal->setAccessibleName("kylin-video_MiniModeButton_btnNormal");
    hb_bottom->addWidget(btnNormal);
    btnNormal->setToolTip(tr("normal mode"));

    hb_bottom->addStretch();

    vb->addLayout(hb_top);
    vb->addStretch();
    vb->addLayout(hb_bottom);

    btnPlayPause->setCursor(Qt::PointingHandCursor);
    btnNormal->setCursor(Qt::PointingHandCursor);
    btnClose->setCursor(Qt::PointingHandCursor);

    btnPlayPause->hide();
    btnNormal->hide();
    btnClose->hide();
}

void MiniModeShade::initConnect()
{
    connect(btnPlayPause, &QPushButton::clicked, [this](){emit sigPlayPause();});
    connect(btnNormal, &QPushButton::clicked, [this](){emit sigShowNormal();});
    connect(btnClose, &QPushButton::clicked, [this](){emit sigClose();});
}

void MiniModeShade::enterEvent(QEvent *e)
{
    btnPlayPause->show();
    btnNormal->show();
    btnClose->show();

    e->accept();
}

void MiniModeShade::leaveEvent(QEvent *e)
{
    btnPlayPause->hide();
    btnNormal->hide();
    btnClose->hide();
}
