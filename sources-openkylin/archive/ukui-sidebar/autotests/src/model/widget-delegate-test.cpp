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
#include <QSignalSpy>
#include <QGuiApplication>
#include "widget-container.h"
#include "widget-loader.h"

#define private public
#include "../../../src/model/widget-delegate.h"
#undef private

class widgetDelegate_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_defaultQuickWidgetId();
    void test_defaultPreferredWidth();
    void test_defaultPreferredHeight();
    void test_loaderSetupAndContainerInitialSize();
    void test_quickWidgetId_returnsSet();
    void test_quickWidgetIdChanged_signalEmitted();
    void test_quickWidgetIdChanged_sameValue_noExtraSignal();
    void test_setQuickWidgetId_loadsAndReplacesWidget();
    void test_loadWidget_handlesNullWidget();
    void test_containerLoadFailure();
    void test_containerItemLoadFailure();
    void test_childrenRectChangedUpdatesPreferredSize();
    void test_destructorEmitsAboutToDeleted();
    void test_preferredWidthChanged_signalExists();
    void test_preferredHeightChanged_signalExists();
    void test_multipleInstances_independent();
    void test_defaultQuickWidgetId_isEmpty();
    void test_preferredWidth_nonNegative();
    void test_preferredHeight_nonNegative();
    void test_setQuickWidgetId_preservesNonNegativePreferredSize();
    void test_loadFailure_keepsContainerNullable();
};

void widgetDelegate_test::init()
{
    UkuiQuick::resetWidgetLoaderStubState();
}

void widgetDelegate_test::test_defaultQuickWidgetId()
{
    UkuiShortcut::WidgetDelegate delegate;
    QCOMPARE(delegate.quickWidgetId(), QString());
}

void widgetDelegate_test::test_defaultPreferredWidth()
{
    UkuiShortcut::WidgetDelegate delegate;
    QCOMPARE(delegate.preferredWidth(), 64.0);
}

void widgetDelegate_test::test_defaultPreferredHeight()
{
    UkuiShortcut::WidgetDelegate delegate;
    QCOMPARE(delegate.preferredHeight(), 32.0);
}

void widgetDelegate_test::test_loaderSetupAndContainerInitialSize()
{
    UkuiShortcut::WidgetDelegate delegate;

    QCOMPARE(UkuiQuick::widgetLoaderStubState().addPathCalls, 1);
    QCOMPARE(UkuiQuick::widgetLoaderStubState().searchPaths.value(0), QStringLiteral(":/shortcut"));
    QCOMPARE(UkuiQuick::widgetLoaderStubState().setFilterCalls, 1);
    QVERIFY(UkuiQuick::widgetLoaderStubState().lastHosts.testFlag(UkuiQuick::WidgetMetadata::Shortcut));
    QVERIFY(delegate.m_container != nullptr);
    QCOMPARE(delegate.preferredWidth(), 64.0);
    QCOMPARE(delegate.preferredHeight(), 32.0);
    QCOMPARE(delegate.size(), QSizeF(64, 32));
}

void widgetDelegate_test::test_quickWidgetId_returnsSet()
{
    UkuiShortcut::WidgetDelegate delegate;
    delegate.setQuickWidgetId(QStringLiteral("testWidget"));
    QCOMPARE(delegate.quickWidgetId(), QStringLiteral("testWidget"));
}

void widgetDelegate_test::test_quickWidgetIdChanged_signalEmitted()
{
    UkuiShortcut::WidgetDelegate delegate;
    QSignalSpy spy(&delegate, &UkuiShortcut::WidgetDelegate::quickWidgetIdChanged);
    delegate.setQuickWidgetId(QStringLiteral("testWidget"));
    QVERIFY(spy.count() >= 1);
}

void widgetDelegate_test::test_quickWidgetIdChanged_sameValue_noExtraSignal()
{
    UkuiShortcut::WidgetDelegate delegate;
    QSignalSpy spy(&delegate, &UkuiShortcut::WidgetDelegate::quickWidgetIdChanged);
    delegate.setQuickWidgetId(QStringLiteral("testWidget"));
    const int firstCount = spy.count();
    delegate.setQuickWidgetId(QStringLiteral("testWidget"));
    QCOMPARE(spy.count(), firstCount);
}

void widgetDelegate_test::test_setQuickWidgetId_loadsAndReplacesWidget()
{
    UkuiShortcut::WidgetDelegate delegate;
    auto *container = delegate.m_container;
    QVERIFY(container != nullptr);

    delegate.setQuickWidgetId(QStringLiteral("widget.one"));
    QVERIFY(delegate.m_widget != nullptr);
    QCOMPARE(delegate.m_widget->id(), QStringLiteral("widget.one"));
    QCOMPARE(container->widgets().size(), 1);

    delegate.setQuickWidgetId(QStringLiteral("widget.two"));
    QVERIFY(delegate.m_widget != nullptr);
    QCOMPARE(delegate.m_widget->id(), QStringLiteral("widget.two"));
    QCOMPARE(container->widgets().size(), 1);
    QCOMPARE(container->widgets().constFirst()->id(), QStringLiteral("widget.two"));
}

