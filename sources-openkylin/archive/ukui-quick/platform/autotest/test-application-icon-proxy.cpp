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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QObject>
#include <QSignalSpy>
#include <QUuid>
#include <QtTest>

#include "application-icon-proxy.h"

using namespace UkuiQuick;

namespace {

QString uniqueMissingAppId(const QString &tag)
{
    return QStringLiteral("ukui-quick-autotest-%1-%2")
        .arg(tag, QUuid::createUuid().toString(QUuid::WithoutBraces));
}

}

class application_icon_proxy_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultConstructorStartsEmpty();
    void constructorStoresParentAndUnknownAppId();
    void setAppIdUpdatesStateAndSignalsOnlyOnChange();
};

void application_icon_proxy_test::defaultConstructorStartsEmpty()
{
    ApplicationIconProxy proxy;

    QCOMPARE(proxy.parent(), nullptr);
    QCOMPARE(proxy.appId(), QString());
    QCOMPARE(proxy.icon(), QString());
}

void application_icon_proxy_test::constructorStoresParentAndUnknownAppId()
{
    QObject parent;
    const QString appId = uniqueMissingAppId(QStringLiteral("ctor"));

    ApplicationIconProxy proxy(appId, &parent);

    QCOMPARE(proxy.parent(), &parent);
    QCOMPARE(proxy.appId(), appId);
    QCOMPARE(proxy.icon(), QString());
}

void application_icon_proxy_test::setAppIdUpdatesStateAndSignalsOnlyOnChange()
{
    ApplicationIconProxy proxy;
    QSignalSpy appIdSpy(&proxy, &ApplicationIconProxy::appIdChanged);
    QSignalSpy iconSpy(&proxy, &ApplicationIconProxy::iconChanged);

    const QString firstAppId = uniqueMissingAppId(QStringLiteral("first"));
    const QString secondAppId = uniqueMissingAppId(QStringLiteral("second"));

    proxy.setAppId(firstAppId);
    QCOMPARE(proxy.appId(), firstAppId);
    QCOMPARE(proxy.icon(), QString());
    QCOMPARE(appIdSpy.count(), 1);
    QCOMPARE(iconSpy.count(), 1);

    proxy.setAppId(firstAppId);
    QCOMPARE(appIdSpy.count(), 1);
    QCOMPARE(iconSpy.count(), 1);

    proxy.setAppId(secondAppId);
    QCOMPARE(proxy.appId(), secondAppId);
    QCOMPARE(proxy.icon(), QString());
    QCOMPARE(appIdSpy.count(), 2);
    QCOMPARE(iconSpy.count(), 2);

    proxy.setAppId(QString());
    QCOMPARE(proxy.appId(), QString());
    QCOMPARE(proxy.icon(), QString());
    QCOMPARE(appIdSpy.count(), 3);
    QCOMPARE(iconSpy.count(), 3);

    proxy.setAppId(QString());
    QCOMPARE(appIdSpy.count(), 3);
    QCOMPARE(iconSpy.count(), 3);
}

QTEST_GUILESS_MAIN(application_icon_proxy_test)

#include "test-application-icon-proxy.moc"
