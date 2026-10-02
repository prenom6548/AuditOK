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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "commontooltip.h"
#include <QBitmap>
#include <QPainter>
#include "theme.h"
#include <QVBoxLayout>

CommonToolTip::CommonToolTip(QWidget *parent) : kdk::KBallonTip(parent)
{

    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
    this->setTipType(kdk::TipType::Nothing);  //设置KBallonTip类型
}

void CommonToolTip::showThisToolTip()
{
    this->setTipTime(2000);  //设置显示时间
    this->showInfo();  //定时显示KBallonTip，随后隐退   默认为1s
}

