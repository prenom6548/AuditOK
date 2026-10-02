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

#include "../../../src/utils/event-track.h"

extern "C" {
#include "kysdk/diagnosetest/libkydatacollect.h"
}

using namespace Sidebar;

namespace Sidebar {
KCustomProperty *appendCustomProp(KTrackData *data, const QVariantMap &map);
void freeKCustomProperty(KCustomProperty *customProperty, int size);
}

class eventTrack_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_appendCustomProp_returnsNullForInvalidInput();
    void test_appendCustomProp_buildsProperties();
    void test_freeKCustomProperty_acceptsNull();
    void test_sendSlideEvent_withEmptyMap();
    void test_sendSlideEvent_withCustomProperties();
    void test_instance_returnsSingleton();
    void test_qmlAttachedProperties_returnsSingleton();
};

void eventTrack_test::init()
{
    kdk_test_reset_state();
}

void eventTrack_test::test_appendCustomProp_returnsNullForInvalidInput()
{
    QVERIFY(appendCustomProp(nullptr, {}) == nullptr);

    KTrackData data{};
    QVERIFY(appendCustomProp(&data, {}) == nullptr);
}

void eventTrack_test::test_appendCustomProp_buildsProperties()
{
    KTrackData data{};
    QVariantMap map;
    map.insert("code", 42);
    map.insert("page", "home");

    KCustomProperty *properties = appendCustomProp(&data, map);

    QVERIFY(properties != nullptr);
    QCOMPARE(QString(properties[0].key), QString("code"));
    QCOMPARE(QString(properties[0].value), QString("42"));
    QCOMPARE(QString(properties[1].key), QString("page"));
    QCOMPARE(QString(properties[1].value), QString("home"));

    freeKCustomProperty(properties, map.size());
}

void eventTrack_test::test_freeKCustomProperty_acceptsNull()
{
    freeKCustomProperty(nullptr, 0);
}

void eventTrack_test::test_sendSlideEvent_withEmptyMap()
{
    EventTrack track;

    track.sendSlideEvent("slide_code", "main_page");

    QCOMPARE(kdk_test_last_source_type(), int(KEVENTSOURCE_DESKTOP));
    QCOMPARE(kdk_test_last_event_type(), int(KEVENT_CUSTOM));
    QCOMPARE(QString::fromUtf8(kdk_test_last_event_code()), QString("slide_code"));
    QCOMPARE(QString::fromUtf8(kdk_test_last_page_name()), QString("main_page"));
    QCOMPARE(kdk_test_last_property_length(), 0);
    QVERIFY(kdk_test_data_freed());
}

void eventTrack_test::test_sendSlideEvent_withCustomProperties()
{
    EventTrack track;
    QVariantMap map;
    map.insert("module", "sidebar");
    map.insert("value", 7);

    track.sendSlideEvent("custom_code", "detail_page", map);

    QCOMPARE(kdk_test_last_property_length(), map.size());
    QCOMPARE(QString::fromUtf8(kdk_test_property_key(0)), QString("module"));
    QCOMPARE(QString::fromUtf8(kdk_test_property_value(0)), QString("sidebar"));
    QCOMPARE(QString::fromUtf8(kdk_test_property_key(1)), QString("value"));
    QCOMPARE(QString::fromUtf8(kdk_test_property_value(1)), QString("7"));
}

void eventTrack_test::test_instance_returnsSingleton()
{
    QCOMPARE(EventTrack::instance(), EventTrack::instance());
}

void eventTrack_test::test_qmlAttachedProperties_returnsSingleton()
{
    QObject object;

    QCOMPARE(EventTrack::qmlAttachedProperties(&object), EventTrack::instance());
}

QTEST_MAIN(eventTrack_test)

#include "event-track-test.moc"
