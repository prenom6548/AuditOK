/*
 * Qt5-UKUI Unit Test for System Settings
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
#include <QFontDatabase>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>

class TestSystemSettings : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testMainWindowCreation();
    void testFontDatabase();
    void testComboBoxProperty();
    void testSliderProperty();
    void testSpinBoxProperty();

private:
    QMainWindow *m_window;
};

void TestSystemSettings::initTestCase()
{
}

void TestSystemSettings::cleanupTestCase()
{
}

void TestSystemSettings::init()
{
    m_window = new QMainWindow();
}

void TestSystemSettings::cleanup()
{
    delete m_window;
}

void TestSystemSettings::testMainWindowCreation()
{
    QVERIFY(m_window != nullptr);
    QVERIFY(m_window->isVisible() == false);
}

void TestSystemSettings::testFontDatabase()
{
    // Test that font database is accessible
    QFontDatabase db;
    QStringList families = db.families();
    QVERIFY(families.size() > 0);
    
    // Test that we can query font info
    QStringList styles = db.styles("Sans");
    QVERIFY(styles.size() >= 0);
    
    // Test that we can get point sizes
    QList<int> sizes = db.pointSizes("Sans");
    QVERIFY(sizes.size() >= 0);
}

void TestSystemSettings::testComboBoxProperty()
{
    QComboBox comboBox;
    
    // Test adding items
    comboBox.addItem("Item 1");
    comboBox.addItem("Item 2");
    comboBox.addItem("Item 3");
    
    QVERIFY(comboBox.count() == 3);
    QVERIFY(comboBox.currentText() == "Item 1");
    
    // Test changing current index
    comboBox.setCurrentIndex(1);
    QVERIFY(comboBox.currentIndex() == 1);
    QVERIFY(comboBox.currentText() == "Item 2");
    
    // Test clear
    comboBox.clear();
    QVERIFY(comboBox.count() == 0);
}

void TestSystemSettings::testSliderProperty()
{
    QSlider slider(Qt::Horizontal);
    
    // Test default range
    QVERIFY(slider.minimum() == 0);
    QVERIFY(slider.maximum() == 99);
    
    // Test setting range
    slider.setRange(0, 100);
    QVERIFY(slider.minimum() == 0);
    QVERIFY(slider.maximum() == 100);
    
    // Test setting value
    slider.setValue(50);
    QVERIFY(slider.value() == 50);
    
    // Test vertical slider
    QSlider vSlider(Qt::Vertical);
    QVERIFY(vSlider.orientation() == Qt::Vertical);
}

void TestSystemSettings::testSpinBoxProperty()
{
    QSpinBox spinBox;
    
    // Test default range
    QVERIFY(spinBox.minimum() == 0);
    QVERIFY(spinBox.maximum() == 99);
    
    // Test setting range
    spinBox.setRange(0, 100);
    QVERIFY(spinBox.minimum() == 0);
    QVERIFY(spinBox.maximum() == 100);
    
    // Test setting value
    spinBox.setValue(50);
    QVERIFY(spinBox.value() == 50);
    
    // Test step
    spinBox.setSingleStep(5);
    spinBox.setValue(10);
    spinBox.stepUp();
    QVERIFY(spinBox.value() == 15);
    spinBox.stepDown();
    QVERIFY(spinBox.value() == 10);
}

QTEST_MAIN(TestSystemSettings)

#include "test_system-settings.moc"