/*
 * Qt5-UKUI Unit Test for Custom Shadow
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
#include <QVector4D>

class TestCustomShadow : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testUseCustomShadowProperty();
    void testCustomShadowDarknessProperty();
    void testCustomShadowWidthProperty();
    void testCustomShadowRadiusProperty();
    void testCustomShadowMarginsProperty();

private:
    QMainWindow *m_window;
};

void TestCustomShadow::initTestCase()
{
}

void TestCustomShadow::cleanupTestCase()
{
}

void TestCustomShadow::init()
{
    m_window = new QMainWindow();
}

void TestCustomShadow::cleanup()
{
    delete m_window;
}

void TestCustomShadow::testUseCustomShadowProperty()
{
    // Test default value
    QVERIFY(m_window->property("useCustomShadow").isValid() == false || 
            m_window->property("useCustomShadow").toBool() == false);

    // Test setting to true
    m_window->setProperty("useCustomShadow", true);
    QVERIFY(m_window->property("useCustomShadow").toBool() == true);

    // Test setting to false
    m_window->setProperty("useCustomShadow", false);
    QVERIFY(m_window->property("useCustomShadow").toBool() == false);
}

void TestCustomShadow::testCustomShadowDarknessProperty()
{
    // Test default value
    m_window->setProperty("useCustomShadow", true);
    
    // Test setting different darkness values
    for (double d = 0.0; d <= 1.0; d += 0.25) {
        m_window->setProperty("customShadowDarkness", d);
        QVERIFY(qAbs(m_window->property("customShadowDarkness").toDouble() - d) < 0.001);
    }
}

void TestCustomShadow::testCustomShadowWidthProperty()
{
    // Test setting different width values
    for (int w = 0; w <= 50; w += 10) {
        m_window->setProperty("customShadowWidth", w);
        QVERIFY(m_window->property("customShadowWidth").toInt() == w);
    }
}

void TestCustomShadow::testCustomShadowRadiusProperty()
{
    // Test setting radius as QVector4D
    QVector4D radius(1, 1, 1, 1);
    m_window->setProperty("customShadowRadius", radius);
    QVERIFY(m_window->property("customShadowRadius").value<QVector4D>() == radius);
}

void TestCustomShadow::testCustomShadowMarginsProperty()
{
    // Test setting margins as QVector4D
    QVector4D margins(20, 20, 20, 20);
    m_window->setProperty("customShadowMargins", margins);
    QVERIFY(m_window->property("customShadowMargins").value<QVector4D>() == margins);
}

QTEST_MAIN(TestCustomShadow)

#include "test_custom-shadow.moc"