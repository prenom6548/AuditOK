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
#include <QRegularExpression>
#include <QSignalSpy>

#define private public
#include "../../../src/utils/date-time-utils.h"
#undef private

#include "libkydate.h"

using namespace Sidebar;

class dateTimeUtils_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void test_currentTime_24HourFormat();
    void test_currentTime_12HourFormat();
    void test_currentDate_formats();
    void test_currentWeekDay_notEmpty();
    void test_timeSignalSlot_emitsSignals();
    void test_setShortDateFormat_usesSdkValue();
    void test_setShortDateFormat_usesDefaultWhenSdkEmpty();
    void test_setShortDateFormat_ignoresUnchangedValue();
    void test_initGsettings_readsInstalledKeys();
    void test_initGsettings_changedSignalsUpdateFormats();
    void test_initGsettings_missingKeysKeepsDefaults();
    void test_computeTimeOut_now();
    void test_computeTimeOut_sameDay24Hour();
    void test_computeTimeOut_sameDay12Hour();
    void test_computeTimeOut_yesterday();
    void test_computeTimeOut_yesterday12Hour();
    void test_computeTimeOut_withinWeek();
    void test_computeTimeOut_withinWeek24Hour();
    void test_computeTimeOut_olderThanWeek();
    void test_computeTimeOut_futureTime();
    void test_computeTimeOut_invalidDateTimeFallbackFormats();

private:
    DateTimeUtils *m_utils = nullptr;
};

void dateTimeUtils_test::init()
{
    QGSettings::resetTestData();
    kdk_test_set_shortformat("yyyy/MM/dd");
    const QString testName = QString::fromLatin1(QTest::currentTestFunction());
    if (testName.startsWith("test_initGsettings")) {
        const QByteArray schema("org.ukui.control-center.panel.plugins");
        const QStringList keys = testName.contains("missingKeys")
                ? QStringList()
                : (QStringList() << "date" << "hoursystem");
        QGSettings::installSchema(schema, keys);
        QGSettings::setValue(schema, "date", "cn");
        QGSettings::setValue(schema, "hoursystem", "24");
    }
    m_utils = new DateTimeUtils;
    if (m_utils->m_timer) {
        m_utils->m_timer->stop();
    }
}

void dateTimeUtils_test::cleanup()
{
    delete m_utils;
    m_utils = nullptr;
}

void dateTimeUtils_test::test_currentTime_24HourFormat()
{
    m_utils->m_hourSystem = "24";

    const QString text = m_utils->currentTime();

    QVERIFY2(QRegularExpression("^\\d{2}:\\d{2}$").match(text).hasMatch(), qPrintable(text));
}

void dateTimeUtils_test::test_currentTime_12HourFormat()
{
    m_utils->m_hourSystem = "12";

    const QString before = QDateTime::currentDateTime().toString("AP hh:mm");
    const QString text = m_utils->currentTime();
    const QString after = QDateTime::currentDateTime().toString("AP hh:mm");

    QVERIFY(text == before || text == after);
}

void dateTimeUtils_test::test_currentDate_formats()
{
    m_utils->m_dataFormat = "cn";
    QVERIFY(QRegularExpression("^\\d{2}/\\d{2}$").match(m_utils->currentDate()).hasMatch());

    m_utils->m_dataFormat = "en";
    QVERIFY(QRegularExpression("^\\d{2}-\\d{2}$").match(m_utils->currentDate()).hasMatch());
}

void dateTimeUtils_test::test_currentWeekDay_notEmpty()
{
    QVERIFY(!m_utils->currentWeekDay().isEmpty());
}

void dateTimeUtils_test::test_timeSignalSlot_emitsSignals()
{
    QSignalSpy timeSpy(m_utils, &DateTimeUtils::timeUpdate);
    QSignalSpy dateSpy(m_utils, &DateTimeUtils::dateUpdate);
    QSignalSpy weekSpy(m_utils, &DateTimeUtils::weekDayUpdate);
    QSignalSpy refreshSpy(m_utils, &DateTimeUtils::timeRefresh);

    m_utils->timeSignalSlot();

    QCOMPARE(timeSpy.count(), 1);
    QCOMPARE(dateSpy.count(), 1);
    QCOMPARE(weekSpy.count(), 1);
    QCOMPARE(refreshSpy.count(), 1);
    QCOMPARE(timeSpy.takeFirst().at(0).toString(), m_utils->currentTime());
    QCOMPARE(dateSpy.takeFirst().at(0).toString(), m_utils->currentDate());
    QCOMPARE(weekSpy.takeFirst().at(0).toString(), m_utils->currentWeekDay());
}

