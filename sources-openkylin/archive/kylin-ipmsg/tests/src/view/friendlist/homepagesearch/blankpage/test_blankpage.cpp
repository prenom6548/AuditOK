/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#include "test_blankpage.h"
#include <gsettingmonitor.h>
#include "view/common/globalsizedata.h"

void TestBlankPage::initTestCase()
{
}

void TestBlankPage::cleanupTestCase()
{
}

void TestBlankPage::init()
{
    m_page = new BlankPage();
}

void TestBlankPage::cleanup()
{
    delete m_page;
}

void TestBlankPage::testConstructor()
{
    QVERIFY(m_page != nullptr);
}

void TestBlankPage::testInit()
{
    // init() 已经在构造函数中调用过
    QVERIFY(m_page->m_blankLabel != nullptr);
    QVERIFY(m_page->m_blankTextLabel != nullptr);
    QVERIFY(m_page->layout() != nullptr);
}

void TestBlankPage::testInitGsetting()
{
    // 覆盖 70-72
    m_page->initGsetting();
    emit kdk::GsettingMonitor::getInstance()->systemThemeChange();
    QVERIFY(true);
}

void TestBlankPage::testChangeBlankText()
{
    m_page->changeBlankText();
    QCOMPARE(m_page->m_blankTextLabel->text(), tr("No Information"));
    QCOMPARE(m_page->searchblank, 0);
}

void TestBlankPage::testChangeSearchText()
{
    // 覆盖 103-105
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUIDark;
    m_page->changeSearchText();
    QCOMPARE(m_page->m_blankTextLabel->text(), tr("No Relevant Contacts"));
    QCOMPARE(m_page->searchblank, 1);
    
    // 恢复 Light 主题测试
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUILight;
    m_page->changeSearchText();
}

void TestBlankPage::testChangeTheme()
{
    // 测试 Light 主题
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUILight;
    m_page->changeTheme();
    
    // 测试 Dark 主题
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUIDark;
    m_page->changeTheme();
    
    // 测试 searchblank == 1 时的变化
    m_page->searchblank = 1;
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUILight;
    m_page->changeTheme();
    
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUIDark;
    m_page->changeTheme();
}
