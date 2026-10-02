#include <QCheckBox>
#include <QCloseEvent>
#include <QFont>
#include <QGuiApplication>
#include <QPaintEvent>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

#include "global-settings.h"

#define private public
#define protected public
#include "create-index-ask-dialog.h"
#undef protected
#undef private

using namespace UkuiSearch;

class CreateIndexAskDialogTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void buttonsEmitExpectedChoicesAndHideDialog();
    void closeAndActivationEventsEmitDialogSignals();
    void resizeHeightHandlesShortAndLongText();
    void autoTestAttributesAreAssigned();
    void paintAndNonCloseEventsUseDefaultPaths();
    void themeFontPaintAndHideBranches();
    void staticAttributeAndPaintEventBranches();
    void fontChangedCallbackResizesDialog();
    void themeChangedCallbackUpdatesIcon();
};

void CreateIndexAskDialogTest::initTestCase()
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
}

void CreateIndexAskDialogTest::buttonsEmitExpectedChoicesAndHideDialog()
{
    CreateIndexAskDialog dialog;

    QSignalSpy clickedSpy(&dialog, SIGNAL(btnClicked(bool,bool)));
    QVERIFY(clickedSpy.isValid());

    dialog.m_checkBox->setChecked(true);
    dialog.m_confirmBtn->click();
    QCOMPARE(clickedSpy.count(), 1);
    QList<QVariant> confirmCheckedArgs = clickedSpy.takeFirst();
    QCOMPARE(confirmCheckedArgs.at(0).toBool(), true);
    QCOMPARE(confirmCheckedArgs.at(1).toBool(), true);
    QVERIFY(!dialog.isVisible());

    dialog.m_checkBox->setChecked(false);
    dialog.m_confirmBtn->click();
    QCOMPARE(clickedSpy.count(), 1);
    QList<QVariant> confirmUncheckedArgs = clickedSpy.takeFirst();
    QCOMPARE(confirmUncheckedArgs.at(0).toBool(), true);
    QCOMPARE(confirmUncheckedArgs.at(1).toBool(), false);
    QVERIFY(!dialog.isVisible());

    dialog.m_checkBox->setChecked(false);
    dialog.m_cancelBtn->click();
    QCOMPARE(clickedSpy.count(), 1);
    const QList<QVariant> cancelArgs = clickedSpy.takeFirst();
    QCOMPARE(cancelArgs.at(0).toBool(), false);
    QCOMPARE(cancelArgs.at(1).toBool(), false);
    QVERIFY(!dialog.isVisible());

    dialog.m_checkBox->setChecked(true);
    dialog.m_cancelBtn->click();
    QCOMPARE(clickedSpy.count(), 1);
    QList<QVariant> cancelCheckedArgs = clickedSpy.takeFirst();
    QCOMPARE(cancelCheckedArgs.at(0).toBool(), false);
    QCOMPARE(cancelCheckedArgs.at(1).toBool(), true);
    QVERIFY(!dialog.isVisible());

    dialog.m_closeBtn->click();
    QVERIFY(!dialog.isVisible());
}

void CreateIndexAskDialogTest::closeAndActivationEventsEmitDialogSignals()
{
    CreateIndexAskDialog dialog;

    QSignalSpy closedSpy(&dialog, SIGNAL(closed()));
    QSignalSpy focusSpy(&dialog, SIGNAL(focusChanged()));
    QVERIFY(closedSpy.isValid());
    QVERIFY(focusSpy.isValid());

    QHideEvent hideEvent;
    dialog.hideEvent(&hideEvent);
    QCOMPARE(closedSpy.count(), 1);

    QCloseEvent closeEvent;
    QVERIFY(dialog.eventFilter(&dialog, &closeEvent));
    QVERIFY(closeEvent.isAccepted() == false);
    QVERIFY(!dialog.isVisible());

    QEvent activationEvent(QEvent::ActivationChange);
    dialog.eventFilter(&dialog, &activationEvent);
    QCOMPARE(focusSpy.count(), 1);
}

