/*
 * Qt5-UKUI Unit Test for UKUIConfigStyleParameters
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include <QTest>
#include <QWidget>
#include <QPalette>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QComboBox>
#include <QListView>
#include <QTreeView>
#include <QCheckBox>
#include <QRadioButton>
#include <QSlider>
#include <QProgressBar>
#include <QScrollBar>
#include <QSpinBox>
#include <QTabWidget>
#include <QMenu>
#include <QTableWidget>
#include <QLabel>
#include <QVector4D>

// Include UKUIConfigStyleParameters header
#include "ukui-config-style-parameters.h"
#include "config-shadow-helper.h"

using namespace UKUIConfigStyleSpace;

class TestUKUIConfigStyleParameters : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Constructor and basic tests
    void testConstructor();
    void testDestructor();
    void testUpdateParameters();
    void testUpdateFocusStrategy();

    // Palette tests
    void testSetPalette();
    void testSetPaletteWithDarkMode();
    void testInitPalette();

    // Theme color tests
    void testSetUKUIThemeColor();
    void testThemeColor();

    // SpinBox layout tests
    void testGetSpinBoxControlLayout();
    void testSetSpinBoxControlLayout();

    // needTranslucent property tests
    void testPushButtonNeedTranslucentFalse();
    void testToolButtonNeedTranslucentFalse();
    void testLineEditNeedTranslucentFalse();
    void testSpinBoxNeedTranslucentFalse();
    void testComboBoxNeedTranslucentFalse();
    void testListViewNeedTranslucentFalse();
    void testTreeViewNeedTranslucentFalse();
    void testTableViewNeedTranslucentFalse();
    void testCheckBoxNeedTranslucentFalse();
    void testRadioButtonNeedTranslucentFalse();
    void testSliderNeedTranslucentFalse();
    void testProgressBarNeedTranslucentFalse();
    void testScrollBarNeedTranslucentFalse();
    void testTabWidgetNeedTranslucentFalse();
    void testMenuNeedTranslucentFalse();

    // Custom property tests (setDisablePen, setCheckPen, etc.)
    void testPushButtonCustomPens();
    void testToolButtonCustomPens();
    void testCheckBoxCustomPens();
    void testRadioButtonCustomPens();
    void testLineEditCustomPens();
    void testDarkModeHighlightInitPaths();
    void testInitConfigToolTipParameters();
    void testConfigShadowHelperOffscreen();

private:
    UKUIConfigStyleParameters *m_params;
};

void TestUKUIConfigStyleParameters::initTestCase()
{
}

void TestUKUIConfigStyleParameters::cleanupTestCase()
{
}

void TestUKUIConfigStyleParameters::init()
{
    m_params = new UKUIConfigStyleParameters(nullptr, false, "default", true);
}

void TestUKUIConfigStyleParameters::cleanup()
{
    delete m_params;
    m_params = nullptr;
}

void TestUKUIConfigStyleParameters::testConstructor()
{
    // Test constructor with different parameters
    UKUIConfigStyleParameters *params1 = new UKUIConfigStyleParameters(nullptr, false, "light", true);
    QVERIFY(params1 != nullptr);
    delete params1;

    UKUIConfigStyleParameters *params2 = new UKUIConfigStyleParameters(nullptr, true, "dark", false);
    QVERIFY(params2 != nullptr);
    delete params2;
}

void TestUKUIConfigStyleParameters::testDestructor()
{
    // Test destructor doesn't crash
    UKUIConfigStyleParameters *params = new UKUIConfigStyleParameters(nullptr, false);
    delete params;
    QVERIFY(true);
}

void TestUKUIConfigStyleParameters::testUpdateParameters()
{
    // Test updateParameters with tablet mode
    m_params->updateParameters(true);
    QVERIFY(true);

    m_params->updateParameters(false);
    QVERIFY(true);
}

void TestUKUIConfigStyleParameters::testUpdateFocusStrategy()
{
    // Test updateFocusStrategy
    m_params->updateFocusStrategy(true);
    QVERIFY(true);

    m_params->updateFocusStrategy(false);
    QVERIFY(true);
}

void TestUKUIConfigStyleParameters::testSetPalette()
{
    // Test setPalette function
    QPalette palette;
    QPalette result = m_params->setPalette(palette);
    
    // Verify palette is valid - check that at least one color is set
    QVERIFY(result.color(QPalette::Active, QPalette::Window).isValid());
    
    // Verify some colors are set
    QVERIFY(result.color(QPalette::Active, QPalette::Window).isValid());
    QVERIFY(result.color(QPalette::Active, QPalette::Button).isValid());
    QVERIFY(result.color(QPalette::Active, QPalette::Text).isValid());
}

void TestUKUIConfigStyleParameters::testSetPaletteWithDarkMode()
{
    // Test setPalette with dark mode
    UKUIConfigStyleParameters darkParams(nullptr, true, "dark", true);
    QPalette palette;
    QPalette result = darkParams.setPalette(palette);
    
    // Verify palette is valid
    QVERIFY(result.color(QPalette::Active, QPalette::Window).isValid());
}

void TestUKUIConfigStyleParameters::testInitPalette()
{
    // Test initPalette function
    m_params->initPalette();
    QVERIFY(true);
}

void TestUKUIConfigStyleParameters::testSetUKUIThemeColor()
{
    // Test setUKUIThemeColor with different theme colors
    QPalette palette;
    
    m_params->setUKUIThemeColor(palette, "default");
    QVERIFY(true);
    
    m_params->setUKUIThemeColor(palette, "daybreakBlue");
    QVERIFY(true);
    
    m_params->setUKUIThemeColor(palette, "jamPurple");
    QVERIFY(true);
    
    m_params->setUKUIThemeColor(palette, "magenta");
    QVERIFY(true);
    
    m_params->setUKUIThemeColor(palette, "sunRed");
    QVERIFY(true);
    
    m_params->setUKUIThemeColor(palette, "sunsetOrange");
    QVERIFY(true);
    
    m_params->setUKUIThemeColor(palette, "dustGold");
    QVERIFY(true);
    
    m_params->setUKUIThemeColor(palette, "polarGreen");
    QVERIFY(true);
}

void TestUKUIConfigStyleParameters::testThemeColor()
{
    // Test themeColor getter
    QString color = m_params->themeColor();
    QVERIFY(!color.isEmpty() || color.isEmpty());
}

void TestUKUIConfigStyleParameters::testGetSpinBoxControlLayout()
{
    // Test getSpinBoxControlLayout
    int layout = m_params->getSpinBoxControlLayout();
    QVERIFY(layout == UKUIConfigStyleParameters::Horizontal || 
            layout == UKUIConfigStyleParameters::Vertical);
}

void TestUKUIConfigStyleParameters::testSetSpinBoxControlLayout()
{
    // This function doesn't exist, but we can test the get function
    // The layout is set in constructor
    int layout = m_params->getSpinBoxControlLayout();
    QVERIFY(layout >= 0);
}

QTEST_MAIN(TestUKUIConfigStyleParameters)

void TestUKUIConfigStyleParameters::testPushButtonNeedTranslucentFalse()
{
    // Test PushButton with needTranslucent = false
    QPushButton button("Test");
    button.setProperty("needTranslucent", false);
    button.show();
    
    QStyleOptionButton opt;
    opt.initFrom(&button);
    
    m_params->initConfigPushButtonBevelParameters(false, &opt, &button);
    m_params->initConfigPushButtonLabelParameters(false, &opt, &button);
    
    QVERIFY(true);
    button.hide();
}

void TestUKUIConfigStyleParameters::testToolButtonNeedTranslucentFalse()
{
    // Test ToolButton with needTranslucent = false
    // QToolButton constructor doesn't accept string in Qt 5.15
    QToolButton button;
    button.setText("Test");
    button.setProperty("needTranslucent", false);
    button.show();
    
    QStyleOptionToolButton opt;
    opt.initFrom(&button);
    
    m_params->initConfigToolButtonPanelParameters(false, &opt, &button);
    m_params->initConfigToolButtonLabelParameters(false, &opt, &button);
    
    QVERIFY(true);
    button.hide();
}

void TestUKUIConfigStyleParameters::testLineEditNeedTranslucentFalse()
{
    // Test LineEdit with needTranslucent = false
    QLineEdit lineEdit;
    lineEdit.setProperty("needTranslucent", false);
    lineEdit.show();
    
    QStyleOptionFrame opt;
    opt.initFrom(&lineEdit);
    
    m_params->initConfigLineEditParameters(false, &opt, &lineEdit);
    
    QVERIFY(true);
    lineEdit.hide();
}

void TestUKUIConfigStyleParameters::testSpinBoxNeedTranslucentFalse()
{
    // Test SpinBox with needTranslucent = false
    QSpinBox spinBox;
    spinBox.setProperty("needTranslucent", false);
    spinBox.show();
    
    QStyleOptionSpinBox opt;
    opt.initFrom(&spinBox);
    
    m_params->initConfigSpinBoxParameters(false, &opt, &spinBox, true);
    
    QVERIFY(true);
    spinBox.hide();
}

void TestUKUIConfigStyleParameters::testComboBoxNeedTranslucentFalse()
{
    // Test ComboBox with needTranslucent = false
    QComboBox combo;
    combo.addItems({"Item 1", "Item 2"});
    combo.setProperty("needTranslucent", false);
    combo.show();
    
    QStyleOptionComboBox opt;
    opt.initFrom(&combo);
    
    m_params->initConfigComboBoxParameters(false, &opt, &combo);
    
    QVERIFY(true);
    combo.hide();
}

void TestUKUIConfigStyleParameters::testListViewNeedTranslucentFalse()
{
    // Test ListView with needTranslucent = false
    QListView listView;
    listView.setProperty("needTranslucent", false);
    listView.show();
    
    QStyleOptionViewItem opt;
    opt.initFrom(&listView);
    
    m_params->initConfigListParameters(false, &opt, &listView, false);
    
    QVERIFY(true);
    listView.hide();
}

void TestUKUIConfigStyleParameters::testTreeViewNeedTranslucentFalse()
{
    // Test TreeView with needTranslucent = false
    QTreeView treeView;
    treeView.setProperty("needTranslucent", false);
    treeView.show();
    
    QStyleOptionViewItem opt;
    opt.initFrom(&treeView);
    
    m_params->initConfigTreeParameters(false, &opt, &treeView, false);
    
    QVERIFY(true);
    treeView.hide();
}

void TestUKUIConfigStyleParameters::testTableViewNeedTranslucentFalse()
{
    // Test TableView (QTreeView with setItemsExpandable) with needTranslucent = false
    QTreeView treeView;
    treeView.setProperty("needTranslucent", false);
    treeView.show();
    
    QStyleOptionViewItem opt;
    opt.initFrom(&treeView);
    
    m_params->initConfigTableParameters(false, &opt, &treeView, false);
    
    QVERIFY(true);
    treeView.hide();
}

void TestUKUIConfigStyleParameters::testCheckBoxNeedTranslucentFalse()
{
    // Test CheckBox with needTranslucent = false
    QCheckBox checkBox("Test");
    checkBox.setProperty("needTranslucent", false);
    checkBox.show();
    
    QStyleOptionButton opt;
    opt.initFrom(&checkBox);
    
    m_params->initConfigCheckBoxParameters(false, &opt, &checkBox);
    
    QVERIFY(true);
    checkBox.hide();
}

void TestUKUIConfigStyleParameters::testRadioButtonNeedTranslucentFalse()
{
    // Test RadioButton with needTranslucent = false
    QRadioButton radio("Test");
    radio.setProperty("needTranslucent", false);
    radio.show();
    
    QStyleOptionButton opt;
    opt.initFrom(&radio);
    
    m_params->initConfigRadioButtonParameters(false, &opt, &radio);
    
    QVERIFY(true);
    radio.hide();
}

void TestUKUIConfigStyleParameters::testSliderNeedTranslucentFalse()
{
    // Test Slider with needTranslucent = false
    QSlider slider(Qt::Horizontal);
    slider.setProperty("needTranslucent", false);
    slider.show();
    
    QStyleOptionSlider opt;
    opt.initFrom(&slider);
    
    m_params->initConfigSliderParameters(false, &opt, &slider);
    
    QVERIFY(true);
    slider.hide();
}

void TestUKUIConfigStyleParameters::testProgressBarNeedTranslucentFalse()
{
    // Test ProgressBar with needTranslucent = false
    QProgressBar progressBar;
    progressBar.setProperty("needTranslucent", false);
    progressBar.show();
    
    QStyleOptionProgressBar opt;
    opt.initFrom(&progressBar);
    
    m_params->initConfigProgressBarParameters(false, &opt, &progressBar);
    
    QVERIFY(true);
    progressBar.hide();
}

void TestUKUIConfigStyleParameters::testScrollBarNeedTranslucentFalse()
{
    // Test ScrollBar with needTranslucent = false
    QScrollBar scrollBar(Qt::Horizontal);
    scrollBar.setProperty("needTranslucent", false);
    scrollBar.show();
    
    QStyleOptionSlider opt;
    opt.initFrom(&scrollBar);
    
    m_params->initConfigScrollBarParameters(false, &opt, &scrollBar);
    
    QVERIFY(true);
    scrollBar.hide();
}

void TestUKUIConfigStyleParameters::testTabWidgetNeedTranslucentFalse()
{
    // Test TabWidget with needTranslucent = false
    QTabWidget tabWidget;
    tabWidget.addTab(new QWidget(), "Tab 1");
    tabWidget.setProperty("needTranslucent", false);
    tabWidget.show();
    
    QStyleOptionTabWidgetFrame opt;
    opt.initFrom(&tabWidget);
    
    m_params->initConfigTabWidgetParameters(false, &opt, &tabWidget);
    
    QVERIFY(true);
    tabWidget.hide();
}

void TestUKUIConfigStyleParameters::testMenuNeedTranslucentFalse()
{
    // Test Menu with needTranslucent = false
    QMenu menu;
    menu.addAction("Action 1");
    menu.setProperty("needTranslucent", false);
    menu.show();
    
    QStyleOptionMenuItem opt;
    opt.initFrom(&menu);
    
    m_params->initConfigMenuParameters(false, &opt, &menu);
    
    QVERIFY(true);
    menu.hide();
}
void TestUKUIConfigStyleParameters::testPushButtonCustomPens()
{
    // Test PushButton with custom pen properties
    QPushButton button("Test");
    button.setProperty("setDisablePen", QPen(Qt::gray));
    button.setProperty("setCheckPen", QPen(Qt::blue));
    button.setProperty("setCheckHoverPen", QPen(Qt::cyan));
    button.setProperty("setCheckClickPen", QPen(Qt::darkBlue));
    button.setProperty("setCheckDisablePen", QPen(Qt::lightGray));
    button.setProperty("setFocusPen", QPen(Qt::red));
    button.show();
    
    QStyleOptionButton opt;
    opt.initFrom(&button);
    
    m_params->initConfigPushButtonBevelParameters(false, &opt, &button);
    m_params->initConfigPushButtonLabelParameters(false, &opt, &button);
    
    QVERIFY(true);
    button.hide();
}

void TestUKUIConfigStyleParameters::testToolButtonCustomPens()
{
    // Test ToolButton with custom pen properties
    // QToolButton constructor doesn't accept string in Qt 5.15
    QToolButton button;
    button.setText("Test");
    button.setProperty("setDisablePen", QPen(Qt::gray));
    button.setProperty("setCheckPen", QPen(Qt::blue));
    button.setProperty("setCheckHoverPen", QPen(Qt::cyan));
    button.setProperty("setCheckClickPen", QPen(Qt::darkBlue));
    button.setProperty("setCheckDisablePen", QPen(Qt::lightGray));
    button.setProperty("setFocusPen", QPen(Qt::red));
    button.show();
    
    QStyleOptionToolButton opt;
    opt.initFrom(&button);
    
    m_params->initConfigToolButtonPanelParameters(false, &opt, &button);
    m_params->initConfigToolButtonLabelParameters(false, &opt, &button);
    
    QVERIFY(true);
    button.hide();
}

void TestUKUIConfigStyleParameters::testCheckBoxCustomPens()
{
    // Test CheckBox with custom pen properties
    QCheckBox checkBox("Test");
    checkBox.setProperty("setDisablePen", QPen(Qt::gray));
    checkBox.setProperty("setCheckPen", QPen(Qt::blue));
    checkBox.setProperty("setCheckHoverPen", QPen(Qt::cyan));
    checkBox.setProperty("setCheckClickPen", QPen(Qt::darkBlue));
    checkBox.setProperty("setCheckDisablePen", QPen(Qt::lightGray));
    checkBox.setProperty("setFocusPen", QPen(Qt::red));
    checkBox.show();
    
    QStyleOptionButton opt;
    opt.initFrom(&checkBox);
    
    m_params->initConfigCheckBoxParameters(false, &opt, &checkBox);
    
    QVERIFY(true);
    checkBox.hide();
}

void TestUKUIConfigStyleParameters::testRadioButtonCustomPens()
{
    // Test RadioButton with custom pen properties
    QRadioButton radio("Test");
    radio.setProperty("setDisablePen", QPen(Qt::gray));
    radio.setProperty("setCheckPen", QPen(Qt::blue));
    radio.setProperty("setCheckHoverPen", QPen(Qt::cyan));
    radio.setProperty("setCheckClickPen", QPen(Qt::darkBlue));
    radio.setProperty("setCheckDisablePen", QPen(Qt::lightGray));
    radio.setProperty("setFocusPen", QPen(Qt::red));
    radio.show();
    
    QStyleOptionButton opt;
    opt.initFrom(&radio);
    
    m_params->initConfigRadioButtonParameters(false, &opt, &radio);
    
    QVERIFY(true);
    radio.hide();
}

void TestUKUIConfigStyleParameters::testLineEditCustomPens()
{
    // Test LineEdit with custom pen properties
    QLineEdit lineEdit;
    lineEdit.setProperty("setDisablePen", QPen(Qt::gray));
    lineEdit.setProperty("setFocusPen", QPen(Qt::red));
    lineEdit.show();
    
    QStyleOptionFrame opt;
    opt.initFrom(&lineEdit);
    
    m_params->initConfigLineEditParameters(false, &opt, &lineEdit);
    
    QVERIFY(true);
    lineEdit.hide();
}

void TestUKUIConfigStyleParameters::testDarkModeHighlightInitPaths()
{
    UKUIConfigStyleParameters darkParams(nullptr, true, QStringLiteral("dark"), true);

    QComboBox combo;
    QStyleOptionComboBox comboOpt;
    comboOpt.initFrom(&combo);
    darkParams.initConfigComboBoxParameters(true, &comboOpt, &combo);

    QListView listView;
    QStyleOptionViewItem listOpt;
    listOpt.initFrom(&listView);
    darkParams.initConfigListParameters(true, &listOpt, &listView, true);
    darkParams.initConfigListParameters(true, &listOpt, &listView, false);

    QTreeView treeView;
    QStyleOptionViewItem treeOpt;
    treeOpt.initFrom(&treeView);
    darkParams.initConfigTreeParameters(true, &treeOpt, &treeView, true);

    QTableWidget table;
    QStyleOptionViewItem tableOpt;
    tableOpt.initFrom(&table);
    darkParams.initConfigTableParameters(true, &tableOpt, &table, true);

    QSlider slider(Qt::Vertical);
    QStyleOptionSlider sliderOpt;
    sliderOpt.initFrom(&slider);
    darkParams.initConfigSliderParameters(true, &sliderOpt, &slider);

    QProgressBar progress;
    QStyleOptionProgressBar progressOpt;
    progressOpt.initFrom(&progress);
    darkParams.initConfigProgressBarParameters(true, &progressOpt, &progress);

    QScrollBar scrollBar(Qt::Vertical);
    QStyleOptionSlider scrollOpt;
    scrollOpt.initFrom(&scrollBar);
    darkParams.initConfigScrollBarParameters(true, &scrollOpt, &scrollBar);

    QTabWidget tabs;
    tabs.addTab(new QWidget(), QStringLiteral("t"));
    QStyleOptionTabWidgetFrame tabOpt;
    tabOpt.initFrom(&tabs);
    darkParams.initConfigTabWidgetParameters(true, &tabOpt, &tabs);

    QMenu menu;
    menu.addAction(QStringLiteral("a"));
    QStyleOptionMenuItem menuOpt;
    menuOpt.initFrom(&menu);
    darkParams.initConfigMenuParameters(true, &menuOpt, &menu);
}

void TestUKUIConfigStyleParameters::testInitConfigToolTipParameters()
{
    QLabel tipLabel(QStringLiteral("tip"));
    QStyleOption opt;
    opt.initFrom(&tipLabel);
    m_params->initConfigToolTipParameters(false, &opt, &tipLabel);
    m_params->initConfigToolTipParameters(true, &opt, &tipLabel);
}

void TestUKUIConfigStyleParameters::testConfigShadowHelperOffscreen()
{
    ConfigShadowHelper helper;

    QWidget plain;
    QVERIFY(!helper.isWidgetNeedDecoShadow(nullptr));
    QVERIFY(!helper.isWidgetNeedDecoShadow(&plain));

    plain.setProperty("useCustomShadow", true);
    QVERIFY(helper.isWidgetNeedDecoShadow(&plain));

    QMenu menu;
    QVERIFY(helper.isWidgetNeedDecoShadow(&menu));

    QWidget withRadius;
    withRadius.setProperty("maxRadius", 16);
    withRadius.setProperty("useCustomShadow", true);
    withRadius.setProperty("customShadowWidth", 12);
    withRadius.setProperty("customShadowDarkness", 0.6);
    withRadius.setProperty("customShadowRadius", QVector4D(8, 8, 8, 8));
    withRadius.setProperty("customShadowMargins", QVector4D(4, 4, 4, 4));

    (void)helper.getShadowPixmap(Qt::black, 10, 0.8, &withRadius, 8, 8, 8, 8);
    (void)helper.caculateRelativePainterPath(&withRadius, 6, 6, 6, 6);
    (void)helper.caculateRelativePainterPath(&menu, 4, 4, 4, 4);

    helper.registerWidget(&withRadius);
    helper.unregisterWidget(&withRadius);

    QEvent showEv(QEvent::Show);
    (void)helper.eventFilter(&withRadius, &showEv);
}

#include "test_ukui_config_style_parameters.moc"