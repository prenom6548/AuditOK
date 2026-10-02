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
#include "controller/Interaction.h"
#include <QStringList>
#include <QStandardItemModel>

Interaction::~Interaction() {}

class MockInteraction : public Interaction {
public:
    void creatCore(const QStringList&) override {}
    void changeCity(QString) override {}
    void searchResult(QString) override {}
    QStandardItemModel* getCityModel() override { static QStandardItemModel m; return &m; }
    QStandardItemModel* getTitleModel() override { static QStandardItemModel m; return &m; }
    QStandardItemModel* getCollectModel() override { static QStandardItemModel m; return &m; }
    QStandardItemModel* getBottomCollectModel() override { 
        static QStandardItemModel m; 
        if (m.rowCount() == 0) {
            m.appendRow(new QStandardItem("City1"));
        }
        return &m; 
    }
    void changeCityFromSearch(QModelIndex) override {}
    void addCollectCity(QModelIndex) override {}
    void changeCityFromCollect(QModelIndex) override {}
    void delCollectCity(QModelIndex) override {}
    void test() override {}
    void loadCollectCity() override {}
    void showCenterAfter() override {}
    void closePrograme() override {}
    void allowLocationYes() override {}
    void allowLocationNo() override {}
    void refresh() override {}
    void addCollectCityFromWidget(QString) override {}
    void delCollectCityFromWidget(QString) override {}
    void changeCityFromTitleSearch(QString) override {}
    void getLAndRCityInfo(QString) override {}
};

Interaction* Interaction::getInstance() {
    static MockInteraction instance;
    return &instance;
}
