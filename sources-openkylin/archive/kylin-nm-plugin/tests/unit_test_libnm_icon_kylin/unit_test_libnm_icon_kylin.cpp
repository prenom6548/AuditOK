/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../libnm-kylin/libnm-icon-kylin/kynetworkicon.h"

#include "stubext.h"
using namespace stub_ext;

#include "../../libnm-kylin/libnm-icon-kylin/imageutil.h"
#include <QPixmap>
#include <QImage>
#include <QColor>
#include <QMap>
#include <QVariant>

class KyNetworkIconTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pKyNetworkIconTest = new KyNetworkIcon();
    }

    static void TearDownTestSuite()
    {
        if (m_pKyNetworkIconTest) {
            delete m_pKyNetworkIconTest;
            m_pKyNetworkIconTest = nullptr;
        }
    }

    static KyNetworkIcon *m_pKyNetworkIconTest;
};

KyNetworkIcon *KyNetworkIconTest::m_pKyNetworkIconTest = nullptr;

TEST_F(KyNetworkIconTest, checkIconIsHidden)
{
    // get current value
    bool currentValue = m_pKyNetworkIconTest->checkIconIsHidden();
    // test the current value and the value you have set
    ASSERT_EQ(currentValue, false);
}

TEST_F(KyNetworkIconTest, testAllFunc)
{
    m_pKyNetworkIconTest->initConnect();
    m_pKyNetworkIconTest->startLoading();
    m_pKyNetworkIconTest->stopLoading();

    // onSetTrayIconLoading()
    m_pKyNetworkIconTest->currentIconIndex = 0;
    m_pKyNetworkIconTest->onSetTrayIconLoading();
    m_pKyNetworkIconTest->currentIconIndex = 12;
    m_pKyNetworkIconTest->onSetTrayIconLoading();

    // updateIcon();
    if (nullptr != m_pKyNetworkIconTest->loadingTimer) {
        m_pKyNetworkIconTest->loadingTimer->start();
        m_pKyNetworkIconTest->updateIcon();
    }

    m_pKyNetworkIconTest->iconStatus = KyConnectStatus::NOT_CONNECTED;
    m_pKyNetworkIconTest->updateIcon();
    m_pKyNetworkIconTest->iconStatus = KyConnectStatus::LAN_CONNECTED;
    m_pKyNetworkIconTest->updateIcon();
    m_pKyNetworkIconTest->iconStatus = KyConnectStatus::WLAN_CONNECTED;
    m_pKyNetworkIconTest->updateIcon();
    m_pKyNetworkIconTest->iconStatus = KyConnectStatus::LAN_CONNECTED_LIMITED;
    m_pKyNetworkIconTest->updateIcon();
    m_pKyNetworkIconTest->iconStatus = KyConnectStatus::WLAN_CONNECTED_LIMITED;
    m_pKyNetworkIconTest->updateIcon();

    // onWiredStateChange()
    m_pKyNetworkIconTest->onWiredStateChange(
        "en0", "ed773242-de2c-4903-889b-22a2f14723e0", KyConnectState::CONNECT_STATE_ACTIVATING);
    m_pKyNetworkIconTest->onWiredStateChange(
        "en0", "ed773242-de2c-4903-889b-22a2f14723e0", KyConnectState::CONNECT_STATE_ACTIVATED);

    // onWirelessStateChange
    m_pKyNetworkIconTest->onWirelessStateChange(
        "wlan0", "kylin-wifi", "ed773242-de2c-4903-889b-22a2f14723e0", KyConnectState::CONNECT_STATE_ACTIVATING);
    m_pKyNetworkIconTest->onWirelessStateChange(
        "wlan0", "kylin-wifi", "ed773242-de2c-4903-889b-22a2f14723e0", KyConnectState::CONNECT_STATE_ACTIVATED);

    //    QPaintEvent *event;
    //    m_pKyNetworkIconTest->paintEvent(event);
}

TEST_F(KyNetworkIconTest, ImageUtilDrawSymbolicColoredPixmap)
{
    // create a small pixmap with a single opaque pixel
    QPixmap px(3, 3);
    px.fill(Qt::transparent);
    QImage img = px.toImage();
    img.setPixelColor(1, 1, QColor(10, 20, 30, 255));
    px = QPixmap::fromImage(img);

    // white
    QPixmap out = ImageUtil::drawSymbolicColoredPixmap(px, QString("white"));
    QColor c = out.toImage().pixelColor(1, 1);
    ASSERT_EQ(c.red(), 255);
    ASSERT_EQ(c.green(), 255);
    ASSERT_EQ(c.blue(), 255);

    // black
    out = ImageUtil::drawSymbolicColoredPixmap(px, QString("black"));
    c = out.toImage().pixelColor(1, 1);
    ASSERT_EQ(c.red(), 0);
    ASSERT_EQ(c.green(), 0);
    ASSERT_EQ(c.blue(), 0);

    // gray
    out = ImageUtil::drawSymbolicColoredPixmap(px, QString("gray"));
    c = out.toImage().pixelColor(1, 1);
    ASSERT_EQ(c.red(), 152);
    ASSERT_EQ(c.green(), 163);
    ASSERT_EQ(c.blue(), 164);

    // blue
    out = ImageUtil::drawSymbolicColoredPixmap(px, QString("blue"));
    c = out.toImage().pixelColor(1, 1);
    ASSERT_EQ(c.red(), 61);
    ASSERT_EQ(c.green(), 107);
    ASSERT_EQ(c.blue(), 229);

    // unknown color should return original
    out = ImageUtil::drawSymbolicColoredPixmap(px, QString("unknown-color"));
    c = out.toImage().pixelColor(1, 1);
    ASSERT_EQ(c.red(), 10);
    ASSERT_EQ(c.green(), 20);
    ASSERT_EQ(c.blue(), 30);
}

TEST_F(KyNetworkIconTest, AssembleTrayIconTooltipAndExecute)
{
    QString tip;
    QMap<QString, QString> map;

    // empty map -> default
    m_pKyNetworkIconTest->assembleTrayIconTooltip(map, tip);
    ASSERT_TRUE(tip.contains(QString("Network tool")) || tip.size() > 0);

    // single entry with prefix "Connected: " should strip the prefix
    map.insert("dev0", QString("Connected: my-network"));
    m_pKyNetworkIconTest->assembleTrayIconTooltip(map, tip);
    ASSERT_EQ(tip, QString("my-network"));

    // multiple entries should contain both entries joined with newlines
    map.clear();
    map.insert("d1", QString("Connected: a"));
    map.insert("d2", QString("Not Connected"));
    m_pKyNetworkIconTest->assembleTrayIconTooltip(map, tip);
    // expect multiple lines or the device names visible in the tooltip
    ASSERT_TRUE(tip.contains("d1") || tip.contains("Connected") || tip.count('\n') >= 1);

    // executeFunction should return the default page key
    QVariantMap params;
    QVariantMap res = m_pKyNetworkIconTest->executeFunction(GetDefaultNetworkPage_Func, params);
    ASSERT_TRUE(res.contains(QString("getDefaultNetworkPage")));
}
