/*
 * Qt5-UKUI Unit Test for Region Blur
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include <QTest>
#include <QMainWindow>
#include <QRegion>
#include <QRect>

class TestRegionBlur : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testUseSystemStyleBlurProperty();
    void testBlurRegionProperty();
    void testTranslucentBackground();

private:
    QMainWindow *m_window;
};

void TestRegionBlur::initTestCase()
{
}

void TestRegionBlur::cleanupTestCase()
{
}

void TestRegionBlur::init()
{
    m_window = new QMainWindow();
}

void TestRegionBlur::cleanup()
{
    delete m_window;
}

void TestRegionBlur::testUseSystemStyleBlurProperty()
{
    // Test default value
    QVERIFY(m_window->property("useSystemStyleBlur").isValid() == false || 
            m_window->property("useSystemStyleBlur").toBool() == false);
    
    // Test setting to true
    m_window->setProperty("useSystemStyleBlur", true);
    QVERIFY(m_window->property("useSystemStyleBlur").toBool() == true);
    
    // Test setting to false
    m_window->setProperty("useSystemStyleBlur", false);
    QVERIFY(m_window->property("useSystemStyleBlur").toBool() == false);
}

void TestRegionBlur::testBlurRegionProperty()
{
    // Test setting blur region
    QRegion region(QRect(0, 0, 100, 100));
    m_window->setProperty("blurRegion", region);
    QVERIFY(m_window->property("blurRegion").value<QRegion>() == region);
    
    // Test with complex region
    QRegion complexRegion;
    complexRegion += QRect(0, 0, 50, 50);
    complexRegion += QRect(100, 100, 50, 50);
    complexRegion += QRect(200, 200, 50, 50);
    m_window->setProperty("blurRegion", complexRegion);
    QVERIFY(m_window->property("blurRegion").value<QRegion>() == complexRegion);
}

void TestRegionBlur::testTranslucentBackground()
{
    // Test setting translucent background
    m_window->setAttribute(Qt::WA_TranslucentBackground, true);
    QVERIFY(m_window->testAttribute(Qt::WA_TranslucentBackground) == true);
    
    // Test removing translucent background
    m_window->setAttribute(Qt::WA_TranslucentBackground, false);
    QVERIFY(m_window->testAttribute(Qt::WA_TranslucentBackground) == false);
}

QTEST_MAIN(TestRegionBlur)

#include "test_region-blur.moc"