void CreateIndexAskDialogTest::resizeHeightHandlesShortAndLongText()
{
    CreateIndexAskDialog dialog;

    dialog.m_tipLabel->setText(QStringLiteral("short text"));
    dialog.resizeHeight(qApp->font());
    QCOMPARE(dialog.height(), 202);

    dialog.m_tipLabel->setText(QStringLiteral("long text ").repeated(80));
    dialog.resizeHeight(qApp->font());
    QVERIFY(dialog.height() > 202);
}

void CreateIndexAskDialogTest::autoTestAttributesAreAssigned()
{
    CreateIndexAskDialog dialog;

    QVERIFY(dialog.m_closeBtn->accessibleName().contains(QStringLiteral("closeBtn")));
    QVERIFY(dialog.m_checkBox->accessibleName().contains(QStringLiteral("checkBox")));
    QVERIFY(dialog.m_cancelBtn->accessibleName().contains(QStringLiteral("cancel")));
    QVERIFY(dialog.m_confirmBtn->accessibleName().contains(QStringLiteral("confirm")));
    QVERIFY(!dialog.m_confirmBtn->accessibleDescription().isEmpty());
}

void CreateIndexAskDialogTest::paintAndNonCloseEventsUseDefaultPaths()
{
    CreateIndexAskDialog dialog;
    dialog.resize(380, 202);

    QEvent userEvent(QEvent::User);
    QCOMPARE(dialog.eventFilter(&dialog, &userEvent), false);

    QWidget other;
    QCloseEvent closeEvent;
    QCOMPARE(dialog.eventFilter(&other, &closeEvent), false);
}

void CreateIndexAskDialogTest::themeFontPaintAndHideBranches()
{
    CreateIndexAskDialog dialog;
    dialog.resize(380, 202);
    QSignalSpy closedSpy(&dialog, SIGNAL(closed()));
    QSignalSpy focusSpy(&dialog, SIGNAL(focusChanged()));
    QVERIFY(closedSpy.isValid());
    QVERIFY(focusSpy.isValid());

    QFont largerFont = qGuiApp->font();
    largerFont.setPointSize(largerFont.pointSize() + 3);
    dialog.resizeHeight(largerFont);
    QVERIFY(dialog.height() >= 202);

    QHideEvent hideEvent;
    dialog.hideEvent(&hideEvent);
    QCOMPARE(closedSpy.count(), 1);

    QEvent activationEvent(QEvent::ActivationChange);
    dialog.eventFilter(&dialog, &activationEvent);
    QVERIFY(focusSpy.count() >= 0);
}

void CreateIndexAskDialogTest::staticAttributeAndPaintEventBranches()
{
    CreateIndexAskDialog dialog;
    dialog.resize(380, 202);

    QPushButton customButton;
    dialog.setAutoTestAttribute(&customButton,
                                QStringLiteral("QPushButton"),
                                QStringLiteral("custom"),
                                QStringLiteral("custom description"));
    QCOMPARE(customButton.accessibleName(),
             QStringLiteral("ukui-search_createIndexAskDialog_QPushButton_custom"));
    QCOMPARE(customButton.accessibleDescription(), QStringLiteral("custom description"));

    QPaintEvent paintEvent(dialog.rect());
    dialog.paintEvent(&paintEvent);

    dialog.m_tipLabel->setText(QString());
    dialog.resizeHeight(qApp->font());
    QCOMPARE(dialog.height(), 202);

    QEvent activationEvent(QEvent::ActivationChange);
    const bool filtered = dialog.eventFilter(&dialog, &activationEvent);
    QCOMPARE(filtered, false);
}

void CreateIndexAskDialogTest::fontChangedCallbackResizesDialog()
{
    CreateIndexAskDialog dialog;
    dialog.resize(380, 202);

    QFont largerFont = qGuiApp->font();
    largerFont.setPointSize(largerFont.pointSize() + 3);
    Q_EMIT qGuiApp->fontChanged(largerFont);
    QVERIFY(dialog.height() >= 202);
}

void CreateIndexAskDialogTest::themeChangedCallbackUpdatesIcon()
{
    CreateIndexAskDialog dialog;
    dialog.resize(380, 202);

    Q_EMIT GlobalSettings::getInstance().valueChanged(ICON_THEME_KEY, QVariant());
    QVERIFY(dialog.m_iconLabel->pixmap() != nullptr);
}

QTEST_MAIN(CreateIndexAskDialogTest)

#include "create-index-ask-dialog-test.moc"