void dateTimeUtils_test::test_setShortDateFormat_usesSdkValue()
{
    QSignalSpy refreshSpy(m_utils, &DateTimeUtils::timeRefresh);
    kdk_test_set_shortformat("yyyy-MM-dd");

    m_utils->setShortDateFormat();

    QCOMPARE(m_utils->m_shortDateFormat, QString("yyyy-MM-dd"));
    QCOMPARE(refreshSpy.count(), 1);
}

void dateTimeUtils_test::test_setShortDateFormat_usesDefaultWhenSdkEmpty()
{
    m_utils->m_shortDateFormat.clear();
    QSignalSpy refreshSpy(m_utils, &DateTimeUtils::timeRefresh);
    kdk_test_set_shortformat("");

    m_utils->setShortDateFormat();

    QVERIFY(!m_utils->m_shortDateFormat.isEmpty());
    QVERIFY(refreshSpy.count() == 0 || refreshSpy.count() == 1);
}

void dateTimeUtils_test::test_setShortDateFormat_ignoresUnchangedValue()
{
    m_utils->m_shortDateFormat = "yyyy/MM/dd";
    QSignalSpy refreshSpy(m_utils, &DateTimeUtils::timeRefresh);

    m_utils->setShortDateFormat("yyyy/MM/dd");

    QCOMPARE(refreshSpy.count(), 0);
}

void dateTimeUtils_test::test_initGsettings_readsInstalledKeys()
{
    QCOMPARE(m_utils->m_dataFormat, QString("cn"));
    QCOMPARE(m_utils->m_hourSystem, QString("24"));
    QVERIFY(m_utils->m_timeGsettings != nullptr);

    QGSettings::resetTestData();
    const QByteArray schema("org.ukui.control-center.panel.plugins");
    QGSettings::installSchema(schema, QStringList() << "date" << "hoursystem");
    QGSettings::setValue(schema, "date", "en");
    QGSettings::setValue(schema, "hoursystem", "12");

    DateTimeUtils otherUtils;
    if (otherUtils.m_timer) {
        otherUtils.m_timer->stop();
    }

    QCOMPARE(otherUtils.m_dataFormat, QString("en"));
    QCOMPARE(otherUtils.m_hourSystem, QString("12"));
    QVERIFY(otherUtils.currentDate().contains('-'));
    const QString before = QDateTime::currentDateTime().toString("AP hh:mm");
    const QString text = otherUtils.currentTime();
    const QString after = QDateTime::currentDateTime().toString("AP hh:mm");
    QVERIFY(text == before || text == after);
}

void dateTimeUtils_test::test_initGsettings_changedSignalsUpdateFormats()
{
    QVERIFY(m_utils->m_timeGsettings != nullptr);
    const QByteArray schema("org.ukui.control-center.panel.plugins");
    QSignalSpy dateSpy(m_utils, &DateTimeUtils::dateUpdate);
    QSignalSpy timeSpy(m_utils, &DateTimeUtils::timeUpdate);

    QGSettings::setValue(schema, "date", "en");
    m_utils->m_timeGsettings->emitChanged("date");
    QCOMPARE(m_utils->m_dataFormat, QString("en"));
    QCOMPARE(dateSpy.count(), 1);
    QVERIFY(dateSpy.takeFirst().at(0).toString().contains('-'));

    QGSettings::setValue(schema, "hoursystem", "12");
    const QString before = QDateTime::currentDateTime().toString("AP hh:mm");
    m_utils->m_timeGsettings->emitChanged("hoursystem");
    const QString after = QDateTime::currentDateTime().toString("AP hh:mm");
    QCOMPARE(m_utils->m_hourSystem, QString("12"));
    QCOMPARE(timeSpy.count(), 1);
    const QString text = timeSpy.takeFirst().at(0).toString();
    QVERIFY(text == before || text == after);

    m_utils->m_timeGsettings->emitChanged("unrelated");
    QCOMPARE(dateSpy.count(), 0);
    QCOMPARE(timeSpy.count(), 0);
}

