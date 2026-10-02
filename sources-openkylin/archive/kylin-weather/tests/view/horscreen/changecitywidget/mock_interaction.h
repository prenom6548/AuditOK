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
#ifndef SIMPLE_INTERACTION_H
#define SIMPLE_INTERACTION_H

#include <QObject>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include "controller/Interaction.h"

class SimpleInteraction : public Interaction
{
public:
    SimpleInteraction();

    // 实现Interaction类的纯虚函数
    void creatCore(const QStringList &list) override;
    void changeCity(QString cityCode) override;
    void searchResult(QString inputText) override;
    QStandardItemModel *getCityModel() override;
    QStandardItemModel *getTitleModel() override;
    QStandardItemModel *getCollectModel() override;
    QStandardItemModel *getBottomCollectModel() override;
    void changeCityFromSearch(QModelIndex index) override;
    void addCollectCity(QModelIndex index) override;
    void changeCityFromCollect(QModelIndex index) override;
    void delCollectCity(QModelIndex index) override;
    void test() override;
    void loadCollectCity() override;
    void showCenterAfter() override;
    void closePrograme() override;
    void allowLocationYes() override;
    void allowLocationNo() override;
    void refresh() override;
    void addCollectCityFromWidget(QString cityId) override;
    void delCollectCityFromWidget(QString cityId) override;
    void changeCityFromTitleSearch(QString cityId) override;
    void getLAndRCityInfo(QString cityId) override;
};

#endif // SIMPLE_INTERACTION_H