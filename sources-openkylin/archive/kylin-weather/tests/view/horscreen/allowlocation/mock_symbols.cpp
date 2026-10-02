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
#include "global/corevar.h"
#include "global/viewvar.h"
#include "windowmanage.hpp"
#include "ukuiwindowhelper.h"

// 提供CoreVar类的模拟实现
namespace {
QVariant settingsValue;
}

QStringList ViewVar::platFormList = QStringList();
QString ViewVar::platForm = "";

void CoreVar::setSettings(const QString&, const QVariant& value) {
    settingsValue = value;
}

QVariant CoreVar::getSettings(const QString&) {
    return settingsValue;
}

// 模拟Interaction类的部分实现
// 使用前向声明和命名空间来避免冲突
namespace Mock {
    class Interaction : public ::Interaction {
    public:
        static Mock::Interaction* getInstance() {
            static Mock::Interaction instance;
            return &instance;
        }

        // 实现纯虚函数
        virtual void creatCore(const QStringList &) override {}
        virtual void changeCity(QString) override {}
        virtual void searchResult(QString) override {}
        virtual QStandardItemModel* getCityModel() override { return nullptr; }
        virtual QStandardItemModel* getTitleModel() override { return nullptr; }
        virtual QStandardItemModel* getCollectModel() override { return nullptr; }
        virtual QStandardItemModel* getBottomCollectModel() override { return nullptr; }
        virtual void changeCityFromSearch(QModelIndex) override {}
        virtual void addCollectCity(QModelIndex) override {}
        virtual void changeCityFromCollect(QModelIndex) override {}
        virtual void delCollectCity(QModelIndex) override {}
        virtual void test() override {}
        virtual void loadCollectCity() override {}
        virtual void showCenterAfter() override {}
        virtual void closePrograme() override {}
        virtual void allowLocationYes() override {}
        virtual void allowLocationNo() override {}
        virtual void refresh() override {}
        virtual void addCollectCityFromWidget(QString) override {}
        virtual void delCollectCityFromWidget(QString) override {}
        virtual void changeCityFromTitleSearch(QString) override {}
        virtual void getLAndRCityInfo(QString) override {}

    private:
        Interaction() {}
    };
}

Interaction::~Interaction() {}

// 重定向getInstance函数
Interaction* Interaction::getInstance() {
    return Mock::Interaction::getInstance();
}