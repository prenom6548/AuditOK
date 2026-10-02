#include "flow-layout-test.h"

#include <QLabel>
#include <QTest>
#include <QWidget>
#include <QWidgetItem>

#include "flow-layout.h"

void FlowLayoutTest::testInitialStateAndSpacing()
{
    FlowLayout layout(0, 7, 9);

    QCOMPARE(layout.count(), 0);
    QCOMPARE(layout.horizontalSpacing(), 7);
    QCOMPARE(layout.verticalSpacing(), 9);
    QVERIFY(layout.hasHeightForWidth());
    QCOMPARE(layout.expandingDirections(), Qt::Orientations());
    QVERIFY(layout.itemAt(0) == nullptr);
    QVERIFY(layout.takeAt(0) == nullptr);
}

void FlowLayoutTest::testAddItemTakeAtAndMinimumSize()
{
    QWidget owner;
    auto *layout = new FlowLayout(&owner, 2, 3, 4);
    auto *first = new QLabel(QStringLiteral("first"));
    auto *second = new QLabel(QStringLiteral("second"));
    first->setFixedSize(30, 10);
    second->setFixedSize(20, 15);

    layout->addWidget(first);
    layout->addWidget(second);

    QCOMPARE(layout->count(), 2);
    QVERIFY(layout->itemAt(0) != nullptr);
    QCOMPARE(layout->minimumSize(), QSize(34, 19));
    QCOMPARE(layout->sizeHint(), layout->minimumSize());

    QLayoutItem *taken = layout->takeAt(0);
    QVERIFY(taken != nullptr);
    QCOMPARE(layout->count(), 1);
    delete taken;
}

void FlowLayoutTest::testHeightForWidthWrapsItems()
{
    QWidget owner;
    auto *layout = new FlowLayout(&owner, 0, 5, 7);
    for (int i = 0; i < 3; ++i) {
        auto *label = new QLabel(QString::number(i));
        label->setFixedSize(20, 10);
        layout->addWidget(label);
    }

    QCOMPARE(layout->heightForWidth(71), 10);
    QCOMPARE(layout->heightForWidth(70), 27);
    QCOMPARE(layout->heightForWidth(45), 44);
}

void FlowLayoutTest::testSetGeometryPositionsWrappedItems()
{
    QWidget owner;
    auto *layout = new FlowLayout(&owner, 0, 5, 7);
    auto *first = new QLabel(QStringLiteral("first"));
    auto *second = new QLabel(QStringLiteral("second"));
    auto *third = new QLabel(QStringLiteral("third"));
    first->setFixedSize(20, 10);
    second->setFixedSize(20, 10);
    third->setFixedSize(20, 10);
    layout->addWidget(first);
    layout->addWidget(second);
    layout->addWidget(third);

    layout->setGeometry(QRect(0, 0, 45, 100));

    QCOMPARE(first->geometry(), QRect(0, 0, 20, 10));
    QCOMPARE(second->geometry(), QRect(0, 17, 20, 10));
    QCOMPARE(third->geometry(), QRect(0, 34, 20, 10));
}

void FlowLayoutTest::testSmartSpacingNullParent()
{
    FlowLayout layout(0, -1, -1);
    QCOMPARE(layout.horizontalSpacing(), -1);
    QCOMPARE(layout.verticalSpacing(), -1);
}

void FlowLayoutTest::testSmartSpacingWidgetParent()
{
    QWidget widget;
    FlowLayout layout(&widget, 0, -1, -1);
    int h = layout.horizontalSpacing();
    QVERIFY(h != -1);
    int v = layout.verticalSpacing();
    QVERIFY(v != -1);
}

void FlowLayoutTest::testDoLayoutWithNegativeSpacing()
{
    FlowLayout layout(0, -1, -1);
    QLabel a("a");
    QLabel b("b");
    a.setFixedSize(20, 10);
    b.setFixedSize(20, 10);
    layout.addItem(new QWidgetItem(&a));
    layout.addItem(new QWidgetItem(&b));

    layout.setGeometry(QRect(0, 0, 100, 100));
    QCOMPARE(a.geometry().topLeft(), QPoint(0, 0));
    QCOMPARE(a.geometry().size(), QSize(20, 10));
}

QTEST_MAIN(FlowLayoutTest)
