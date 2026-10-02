/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 *
 */

#include <QtTest>
#include <QStandardItemModel>

#include "../../../src/data-entity.h"
#include "../../../src/libappdata/combined-list-model.h"

using namespace UkuiMenu;

namespace {

QStandardItem *createCombinedItem(const QString &name, const QString &group)
{
    auto *item = new QStandardItem(name);
    item->setData(name, DataEntity::Name);
    item->setData(group, DataEntity::Group);
    return item;
}

}

class combinedListModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void test_insertSubModel_combinesRowsAndMapsIndexes();
    void test_subModelSignals_updateProxyModel();
    void test_removeSubModel_shrinksRowCount();

private:
    QStandardItemModel m_firstModel;
    QStandardItemModel m_secondModel;
    CombinedListModel *m_model = nullptr;
};

void combinedListModel_test::init()
{
    m_firstModel.clear();
    m_secondModel.clear();
    m_firstModel.setColumnCount(1);
    m_secondModel.setColumnCount(1);
    m_firstModel.appendRow(createCombinedItem(QStringLiteral("Alpha"), QStringLiteral("A")));
    m_firstModel.appendRow(createCombinedItem(QStringLiteral("Beta"), QStringLiteral("B")));
    m_secondModel.appendRow(createCombinedItem(QStringLiteral("Gamma"), QStringLiteral("C")));
    delete m_model;
    m_model = new CombinedListModel(this);
}

void combinedListModel_test::test_insertSubModel_combinesRowsAndMapsIndexes()
{
    m_model->insertSubModel(&m_firstModel);
    m_model->insertSubModel(&m_secondModel);

    QCOMPARE(m_model->subModelCount(), 2);
    QCOMPARE(m_model->rowCount(QModelIndex()), 3);
    QCOMPARE(m_model->data(m_model->index(0, 0, QModelIndex()), DataEntity::Name).toString(), QStringLiteral("Alpha"));
    QCOMPARE(m_model->data(m_model->index(2, 0, QModelIndex()), DataEntity::Name).toString(), QStringLiteral("Gamma"));
    QCOMPARE(m_model->mapToSource(m_model->index(2, 0, QModelIndex())), m_secondModel.index(0, 0));
    QCOMPARE(m_model->mapFromSource(m_firstModel.index(1, 0)).row(), 1);
}

void combinedListModel_test::test_subModelSignals_updateProxyModel()
{
    m_model->insertSubModel(&m_firstModel);
    QSignalSpy insertSpy(m_model, &CombinedListModel::rowsInserted);
    QSignalSpy removeSpy(m_model, &CombinedListModel::rowsRemoved);
    QSignalSpy dataSpy(m_model, &CombinedListModel::dataChanged);

    m_firstModel.appendRow(createCombinedItem(QStringLiteral("Delta"), QStringLiteral("D")));
    QCOMPARE(m_model->rowCount(QModelIndex()), 3);
    QCOMPARE(insertSpy.count(), 1);

    m_firstModel.item(0)->setData(QStringLiteral("Alpha Updated"), DataEntity::Name);
    QVERIFY(dataSpy.count() >= 1);

    m_firstModel.removeRow(1);
    QCOMPARE(m_model->rowCount(QModelIndex()), 2);
    QCOMPARE(removeSpy.count(), 1);
}

void combinedListModel_test::test_removeSubModel_shrinksRowCount()
{
    m_model->insertSubModel(&m_firstModel);
    m_model->insertSubModel(&m_secondModel);

    m_model->removeSubModel(&m_firstModel);

    QCOMPARE(m_model->subModelCount(), 1);
    QCOMPARE(m_model->rowCount(QModelIndex()), 1);
    QCOMPARE(m_model->data(m_model->index(0, 0, QModelIndex()), DataEntity::Name).toString(), QStringLiteral("Gamma"));
}

QTEST_MAIN(combinedListModel_test)

#include "combined-list-model-test.moc"
