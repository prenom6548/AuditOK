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
#ifndef MOCK_INTERACTION_H
#define MOCK_INTERACTION_H

#include <QObject>
#include <QStandardItemModel>
#include <QModelIndex>

// 模拟Interaction类
class MockInteraction : public QObject
{
    Q_OBJECT

public:
    static MockInteraction* getInstance() {
        static MockInteraction instance;
        return &instance;
    }

    QStandardItemModel* getCityModel() {
        static QStandardItemModel model;
        // 添加一些测试数据
        if (model.rowCount() == 0) {
            model.appendRow(new QStandardItem("北京"));
            model.appendRow(new QStandardItem("上海"));
            model.appendRow(new QStandardItem("广州"));
        }
        return &model;
    }
    
    // 添加缺失的方法
    void searchResult(QString inputText) {
        Q_UNUSED(inputText);
        // 模拟搜索结果处理
    }
    
    void addCollectCity(QModelIndex index) {
        Q_UNUSED(index);
        // 模拟添加收藏城市
    }
    
    void changeCityFromSearch(QModelIndex index) {
        Q_UNUSED(index);
        // 模拟从搜索界面切换城市
    }
    
    void changeCityFromCollect(QModelIndex index) {
        Q_UNUSED(index);
        // 模拟从收藏界面切换城市
    }
    
    // 模拟信号函数
    void searchNull() {
        emit searchNullSignal();
    }

signals:
    // 模拟Interaction类的所有信号
    void searchNullSignal();
    void searchUiShow(bool hasCity);

private:
    MockInteraction() {}
};

#endif // MOCK_INTERACTION_H