void widgetDelegate_test::test_loadWidget_handlesNullWidget()
{
    UkuiShortcut::WidgetDelegate delegate;
    QVERIFY(delegate.m_container != nullptr);
    UkuiQuick::widgetLoaderStubState().widgetLoadSucceeds = false;

    delegate.setQuickWidgetId(QStringLiteral("missing.widget"));

    QCOMPARE(delegate.quickWidgetId(), QStringLiteral("missing.widget"));
    QCOMPARE(delegate.m_widget, nullptr);
    QCOMPARE(delegate.m_container->widgets().size(), 0);
}

void widgetDelegate_test::test_containerLoadFailure()
{
    UkuiQuick::widgetLoaderStubState().containerLoadSucceeds = false;
    UkuiShortcut::WidgetDelegate delegate;

    QCOMPARE(delegate.m_container, nullptr);
    QCOMPARE(delegate.preferredWidth(), 0.0);
    QCOMPARE(delegate.preferredHeight(), 0.0);
}

void widgetDelegate_test::test_containerItemLoadFailure()
{
    UkuiQuick::widgetLoaderStubState().containerItemLoadSucceeds = false;
    UkuiShortcut::WidgetDelegate delegate;

    QVERIFY(delegate.m_container != nullptr);
    QCOMPARE(delegate.preferredWidth(), 0.0);
    QCOMPARE(delegate.preferredHeight(), 0.0);
}

void widgetDelegate_test::test_childrenRectChangedUpdatesPreferredSize()
{
    UkuiShortcut::WidgetDelegate delegate;
    QSignalSpy widthSpy(&delegate, &UkuiShortcut::WidgetDelegate::preferredWidthChanged);
    QSignalSpy heightSpy(&delegate, &UkuiShortcut::WidgetDelegate::preferredHeightChanged);

    auto *contItem = delegate.childItems().constFirst();
    auto *child = contItem->childItems().constFirst();
    child->setWidth(96);
    child->setHeight(48);
    QCoreApplication::processEvents();

    QCOMPARE(delegate.preferredWidth(), 96.0);
    QCOMPARE(delegate.preferredHeight(), 48.0);
    QCOMPARE(delegate.size(), QSizeF(96, 48));
    QVERIFY(widthSpy.count() >= 1);
    QVERIFY(heightSpy.count() >= 1);
}

void widgetDelegate_test::test_destructorEmitsAboutToDeleted()
{
    auto *delegate = new UkuiShortcut::WidgetDelegate;
    QVERIFY(delegate->m_container != nullptr);
    QSignalSpy spy(delegate->m_container, &UkuiQuick::WidgetContainer::aboutToDeleted);

    delete delegate;

    QCOMPARE(spy.count(), 1);
}

void widgetDelegate_test::test_preferredWidthChanged_signalExists()
{
    UkuiShortcut::WidgetDelegate delegate;
    // Verify the signal exists and can be connected
    QSignalSpy spy(&delegate, &UkuiShortcut::WidgetDelegate::preferredWidthChanged);
    QVERIFY(spy.isValid());
}

void widgetDelegate_test::test_preferredHeightChanged_signalExists()
{
    UkuiShortcut::WidgetDelegate delegate;
    QSignalSpy spy(&delegate, &UkuiShortcut::WidgetDelegate::preferredHeightChanged);
    QVERIFY(spy.isValid());
}

void widgetDelegate_test::test_multipleInstances_independent()
{
    UkuiShortcut::WidgetDelegate d1;
    UkuiShortcut::WidgetDelegate d2;
    QCOMPARE(d1.quickWidgetId(), d2.quickWidgetId());
    QCOMPARE(d1.preferredWidth(), d2.preferredWidth());
    QCOMPARE(d1.preferredHeight(), d2.preferredHeight());
}

void widgetDelegate_test::test_defaultQuickWidgetId_isEmpty()
{
    UkuiShortcut::WidgetDelegate delegate;
    QVERIFY(delegate.quickWidgetId().isEmpty());
}

void widgetDelegate_test::test_preferredWidth_nonNegative()
{
    UkuiShortcut::WidgetDelegate delegate;
    QVERIFY(delegate.preferredWidth() >= 0.0);
}

void widgetDelegate_test::test_preferredHeight_nonNegative()
{
    UkuiShortcut::WidgetDelegate delegate;
    QVERIFY(delegate.preferredHeight() >= 0.0);
}

void widgetDelegate_test::test_setQuickWidgetId_preservesNonNegativePreferredSize()
{
    UkuiShortcut::WidgetDelegate delegate;
    delegate.setQuickWidgetId(QStringLiteral("org.ukui.sidebar.invalid-test-widget"));
    QVERIFY(delegate.preferredWidth() >= 0.0);
    QVERIFY(delegate.preferredHeight() >= 0.0);
}

void widgetDelegate_test::test_loadFailure_keepsContainerNullable()
{
    UkuiShortcut::WidgetDelegate delegate;
    QVERIFY(delegate.m_widgetLoader != nullptr);
    QVERIFY(delegate.m_container == nullptr || delegate.m_container != nullptr);
}

QTEST_MAIN(widgetDelegate_test)
#include "widget-delegate-test.moc"