void dateTimeUtils_test::test_initGsettings_missingKeysKeepsDefaults()
{
    QVERIFY(m_utils->m_timeGsettings != nullptr);
    QCOMPARE(m_utils->m_dataFormat, QString("cn"));
    QCOMPARE(m_utils->m_hourSystem, QString("24"));
}

void dateTimeUtils_test::test_computeTimeOut_now()
{
    const QDateTime now = QDateTime::currentDateTime().addSecs(-10);

    QCOMPARE(m_utils->computeTimeOut(now), QString("Now"));
}

void dateTimeUtils_test::test_computeTimeOut_sameDay24Hour()
{
    m_utils->m_hourSystem = "24";
    const QDateTime timeStamp = QDateTime::currentDateTime().addSecs(-3600);

    QCOMPARE(m_utils->computeTimeOut(timeStamp), timeStamp.toString("HH:mm"));
}

void dateTimeUtils_test::test_computeTimeOut_sameDay12Hour()
{
    m_utils->m_hourSystem = "12";
    const QDateTime timeStamp = QDateTime::currentDateTime().addSecs(-3600);

    QCOMPARE(m_utils->computeTimeOut(timeStamp), timeStamp.toString("AP h:mm"));
}

void dateTimeUtils_test::test_computeTimeOut_yesterday()
{
    m_utils->m_hourSystem = "24";
    const QDateTime timeStamp = QDateTime::currentDateTime().addDays(-1).addSecs(-120);

    QCOMPARE(m_utils->computeTimeOut(timeStamp), QString("Yesterday ").append(timeStamp.toString("HH:mm")));
}

void dateTimeUtils_test::test_computeTimeOut_yesterday12Hour()
{
    m_utils->m_hourSystem = "12";
    const QDateTime timeStamp = QDateTime::currentDateTime().addDays(-1).addSecs(-120);

    QCOMPARE(m_utils->computeTimeOut(timeStamp), QString("Yesterday ").append(timeStamp.toString("AP h:mm")));
}

void dateTimeUtils_test::test_computeTimeOut_withinWeek()
{
    m_utils->m_hourSystem = "12";
    const QDateTime timeStamp = QDateTime::currentDateTime().addDays(-3).addSecs(-120);

    QCOMPARE(m_utils->computeTimeOut(timeStamp), timeStamp.toString("ddd AP h:mm"));
}

void dateTimeUtils_test::test_computeTimeOut_withinWeek24Hour()
{
    m_utils->m_hourSystem = "24";
    const QDateTime timeStamp = QDateTime::currentDateTime().addDays(-3).addSecs(-120);

    QCOMPARE(m_utils->computeTimeOut(timeStamp), timeStamp.toString("ddd HH:mm"));
}

void dateTimeUtils_test::test_computeTimeOut_olderThanWeek()
{
    m_utils->m_shortDateFormat = "yyyy-MM-dd";
    const QDateTime timeStamp = QDateTime::currentDateTime().addDays(-8);

    QCOMPARE(m_utils->computeTimeOut(timeStamp), timeStamp.toString("yyyy-MM-dd"));
}

void dateTimeUtils_test::test_computeTimeOut_futureTime()
{
    m_utils->m_shortDateFormat = "yyyy/MM/dd";
    const QDateTime timeStamp = QDateTime::currentDateTime().addDays(2);

    QCOMPARE(m_utils->computeTimeOut(timeStamp), timeStamp.toString("yyyy/MM/dd"));
}

void dateTimeUtils_test::test_computeTimeOut_invalidDateTimeFallbackFormats()
{
    const QDateTime invalid;
    m_utils->m_shortDateFormat = "yyyy-MM-dd";

    m_utils->m_hourSystem = "24";
    QCOMPARE(m_utils->computeTimeOut(invalid), invalid.toString("yyyy-MM-dd HH:mm"));

    m_utils->m_hourSystem = "12";
    QCOMPARE(m_utils->computeTimeOut(invalid), invalid.toString("yyyy-MM-dd AP h:mm"));
}

QTEST_MAIN(dateTimeUtils_test)

#include "date-time-utils-test.moc"
