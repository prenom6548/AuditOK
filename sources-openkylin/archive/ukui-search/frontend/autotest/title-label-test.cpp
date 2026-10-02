#include "title-label-test.h"

#include <QApplication>
#include <QPaintEvent>
#include <QSignalSpy>
#include <QTest>

#define protected public
#include "show-more-label.h"
#include "title-label.h"
#undef protected

#include "global-settings.h"

using namespace UkuiSearch;

namespace {
ShowMoreLabel *showMoreLabel(TitleLabel &label)
{
    return label.findChild<ShowMoreLabel *>();
}
}

void TitleLabelTest::testShowMoreVisibilityFollowsListLength()
{
    TitleLabel label(QStringLiteral("File"));
    auto *showMore = showMoreLabel(label);
    QVERIFY(showMore != nullptr);

    QVERIFY(!showMore->isVisible());
    label.onListLengthChanged(5);
    QVERIFY(!showMore->isVisible());

    label.show();
    QVERIFY(QTest::qWaitForWindowExposed(&label));
    label.onListLengthChanged(6);
    QVERIFY(showMore->isVisible());
}

void TitleLabelTest::testSetVisibleForcesExpandedState()
{
    TitleLabel label(QStringLiteral("File"));
    auto *showMore = showMoreLabel(label);
    QVERIFY(showMore != nullptr);

    label.show();
    QVERIFY(QTest::qWaitForWindowExposed(&label));
    label.setShowMoreLableVisible();

    QVERIFY(showMore->isVisible());
    QVERIFY(showMore->getExpanded());
}

void TitleLabelTest::testSignalsForwardedFromShowMoreLabel()
{
    TitleLabel label(QStringLiteral("File"));
    auto *showMore = showMoreLabel(label);
    QVERIFY(showMore != nullptr);
    auto *icon = showMore->findChild<QLabel *>();
    QVERIFY(icon != nullptr);

    QSignalSpy showMoreSpy(&label, SIGNAL(showMoreClicked()));
    QSignalSpy retractSpy(&label, SIGNAL(retractClicked()));
    QVERIFY(showMoreSpy.isValid());
    QVERIFY(retractSpy.isValid());

    QTest::mouseClick(icon, Qt::LeftButton);
    QCOMPARE(showMoreSpy.count(), 1);
    QCOMPARE(retractSpy.count(), 0);

    QTest::mouseClick(icon, Qt::LeftButton);
    QCOMPARE(showMoreSpy.count(), 1);
    QCOMPARE(retractSpy.count(), 1);
}

void TitleLabelTest::testLabelResetCollapsesShowMoreLabel()
{
    TitleLabel label(QStringLiteral("File"));
    auto *showMore = showMoreLabel(label);
    QVERIFY(showMore != nullptr);

    label.setShowMoreLableVisible();
    QVERIFY(showMore->getExpanded());

    label.labelReset();
    QVERIFY(!showMore->getExpanded());
}

void TitleLabelTest::testPaintAndThemeChangeBranches()
{
    qApp->setProperty("kradius-normal", 9);
    TitleLabel label(QStringLiteral("File"));
    label.resize(180, 24);
    label.show();
    QVERIFY(QTest::qWaitForWindowExposed(&label));

    QPaintEvent paint(label.rect());
    label.paintEvent(&paint);

    qApp->setProperty("kradius-normal", 13);
    Q_EMIT GlobalSettings::getInstance().valueChanged(STYLE_NAME_KEY, QStringLiteral("ukui-light"));
    label.paintEvent(&paint);

    Q_EMIT GlobalSettings::getInstance().valueChanged(QStringLiteral("unrelated-key"), QVariant());
    label.paintEvent(&paint);
}

QTEST_MAIN(TitleLabelTest)
