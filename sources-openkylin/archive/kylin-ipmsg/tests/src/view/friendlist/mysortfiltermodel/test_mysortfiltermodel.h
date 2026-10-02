/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef TEST_MYSORTFILTERMODEL_H
#define TEST_MYSORTFILTERMODEL_H

#include <gtest/gtest.h>
#include "view/friendlist/mysortfiltermodel.h"
#include <QStandardItemModel>

// 包含被测类
#include "../../../../../src/view/friendlist/mysortfiltermodel.h"

class TestableMySortFilterModel : public MySortFilterModel {
public:
    using MySortFilterModel::lessThan;
    using MySortFilterModel::filterAcceptsRow;
};

class TestMySortFilterModel : public ::testing::Test
{
protected:
    void SetUp() override {
        sourceModel = new QStandardItemModel();
        proxyModel = new TestableMySortFilterModel();
        proxyModel->setSourceModel(sourceModel);
    }

    void TearDown() override {
        delete proxyModel;
        delete sourceModel;
    }

    QStandardItemModel *sourceModel;
    TestableMySortFilterModel *proxyModel;
};

#endif // TEST_MYSORTFILTERMODEL_H
