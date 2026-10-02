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
#include "../../../src/libappdata/app-category-model.h"
#include "../../../src/libappdata/basic-app-filter-model.h"

using namespace UkuiMenu;

namespace {

QStandardItem *createAppItem(const QString &name,
                             const QString &category,
                             const QString &firstLetter)
{
    auto *item = new QStandardItem(name);
    item->setData(name, DataEntity::Name);
    item->setData(category, DataEntity::Category);
    item->setData(firstLetter, DataEntity::FirstLetter);
    return item;
}

}

namespace UkuiMenu {

BasicAppFilterModel *BasicAppFilterModel::instance()
{
    static BasicAppFilterModel model;
    return &model;
}

const AppDatabaseInterface *BasicAppFilterModel::databaseInterface() const
{
    return nullptr;
}

DataEntity BasicAppFilterModel::appOfIndex(int row) const
{
    if (!sourceModel()) {
        return {};
    }

    DataEntity app;
    const QModelIndex idx = index(row, 0);
    app.setName(data(idx, DataEntity::Name).toString());
    app.setCategory(data(idx, DataEntity::Category).toString());
    app.setFirstLetter(data(idx, DataEntity::FirstLetter).toString());
    return app;
}

int BasicAppFilterModel::indexOfApp(const QString &appid) const
{
    Q_UNUSED(appid);
    return -1;
}

bool BasicAppFilterModel::getAppById(const QString &appid, DataEntity &app) const
{
    Q_UNUSED(appid);
    Q_UNUSED(app);
    return false;
}

QVariant BasicAppFilterModel::data(const QModelIndex &index, int role) const
{
    if (!sourceModel()) {
        return {};
    }

    return QSortFilterProxyModel::data(index, role);
}

bool BasicAppFilterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!sourceModel()) {
        return false;
    }

    return QSortFilterProxyModel::setData(index, value, role);
}

bool BasicAppFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_row);
    Q_UNUSED(source_parent);
    return true;
}

BasicAppFilterModel::BasicAppFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

}

class appCategoryModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void test_defaultMode_hidesAllRows();
    void test_data_invalidIndex_returnsEmptyVariant();
    void test_firstLetterMode_groupRoleFormatsSectionKey();
    void test_categoryMode_groupRoleMapsMainAndAdditionalCategories();
    void test_categoryMode_sortByCategoryThenFirstLetter();

private:
    QStandardItemModel m_sourceModel;
    AppCategoryModel m_model;
};

void appCategoryModel_test::init()
{
    m_sourceModel.clear();
    m_sourceModel.setColumnCount(1);
    BasicAppFilterModel::instance()->setSourceModel(&m_sourceModel);
    m_model.setMode(AppCategoryModel::All);
}

void appCategoryModel_test::test_defaultMode_hidesAllRows()
{
    m_sourceModel.appendRow(createAppItem(QStringLiteral("Alpha"),
                                          QStringLiteral("Office;Spreadsheet"),
                                          QStringLiteral("alpha")));

    QCOMPARE(m_model.mode(), AppCategoryModel::All);
    QCOMPARE(m_model.rowCount(), 0);
}

void appCategoryModel_test::test_data_invalidIndex_returnsEmptyVariant()
{
    QCOMPARE(m_model.data(QModelIndex(), DataEntity::Group), QVariant());
}

void appCategoryModel_test::test_firstLetterMode_groupRoleFormatsSectionKey()
{
    m_sourceModel.appendRow(createAppItem(QStringLiteral("Alpha"),
                                          QStringLiteral("Office;Spreadsheet"),
                                          QStringLiteral("alpha")));
    m_sourceModel.appendRow(createAppItem(QStringLiteral("Digit"),
                                          QStringLiteral("Utility;Calculator"),
                                          QStringLiteral("9calc")));
    m_sourceModel.appendRow(createAppItem(QStringLiteral("Symbol"),
                                          QStringLiteral("Other"),
                                          QStringLiteral("?mark")));

    m_model.setMode(AppCategoryModel::FirstLatter);

    QCOMPARE(m_model.rowCount(), 3);
    QCOMPARE(m_model.data(m_model.index(0, 0), DataEntity::Group).toString(), QStringLiteral("#"));
    QCOMPARE(m_model.data(m_model.index(1, 0), DataEntity::Group).toString(), QStringLiteral("&"));
    QCOMPARE(m_model.data(m_model.index(2, 0), DataEntity::Group).toString(), QStringLiteral("A"));
}

void appCategoryModel_test::test_categoryMode_groupRoleMapsMainAndAdditionalCategories()
{
    m_sourceModel.appendRow(createAppItem(QStringLiteral("Calendar"),
                                          QStringLiteral("Office;Calendar"),
                                          QStringLiteral("calendar")));
    m_sourceModel.appendRow(createAppItem(QStringLiteral("Composer"),
                                          QStringLiteral("Music"),
                                          QStringLiteral("composer")));
    m_sourceModel.appendRow(createAppItem(QStringLiteral("Unknown"),
                                          QStringLiteral("CustomCategory"),
                                          QStringLiteral("unknown")));

    m_model.setMode(AppCategoryModel::Category);

    QCOMPARE(m_model.rowCount(), 3);
    QCOMPARE(m_model.data(m_model.index(0, 0), DataEntity::Group).toString(), QStringLiteral("AudioVideo"));
    QCOMPARE(m_model.data(m_model.index(1, 0), DataEntity::Group).toString(), QStringLiteral("Office"));
    QCOMPARE(m_model.data(m_model.index(2, 0), DataEntity::Group).toString(), QStringLiteral("Other"));
}

void appCategoryModel_test::test_categoryMode_sortByCategoryThenFirstLetter()
{
    m_sourceModel.appendRow(createAppItem(QStringLiteral("zeta-office"),
                                          QStringLiteral("Office;Spreadsheet"),
                                          QStringLiteral("zeta")));
    m_sourceModel.appendRow(createAppItem(QStringLiteral("alpha-office"),
                                          QStringLiteral("Office;Calendar"),
                                          QStringLiteral("alpha")));
    m_sourceModel.appendRow(createAppItem(QStringLiteral("beta-utility"),
                                          QStringLiteral("Utility;TextEditor"),
                                          QStringLiteral("beta")));
    m_sourceModel.appendRow(createAppItem(QStringLiteral("gamma-other"),
                                          QStringLiteral("CustomCategory"),
                                          QStringLiteral("gamma")));

    m_model.setMode(AppCategoryModel::Category);

    QCOMPARE(m_model.rowCount(), 4);
    QCOMPARE(m_model.data(m_model.index(0, 0), DataEntity::Name).toString(), QStringLiteral("alpha-office"));
    QCOMPARE(m_model.data(m_model.index(1, 0), DataEntity::Name).toString(), QStringLiteral("zeta-office"));
    QCOMPARE(m_model.data(m_model.index(2, 0), DataEntity::Name).toString(), QStringLiteral("beta-utility"));
    QCOMPARE(m_model.data(m_model.index(3, 0), DataEntity::Name).toString(), QStringLiteral("gamma-other"));
}

QTEST_MAIN(appCategoryModel_test)

#include "app-category-model-test.moc"
