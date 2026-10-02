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
#include "mock_interaction.h"

Interaction *Interaction::m_interaction = nullptr;

Interaction::~Interaction() {}

Interaction *Interaction::getInstance()
{
    if (m_interaction == nullptr) {
        m_interaction = new SimpleInteraction;
    }
    return m_interaction;
}

SimpleInteraction::SimpleInteraction()
{
}

void SimpleInteraction::creatCore(const QStringList &list)
{
    Q_UNUSED(list);
}

void SimpleInteraction::changeCity(QString cityCode)
{
    Q_UNUSED(cityCode);
}

void SimpleInteraction::searchResult(QString inputText)
{
    Q_UNUSED(inputText);
}

QStandardItemModel *SimpleInteraction::getCityModel()
{
    return nullptr;
}

QStandardItemModel *SimpleInteraction::getTitleModel()
{
    return nullptr;
}

QStandardItemModel *SimpleInteraction::getCollectModel()
{
    return nullptr;
}

QStandardItemModel *SimpleInteraction::getBottomCollectModel()
{
    return nullptr;
}

void SimpleInteraction::changeCityFromSearch(QModelIndex index)
{
    Q_UNUSED(index);
}

void SimpleInteraction::addCollectCity(QModelIndex index)
{
    Q_UNUSED(index);
}

void SimpleInteraction::changeCityFromCollect(QModelIndex index)
{
    Q_UNUSED(index);
}

void SimpleInteraction::delCollectCity(QModelIndex index)
{
    Q_UNUSED(index);
}

void SimpleInteraction::test()
{
}

void SimpleInteraction::loadCollectCity()
{
}

void SimpleInteraction::showCenterAfter()
{
}

void SimpleInteraction::closePrograme()
{
}

void SimpleInteraction::allowLocationYes()
{
}

void SimpleInteraction::allowLocationNo()
{
}

void SimpleInteraction::refresh()
{
}

void SimpleInteraction::addCollectCityFromWidget(QString cityId)
{
    Q_UNUSED(cityId);
}

void SimpleInteraction::delCollectCityFromWidget(QString cityId)
{
    Q_UNUSED(cityId);
}

void SimpleInteraction::changeCityFromTitleSearch(QString cityId)
{
    Q_UNUSED(cityId);
}

void SimpleInteraction::getLAndRCityInfo(QString cityId)
{
    Q_UNUSED(cityId);
}