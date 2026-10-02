#include "search-line-edit-test.h"

#include <QFocusEvent>
#include <QImage>
#include <QPainter>
#include <QSignalSpy>
#include <QStyleOption>
#include <QTest>

#define protected public
#include "search-line-edit.h"
#undef protected

using namespace UkuiSearch;

void SearchLineEditTest::testSearchLineEditEmitsImmediateThenDebounce()
{
    SearchLineEdit lineEdit;
    QSignalSpy spy(&lineEdit, SIGNAL(requestSearchKeyword(QString)));
    QVERIFY(spy.isValid());

    lineEdit.setText("a");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QString("a"));

    lineEdit.setText("ab");
    QTest::qWait(150);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QString("ab"));

    lineEdit.clear();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QString(""));
}

void SearchLineEditTest::testSearchLineEditPaintFocusAndRadiusPaths()
{
    SearchLineEdit lineEdit(6);
    lineEdit.resize(120, 36);
    lineEdit.setText(QStringLiteral("paint"));
    lineEdit.setRadius(18);
    Q_EMIT GlobalSettings::getInstance().valueChanged(TRANSPARENCY_KEY, 0.8);

    QImage target(lineEdit.size(), QImage::Format_ARGB32_Premultiplied);
    target.fill(Qt::transparent);
    lineEdit.render(&target);

    QFocusEvent outEvent(QEvent::FocusOut);
    lineEdit.focusOutEvent(&outEvent);

    QVERIFY(!target.isNull());
    QCOMPARE(dynamic_cast<LineEditStyle *>(lineEdit.style()) != nullptr, true);
}

void SearchLineEditTest::testSearchBarWidgetSetTextAndClear()
{
    SearchBarWidget widget;

    widget.setText("keyword");
    QCOMPARE(widget.text(), QString("keyword"));

    widget.clear();
    QCOMPARE(widget.text(), QString(""));
}

void SearchLineEditTest::testSearchBarWidgetReSearchEmitsCurrentText()
{
    SearchBarWidget widget;
    QSignalSpy spy(&widget, SIGNAL(requestSearchKeyword(QString)));
    QVERIFY(spy.isValid());

    widget.setText("ukui");
    QTest::qWait(150);
    spy.clear();

    widget.reSearch();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QString("ukui"));
}

void SearchLineEditTest::testSearchBarWidgetPaintAndSettingsSignal()
{
    SearchBarWidget widget;
    widget.resize(160, 70);

    QImage target(widget.size(), QImage::Format_ARGB32_Premultiplied);
    target.fill(Qt::transparent);
    widget.render(&target);

    Q_EMIT GlobalSettings::getInstance().valueChanged(WINDOW_RADIUS_KEY, 4);
    widget.setText(QStringLiteral("after-radius-change"));

    QCOMPARE(widget.text(), QStringLiteral("after-radius-change"));
    QVERIFY(!target.isNull());
}

void SearchLineEditTest::testLineEditStyleDrawPrimitiveBranches()
{
    LineEditStyle style(9);
    QImage target(40, 24, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&target);
    QStyleOptionFrame option;
    option.rect = QRect(0, 0, 40, 24);
    option.palette = QApplication::palette();

    target.fill(Qt::transparent);
    option.state = QStyle::State_None;
    style.drawPrimitive(QStyle::PE_PanelLineEdit, &option, &painter);

    target.fill(Qt::transparent);
    option.state = QStyle::State_Enabled | QStyle::State_ReadOnly;
    style.drawPrimitive(QStyle::PE_PanelLineEdit, &option, &painter);

    target.fill(Qt::transparent);
    option.state = QStyle::State_Enabled;
    style.drawPrimitive(QStyle::PE_PanelLineEdit, &option, &painter);

    target.fill(Qt::transparent);
    option.state = QStyle::State_Enabled | QStyle::State_HasFocus;
    style.drawPrimitive(QStyle::PE_PanelLineEdit, &option, &painter);

    QStyleOption invalidOption;
    invalidOption.rect = option.rect;
    style.drawPrimitive(QStyle::PE_PanelLineEdit, &invalidOption, &painter);
    style.drawPrimitive(QStyle::PE_Frame, &option, &painter);
    style.setRadius(3);

    QVERIFY(painter.isActive());
}

QTEST_MAIN(SearchLineEditTest)
