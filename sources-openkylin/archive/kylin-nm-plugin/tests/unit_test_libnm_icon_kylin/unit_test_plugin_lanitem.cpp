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
#include <QDebug>

#include "../../plugins/netconnect/lanitem.h"

#include "stubext.h"
using namespace stub_ext;

class LanItemTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pLanItemTest = new LanItem(false);
    }

    static void TearDownTestSuite()
    {
        if (m_pLanItemTest) {
            delete m_pLanItemTest;
            m_pLanItemTest = nullptr;
        }
    }

    static LanItem *m_pLanItemTest;
};

LanItem *LanItemTest::m_pLanItemTest = nullptr;

TEST_F(LanItemTest, testAllFunc)
{
    m_pLanItemTest->setName("lan1");

    QString strLan = m_pLanItemTest->getName();
    qDebug() << Q_FUNC_INFO << __LINE__ << strLan;

    QIcon icon("");
    m_pLanItemTest->setItemIcon(icon);
    m_pLanItemTest->setStatus(true);

    bool status = m_pLanItemTest->getStatus();
    qDebug() << Q_FUNC_INFO << __LINE__ << status;

    bool loadStatus = m_pLanItemTest->getIsLoading();
    qDebug() << Q_FUNC_INFO << __LINE__ << loadStatus;

    QString strUuid = m_pLanItemTest->getUuid();
    qDebug() << Q_FUNC_INFO << __LINE__ << strUuid;

    m_pLanItemTest->setUuid(strUuid);

    QString strPath = m_pLanItemTest->getPath();
    qDebug() << Q_FUNC_INFO << __LINE__ << strPath;

    m_pLanItemTest->setPath(strPath);
    m_pLanItemTest->startLoading();
    m_pLanItemTest->stopLoading();

    //    QMouseEvent *p_mouseEvent;
    //    m_pLanItemTest->mousePressEvent(p_mouseEvent);
    //    m_pLanItemTest->mouseReleaseEvent(p_mouseEvent);

    //    QEvent *p_event;
    //    m_pLanItemTest->enterEvent(p_event);
    //    m_pLanItemTest->leaveEvent(p_event);

    //    QPaintEvent *p_paintEvent;
    //    m_pLanItemTest->paintEvent(p_paintEvent);

    m_pLanItemTest->updateIcon();
}
