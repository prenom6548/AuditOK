#include "separation-line-test.h"

#include <QApplication>
#include <QTest>

#include "separation-line.h"

void SeparationLineTest::testInitialGeometryAndStyle()
{
    SeparationLine line;

    QCOMPARE(line.lineWidth(), 0);
    QCOMPARE(line.minimumHeight(), 1);
    QCOMPARE(line.maximumHeight(), 1);
    QVERIFY(line.styleSheet().contains(QStringLiteral("QFrame")));
}

void SeparationLineTest::testSetLineStyleKeepsStyleSheetValid()
{
    SeparationLine line;
    line.setStyleSheet(QString());

    line.setLineStyle();

    QVERIFY(line.styleSheet().contains(QStringLiteral("background")));
}

void SeparationLineTest::testPaletteChangedTriggersLineStyleUpdate()
{
    SeparationLine line;
    line.setStyleSheet(QString());

    Q_EMIT qApp->paletteChanged(qApp->palette());
    QVERIFY(line.styleSheet().contains(QStringLiteral("background")));
}

QTEST_MAIN(SeparationLineTest)
