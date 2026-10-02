#include <QtTest>
#include <QDBusConnection>

#define private public
#define protected public
#include "plugin/items-filter-model.h"
#include "plugin/item-group-model.h"
#undef protected
#undef private

class itemsFilterModel_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void test_basicPath();
    void test_orderApiWithInvalidIndex();
};

void itemsFilterModel_test::initTestCase()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session DBus is not available.");
    }
}

void itemsFilterModel_test::test_basicPath()
{
    ItemsFilterModel *model = ItemsFilterModel::instance();
    QVERIFY(model != nullptr);

    const QVariant value = model->data(QModelIndex(), Qt::DisplayRole);
    QCOMPARE(value.isValid(), false);

    model->activate(QModelIndex());
    model->showContextMenu(QModelIndex(), nullptr);

    model->setSeparateIndex(9999);
    const int current = model->getSeparateIndex();
    QVERIFY(current <= model->rowCount() - 1);

    (void)model->filterAcceptsRow(-1, QModelIndex());
}

void itemsFilterModel_test::test_orderApiWithInvalidIndex()
{
    ItemsFilterModel *model = ItemsFilterModel::instance();
    QVERIFY(model != nullptr);

    ItemGroupModel showGroup;
    showGroup.setModelType(ItemGroupModel::Show);
    ItemGroupModel foldGroup;
    foldGroup.setModelType(ItemGroupModel::Fold);

    model->setOrderInGroup(&showGroup, QModelIndex(), 0);
    model->setOrderBetweenGroups(&showGroup, QModelIndex(), &foldGroup, 0);
}

QTEST_MAIN(itemsFilterModel_test)

#include "itemsFilterModelTest.moc"
