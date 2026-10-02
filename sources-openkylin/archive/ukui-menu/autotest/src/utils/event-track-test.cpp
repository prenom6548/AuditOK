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

#define private public
#include "../../../src/utils/event-track.h"
#undef private

#include <QtTest>

#include "fakes/kysdk/diagnosetest/libkydatacollect.h"

using namespace UkuiMenu;

namespace UkuiMenu {
KCustomProperty* appendCustomProp(KTrackData *data, const QVariantMap &map);
void freeKCustomProperty(KCustomProperty *customProperty, int size);
}

class eventTrack_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_appendCustomProp_returnsNullForEmptyInput();
    void test_appendCustomProp_and_freeKCustomProperty_roundTrip();
    void test_qmlAttachedProperties_returnsSingleton();
    void test_sendClickEvent_withProperties();
    void test_sendDefaultEvent_withoutProperties();
    void test_sendSearchEvent_recordsPayload();
};

void eventTrack_test::init()
{
    FakeKdkDiaStore::reset();
}

void eventTrack_test::test_appendCustomProp_returnsNullForEmptyInput()
{
    QVariantMap emptyMap;

    QVERIFY(appendCustomProp(nullptr, emptyMap) == nullptr);

    KTrackData data { KEVENTSOURCE_DESKTOP, KEVENT_CLICK };
    QVERIFY(appendCustomProp(&data, emptyMap) == nullptr);
}

void eventTrack_test::test_appendCustomProp_and_freeKCustomProperty_roundTrip()
{
    KTrackData data { KEVENTSOURCE_DESKTOP, KEVENT_CLICK };
    QVariantMap map;
    map.insert(QStringLiteral("app"), QStringLiteral("ukui-menu"));
    map.insert(QStringLiteral("count"), 3);

    KCustomProperty *properties = appendCustomProp(&data, map);

    QVERIFY(properties != nullptr);

    QStringList pairs;
    for (int i = 0; i < map.size(); ++i) {
        pairs << QStringLiteral("%1=%2").arg(QString::fromUtf8(properties[i].key),
                                             QString::fromUtf8(properties[i].value));
    }

    QVERIFY(pairs.contains(QStringLiteral("app=ukui-menu")));
    QVERIFY(pairs.contains(QStringLiteral("count=3")));

    freeKCustomProperty(properties, map.size());
}

void eventTrack_test::test_qmlAttachedProperties_returnsSingleton()
{
    QObject object;

    QCOMPARE(EventTrack::qmlAttachedProperties(&object), EventTrack::instance());
    QVERIFY(EventTrack::instance() != nullptr);
}

void eventTrack_test::test_sendClickEvent_withProperties()
{
    QVariantMap map;
    map.insert(QStringLiteral("name"), QStringLiteral("calculator"));
    map.insert(QStringLiteral("position"), QStringLiteral("1"));

    EventTrack track;
    track.sendClickEvent(QStringLiteral("click-app"), QStringLiteral("menu-home"), map);

    const FakeKdkDiaRecord record = FakeKdkDiaStore::lastRecord();
    QCOMPARE(record.initSource, KEVENTSOURCE_DESKTOP);
    QCOMPARE(record.initEventType, KEVENT_CLICK);
    QVERIFY(record.appendCalled);
    QVERIFY(record.uploadDefaultCalled);
    QVERIFY(record.freeCalled);
    QCOMPARE(record.uploadEvent, QStringLiteral("click-app"));
    QCOMPARE(record.uploadPage, QStringLiteral("menu-home"));

    QStringList pairs;
    for (const auto &pair : record.customProperties) {
        pairs << QStringLiteral("%1=%2").arg(pair.first, pair.second);
    }
    QVERIFY(pairs.contains(QStringLiteral("name=calculator")));
    QVERIFY(pairs.contains(QStringLiteral("position=1")));
}

void eventTrack_test::test_sendDefaultEvent_withoutProperties()
{
    EventTrack track;
    track.sendDefaultEvent(QStringLiteral("open-page"), QStringLiteral("favorite"));

    const FakeKdkDiaRecord record = FakeKdkDiaStore::lastRecord();
    QCOMPARE(record.initEventType, KEVENT_CUSTOM);
    QVERIFY(!record.appendCalled);
    QVERIFY(record.uploadDefaultCalled);
    QVERIFY(record.freeCalled);
    QCOMPARE(record.uploadEvent, QStringLiteral("open-page"));
    QCOMPARE(record.uploadPage, QStringLiteral("favorite"));
}

void eventTrack_test::test_sendSearchEvent_recordsPayload()
{
    EventTrack track;
    track.sendSearchEvent(QStringLiteral("search"), QStringLiteral("search-page"), QStringLiteral("terminal"));

    const FakeKdkDiaRecord record = FakeKdkDiaStore::lastRecord();
    QCOMPARE(record.initEventType, KEVENT_SEARCH);
    QVERIFY(record.uploadSearchCalled);
    QVERIFY(record.freeCalled);
    QCOMPARE(record.uploadEvent, QStringLiteral("search"));
    QCOMPARE(record.uploadPage, QStringLiteral("search-page"));
    QCOMPARE(record.uploadContent, QStringLiteral("terminal"));
}

QTEST_MAIN(eventTrack_test)

#include "event-track-test.moc"
