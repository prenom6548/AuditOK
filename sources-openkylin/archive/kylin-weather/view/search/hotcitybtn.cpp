/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include "hotcitybtn.h"

HotcityBtn::HotcityBtn(QWidget *parent, QString name, QString code) : QPushButton(parent)
{
    this->resize(ViewVar::HOT_BTN);
    this->setText(name);
    this->setFocusPolicy(Qt::NoFocus); //设置焦点类型
    cityCode = code;
    connect(this, &HotcityBtn::clicked, this, &HotcityBtn::onHotCityClick);
    QPalette pal = this->palette();
    pal.setColor(QPalette::Button, Qt::transparent);
    this->setPalette(pal);
}

void HotcityBtn::onHotCityClick()
{
    emit hotCityClick(cityCode);
}
