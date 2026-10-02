#include <QtTest>
#include <QDBusConnection>

#define private public
#define protected public
#include "plugin/item-group-model.h"
#undef protected
#undef private

class itemGroupModel_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void test_basicPropertyAndBoundary();
    void test_invokableCallsWithInvalidIndex();
};

void itemGroupModel_test::initTestCase()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session DBus is not available.");
    }
}

void itemGroupModel_test::test_basicPropertyAndBoundary()
{
    ItemGroupModel model;

    model.setModelType(ItemGroupModel::Show);
    QCOMPARE(model.getModelType(), ItemGroupModel::Show);
    QCOMPARE(model.groupBegin(), 0);

    model.setCurrentSeparateIndex(2);
    QCOMPARE(model.currentSeparateIndex(), 2);

    model.setCurrentSeparateIndex(-1);
    QCOMPARE(model.currentSeparateIndex(), 2);

    model.setShowCount(-3);
    QCOMPARE(model.showCount(), 0);
    QCOMPARE(model.currentSeparateIndex(), 0);

    model.setShowCount(3);
    QCOMPARE(model.showCount(), 3);

    model.setModelType(ItemGroupModel::Fold);
    QCOMPARE(model.getModelType(), ItemGroupModel::Fold);
    QVERIFY(model.groupBegin() >= 1);
    QVERIFY(model.groupEnd() >= -1);

    const QVariant fold = model.foldIcon();
    QVERIFY(fold.isValid());

    const bool acceptNegative = model.filterAcceptsRow(-1, QModelIndex());
    QCOMPARE(acceptNegative, false);
}

void itemGroupModel_test::test_invokableCallsWithInvalidIndex()
{
    ItemGroupModel model;
    model.onActivate(QModelIndex());
    model.onContextMenu(QModelIndex(), nullptr);

    const QVariant value = model.data(QModelIndex(), Qt::DisplayRole);
    QCOMPARE(value.isValid(), false);
}

QTEST_MAIN(itemGroupModel_test)

#include "itemGroupModelTest.moc"
