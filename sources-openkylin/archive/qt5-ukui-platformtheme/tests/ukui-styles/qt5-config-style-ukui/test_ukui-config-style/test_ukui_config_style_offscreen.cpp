/*
 * Broad ukui-config-style.cpp coverage — QImage painter only (offscreen, no event loop).
 */

#include <QTest>
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QSlider>
#include <QProgressBar>
#include <QScrollBar>
#include <QTabWidget>
#include <QTabBar>
#include <QHeaderView>
#include <QTableWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QListWidget>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStyledItemDelegate>
#include <QStyleOptionTabBarBase>
#include <QGroupBox>
#include <QMenu>
#include <QToolBar>
#include <QDockWidget>
#include <QSplitter>
#include <QFrame>
#include <QFocusEvent>
#include <QStyleOptionMenuItem>
#include <QStyleOptionToolBox>
#include <QStyleOptionSizeGrip>
#include <QStyleOptionDockWidget>
#include <QStyleOptionToolButton>
#include <QStyleOptionGroupBox>
#include <QStyleOptionTitleBar>
#include <QStyleOptionToolBar>
#include <QStyleOptionViewItem>
#include <QStyleOptionSpinBox>
#include <QStyleOptionGroupBox>
#include <QStyleOptionToolBar>
#include <QStyleOptionDockWidget>
#include <QStyleOptionTitleBar>
#include <QMenu>
#include <QSpinBox>
#include <QGroupBox>
#include <QToolBar>
#include <QDockWidget>
#include <QMainWindow>
#include <QLabel>
#include <QStyleHintReturn>
#include <QStyleOption>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QStyleOptionFrame>
#include <QStyleOptionProgressBar>
#include <QStyleOptionSlider>
#include <QStyleOptionTab>
#include <QStyleOptionTabWidgetFrame>
#include <QStyleOptionViewItem>
#include <QStyleOptionHeader>
#include <QStyleOptionToolButton>
#include <QStyleOptionGroupBox>
#include <QStyleOptionToolBox>
#include <QStyleOptionSpinBox>
#include <QImage>
#include <QPainter>

#include "ukui-config-style.h"
#include "effects/highlight-effect.h"
#include "readconfig.h"

#include <QFile>
#include <QGuiApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>
#include <functional>

using namespace UKUIConfigStyleSpace;

class ExposedStyledItemDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::initStyleOption;
};

class TestableUKUIConfigStyle : public UKUIConfigStyle
{
public:
    explicit TestableUKUIConfigStyle(const QString &name) : UKUIConfigStyle(name) {}
    using UKUIConfigStyle::realSetWindowSurfaceFormatAlpha;
    using UKUIConfigStyle::realSetMenuTypeToMenu;
    using UKUIConfigStyle::centerRect;
};

class TestUKUIConfigStyleOffscreen : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testStyleHintsAndMetrics();
    void testDrawPrimitivesBulk();
    void testDrawControlsBulk();
    void testDrawComplexControlsBulk();
    void testPolishPaletteAndWidgets();
    void testSizeFromContentsAndSubElements();
    void testEnumerateStyleElements();
    void testConfigStyleExtendedApi();
    void testConfigStyleWidgetStates();
    void testDrawControlsMenuAndMisc();
    void testDrawPrimitivesWithStates();
    void testMoreSizeFromContentsAndSubElements();
    void testDrawControlsScrollAndMenuExtra();
    void testSubControlRectComprehensive();
    void testSubElementRectComprehensive();
    void testPixelMetricsWithWidgetVariants();
    void testDrawComplexControlsExtended();
    void testHitTestAndStandardPixmapVariants();
    void testDrawWithUkuiWidgetProperties();
    void testDrawPrimitivePropertyPaths();
    void testDrawItemViewTreeTableList();
    void testDrawComplexControlFullProperties();
    void testSizeFromContentsUkuiTypes();
    void testSubElementRectRtlAndVariants();
    void testDrawListTableItemViewDeep();
    void testItemViewExhaustiveStates();
    void testDrawControlRichBulk();
    void testDrawControlMegaSuite();
    void testSizeFromContentsAllTypes();
    void testComplexControlPropertyPaths();
    void testIconModeSelectedFullPath();
    void testReadConfigAndHighlightPaths();
    void testPrimitiveItemViewPePaths();
    void testReadConfigAllTokenGetters();
    void testReadConfigDeepCoverage();
    void testReadConfigParseEdgeCases();
    void testItemViewDelegateStyleOptionPe();
    void testDrawPrimitiveSettingsFullPort();
    void testPolishEventFilterAndRealWidgetDraw();
    void testComplexControlSubControlsExhaustive();
    void testHighlightEffectExhaustiveOffscreen();
};

static QImage paintBuffer(int w = 160, int h = 120)
{
    QImage img(w, h, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    return img;
}

void TestUKUIConfigStyleOffscreen::testStyleHintsAndMetrics()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget w;
    w.resize(200, 200);

    const QStyle::StyleHint hints[] = {
        QStyle::SH_ScrollBar_Transient,
        QStyle::SH_ItemView_ShowDecorationSelected,
        QStyle::SH_DialogButtonBox_ButtonsHaveIcons,
        QStyle::SH_DialogButtons_DefaultButton,
        QStyle::SH_UnderlineShortcut,
        QStyle::SH_ComboBox_Popup,
        QStyle::SH_ComboBox_AllowWheelScrolling,
        QStyle::SH_ComboBox_PopupFrameStyle,
        QStyle::SH_Button_FocusPolicy,
        QStyle::SH_Header_ArrowAlignment,
        QStyle::SH_ItemView_ActivateItemOnSingleClick,
        QStyle::SH_DialogButtonLayout,
        QStyle::SH_Menu_Scrollable,
        QStyle::SH_BlinkCursorWhenTextSelected,
        QStyle::SH_Slider_SnapToValue,
        QStyle::SH_TabBar_Alignment,
        QStyle::SH_ToolBar_Movable,
    };
    for (QStyle::StyleHint h : hints)
        (void)style.styleHint(h, nullptr, &w, nullptr);

    const QStyle::PixelMetric metrics[] = {
        QStyle::PM_ButtonMargin,
        QStyle::PM_ButtonDefaultIndicator,
        QStyle::PM_MenuButtonIndicator,
        QStyle::PM_ListViewIconSize,
        QStyle::PM_TabBarTabOverlap,
        QStyle::PM_TabBarTabHSpace,
        QStyle::PM_TabBarTabVSpace,
        QStyle::PM_TabBarBaseHeight,
        QStyle::PM_ScrollBarExtent,
        QStyle::PM_HeaderMargin,
        QStyle::PM_ToolBarIconSize,
    };
    QStyleOptionButton btnOpt;
    btnOpt.rect = QRect(0, 0, 80, 28);
    for (QStyle::PixelMetric m : metrics)
        (void)style.pixelMetric(m, &btnOpt, &w);

    int colorVal = 64;
    style.calculateHighColor(colorVal);
    colorVal = 200;
    style.calculateHighColor(colorVal);

    const QStyle::StandardPixmap pixmaps[] = {
        QStyle::SP_DirIcon,
        QStyle::SP_FileDialogListView,
        QStyle::SP_FileDialogDetailedView,
        QStyle::SP_LineEditClearButton,
        QStyle::SP_ToolBarHorizontalExtensionButton,
        QStyle::SP_TitleBarMinButton,
        QStyle::SP_MessageBoxInformation,
    };
    for (QStyle::StandardPixmap sp : pixmaps)
        (void)style.standardIcon(sp, nullptr, &w);
}

void TestUKUIConfigStyleOffscreen::testDrawPrimitivesBulk()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;
    QStyleOption opt;
    opt.rect = QRect(0, 0, 100, 30);
    opt.state = QStyle::State_Enabled;
    opt.palette = style.standardPalette();

    const QStyle::PrimitiveElement elements[] = {
        QStyle::PE_PanelButtonCommand,
        QStyle::PE_PanelButtonBevel,
        QStyle::PE_PanelButtonTool,
        QStyle::PE_IndicatorArrowDown,
        QStyle::PE_IndicatorArrowUp,
        QStyle::PE_IndicatorArrowLeft,
        QStyle::PE_IndicatorArrowRight,
        QStyle::PE_IndicatorCheckBox,
        QStyle::PE_IndicatorRadioButton,
        QStyle::PE_FrameTabWidget,
        QStyle::PE_FrameGroupBox,
        QStyle::PE_Frame,
        QStyle::PE_PanelLineEdit,
        QStyle::PE_IndicatorToolBarHandle,
        QStyle::PE_IndicatorBranch,
        QStyle::PE_PanelMenu,
        QStyle::PE_IndicatorHeaderArrow,
        QStyle::PE_FrameStatusBarItem,
    };

    for (QStyle::PrimitiveElement pe : elements) {
        QImage img = paintBuffer();
        QPainter p(&img);
        style.drawPrimitive(pe, &opt, &p, &widget);
    }
}

void TestUKUIConfigStyleOffscreen::testDrawControlsBulk()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget host;
    host.resize(240, 240);

    auto draw = [&](QStyle::ControlElement ce, QStyleOption &opt) {
        QImage img = paintBuffer(200, 80);
        QPainter p(&img);
        style.drawControl(ce, &opt, &p, &host);
    };

    QPushButton pushBtn(QStringLiteral("btn"));
    QStyleOptionButton btnOpt;
    btnOpt.initFrom(&pushBtn);
    draw(QStyle::CE_PushButton, btnOpt);
    draw(QStyle::CE_PushButtonLabel, btnOpt);

    QCheckBox checkBox(QStringLiteral("cb"));
    QStyleOptionButton checkOpt;
    checkOpt.initFrom(&checkBox);
    draw(QStyle::CE_CheckBox, checkOpt);

    QRadioButton radio(QStringLiteral("rb"));
    QStyleOptionButton radioOpt;
    radioOpt.initFrom(&radio);
    draw(QStyle::CE_RadioButton, radioOpt);

    QComboBox combo;
    combo.addItem(QStringLiteral("a"));
    QStyleOptionComboBox comboOpt;
    comboOpt.initFrom(&combo);
    draw(QStyle::CE_ComboBoxLabel, comboOpt);

    QLineEdit lineEdit;
    QStyleOptionFrame frameOpt;
    frameOpt.initFrom(&lineEdit);
    draw(QStyle::CE_ShapedFrame, frameOpt);

    QProgressBar progress;
    QStyleOptionProgressBar pbOpt;
    pbOpt.initFrom(&progress);
    draw(QStyle::CE_ProgressBar, pbOpt);
    draw(QStyle::CE_ProgressBarGroove, pbOpt);
    draw(QStyle::CE_ProgressBarContents, pbOpt);

    QTabBar tabBar;
    tabBar.addTab(QStringLiteral("t"));
    QStyleOptionTab tabOpt;
    tabOpt.initFrom(&tabBar);
    draw(QStyle::CE_TabBarTab, tabOpt);
    draw(QStyle::CE_TabBarTabLabel, tabOpt);

    QHeaderView header(Qt::Horizontal);
    QStyleOptionHeader headerOpt;
    headerOpt.rect = QRect(0, 0, 80, 24);
    draw(QStyle::CE_Header, headerOpt);
    draw(QStyle::CE_HeaderSection, headerOpt);

    QStyleOptionViewItem viewOpt;
    viewOpt.rect = QRect(0, 0, 100, 24);
    viewOpt.state = QStyle::State_Enabled;
    draw(QStyle::CE_ItemViewItem, viewOpt);

}

void TestUKUIConfigStyleOffscreen::testDrawComplexControlsBulk()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget host;

    auto drawCc = [&](QStyle::ComplexControl cc, QStyleOptionComplex &opt) {
        QImage img = paintBuffer(200, 60);
        QPainter p(&img);
        style.drawComplexControl(cc, &opt, &p, &host);
    };

    QSpinBox spin;
    QStyleOptionSpinBox spinOpt;
    spinOpt.initFrom(&spin);
    drawCc(QStyle::CC_SpinBox, spinOpt);

    QSlider slider(Qt::Horizontal);
    QStyleOptionSlider sliderOpt;
    sliderOpt.initFrom(&slider);
    drawCc(QStyle::CC_Slider, sliderOpt);

    QScrollBar scroll(Qt::Horizontal);
    QStyleOptionSlider scrollOpt;
    scrollOpt.initFrom(&scroll);
    drawCc(QStyle::CC_ScrollBar, scrollOpt);

    QComboBox combo;
    combo.addItem(QStringLiteral("x"));
    QStyleOptionComboBox comboOpt;
    comboOpt.initFrom(&combo);
    drawCc(QStyle::CC_ComboBox, comboOpt);
}

void TestUKUIConfigStyleOffscreen::testPolishPaletteAndWidgets()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));

    QPalette pal = style.standardPalette();
    style.polish(pal);

    QPushButton btn(QStringLiteral("b"));
    QLineEdit edit;
    QComboBox combo;
    combo.addItem(QStringLiteral("i"));
    QTabWidget tabs;
    tabs.addTab(new QWidget, QStringLiteral("t"));

    style.polish(&btn);
    style.polish(&edit);
    style.polish(&combo);
    style.polish(&tabs);
    style.unpolish(&btn);

    QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance());
    if (app)
        style.polish(app);
}

void TestUKUIConfigStyleOffscreen::testSizeFromContentsAndSubElements()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;
    const QSize base(100, 30);

    QStyleOptionButton btnOpt;
    btnOpt.rect = QRect(0, 0, 80, 28);
    (void)style.sizeFromContents(QStyle::CT_PushButton, &btnOpt, base, &widget);
    (void)style.sizeFromContents(QStyle::CT_CheckBox, &btnOpt, base, &widget);
    (void)style.sizeFromContents(QStyle::CT_RadioButton, &btnOpt, base, &widget);
    (void)style.sizeFromContents(QStyle::CT_ToolButton, &btnOpt, base, &widget);

    QStyleOptionComboBox comboOpt;
    comboOpt.rect = btnOpt.rect;
    (void)style.sizeFromContents(QStyle::CT_ComboBox, &comboOpt, base, &widget);

    QStyleOptionFrame frameOpt;
    frameOpt.rect = btnOpt.rect;
    (void)style.sizeFromContents(QStyle::CT_LineEdit, &frameOpt, base, &widget);

    QStyleOptionTab tabOpt;
    tabOpt.rect = btnOpt.rect;
    (void)style.subElementRect(UKUIConfigStyle::SE_TabBarTabText, &tabOpt, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_PushButtonContents, &btnOpt, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_CheckBoxIndicator, &btnOpt, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_RadioButtonIndicator, &btnOpt, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_ProgressBarGroove, &btnOpt, &widget);

    QStyleOptionTabWidgetFrame tabFrameOpt;
    tabFrameOpt.rect = QRect(0, 0, 200, 100);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetTabPane, &tabFrameOpt, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetTabContents, &tabFrameOpt, &widget);
}

void TestUKUIConfigStyleOffscreen::testEnumerateStyleElements()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;
    widget.resize(120, 120);

    QStyleOption opt;
    opt.rect = QRect(0, 0, 100, 32);
    opt.state = QStyle::State_Enabled | QStyle::State_Active;
    opt.palette = style.standardPalette();

    for (int pe = int(QStyle::PE_Frame); pe <= int(QStyle::PE_IndicatorTabTearRight); ++pe) {
        QImage img = paintBuffer();
        QPainter p(&img);
        style.drawPrimitive(static_cast<QStyle::PrimitiveElement>(pe), &opt, &p, &widget);
    }

    for (int ce = int(QStyle::CE_PushButton); ce <= int(QStyle::CE_ShapedFrame); ++ce) {
        QStyleOptionButton btnOpt;
        btnOpt.initFrom(&widget);
        btnOpt.rect = opt.rect;
        QImage img = paintBuffer();
        QPainter p(&img);
        style.drawControl(static_cast<QStyle::ControlElement>(ce), &btnOpt, &p, &widget);
    }

    QStyleOptionComplex complexOpt;
    complexOpt.rect = opt.rect;
    complexOpt.state = opt.state;
    for (int cc = int(QStyle::CC_SpinBox); cc <= int(QStyle::CC_MdiControls); ++cc) {
        QImage img = paintBuffer();
        QPainter p(&img);
        style.drawComplexControl(static_cast<QStyle::ComplexControl>(cc), &complexOpt, &p, &widget);
    }

    for (int sh = int(QStyle::SH_EtchDisabledText); sh <= int(QStyle::SH_SpinBox_StepModifier); ++sh) {
        (void)style.styleHint(static_cast<QStyle::StyleHint>(sh), &opt, &widget, nullptr);
    }

    for (int pm = int(QStyle::PM_ButtonMargin); pm <= int(QStyle::PM_TitleBarButtonSize); ++pm) {
        (void)style.pixelMetric(static_cast<QStyle::PixelMetric>(pm), &opt, &widget);
    }
}

void TestUKUIConfigStyleOffscreen::testConfigStyleExtendedApi()
{
    TestableUKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget w;
    w.resize(200, 200);

    (void)style.shouldBeTransparent(&w);
    QMetaObject::invokeMethod(&style, "updateTabletModeValue", Q_ARG(bool, true));
    QMetaObject::invokeMethod(&style, "updateTabletModeValue", Q_ARG(bool, false));

    QStyleOption opt;
    opt.initFrom(&w);
    opt.rect = w.rect();

    QPixmap pm(32, 32);
    pm.fill(Qt::red);
    (void)style.generatedIconPixmap(QIcon::Mode::Normal, pm, &opt);
    (void)style.testGeneratedSVGPixmap(pm);

    QStyleOptionComplex ccOpt;
    ccOpt.rect = opt.rect;
    ccOpt.state = opt.state;
    (void)style.hitTestComplexControl(QStyle::CC_ComboBox, &ccOpt, QPoint(4, 4), &w);
    (void)style.subControlRect(QStyle::CC_ComboBox, &ccOpt, QStyle::SC_ComboBoxEditField, &w);

    QImage img = paintBuffer();
    QPainter painter(&img);
    style.drawItemPixmap(&painter, opt.rect, Qt::AlignCenter, pm);

    QPixmap colored = style.drawColorPixmap(&painter, QColor(0x3D, 0x6B, 0xE5), pm);

    style.realSetWindowSurfaceFormatAlpha(&w);
    style.realSetMenuTypeToMenu(&w);
    (void)style.centerRect(QRect(0, 0, 100, 100), 20, 20);
    Q_UNUSED(colored);

    QFocusEvent focusIn(QEvent::FocusIn);
    style.eventFilter(&w, &focusIn);
}

void TestUKUIConfigStyleOffscreen::testConfigStyleWidgetStates()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));

    QPushButton btn(QStringLiteral("push"));
    btn.setCheckable(true);
    btn.setChecked(true);
    QToolButton toolBtn;
    QCheckBox check(QStringLiteral("cb"));
    check.setTristate(true);
    check.setCheckState(Qt::PartiallyChecked);
    QRadioButton radio(QStringLiteral("rb"));
    QComboBox combo;
    combo.setEditable(true);
    combo.addItem(QStringLiteral("one"));
    QLineEdit lineEdit(QStringLiteral("edit"));
    QSpinBox spin;
    QSlider slider(Qt::Horizontal);
    QProgressBar progress;
    progress.setRange(0, 100);
    progress.setValue(40);
    QScrollBar scroll(Qt::Vertical);
    QTabWidget tabs;
    tabs.addTab(new QWidget, QStringLiteral("tab"));
    QTableWidget table(2, 2);
    QTreeView tree;
    QListView list;
    QGroupBox group(QStringLiteral("group"));
    QToolBar toolbar;
    QDockWidget dock(QStringLiteral("dock"));
    QSplitter splitter;
    QFrame frame;

    const QWidget *widgets[] = {
        &btn, &toolBtn, &check, &radio, &combo, &lineEdit, &spin, &slider,
        &progress, &scroll, &tabs, &table, &tree, &list, &group, &toolbar, &dock, &frame,
    };
    for (const QWidget *widget : widgets) {
        style.polish(const_cast<QWidget *>(widget));
        (void)style.shouldBeTransparent(widget);
    }

    auto drawAll = [&](const QWidget *widget) {
        QStyleOption opt;
        opt.initFrom(widget);
        opt.rect = QRect(0, 0, 120, 32);
        opt.state |= QStyle::State_Enabled | QStyle::State_Active | QStyle::State_MouseOver;
        QImage img = paintBuffer(140, 48);
        QPainter p(&img);
        style.drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, &p, widget);
        QStyleOptionButton btnOpt;
        btnOpt.initFrom(widget);
        btnOpt.rect = opt.rect;
        style.drawControl(QStyle::CE_PushButton, &btnOpt, &p, widget);
    };
    drawAll(&btn);
    drawAll(&check);
    drawAll(&combo);
}

void TestUKUIConfigStyleOffscreen::testDrawControlsMenuAndMisc()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget host;
    host.resize(240, 240);

    auto drawCe = [&](QStyle::ControlElement ce, QStyleOption &opt) {
        QImage img = paintBuffer(220, 80);
        QPainter p(&img);
        style.drawControl(ce, &opt, &p, &host);
    };

    {
        QStyleOptionMenuItem opt;
        opt.state = QStyle::State_Enabled;
        opt.menuItemType = QStyleOptionMenuItem::EmptyArea;
        opt.rect = QRect(0, 0, 200, 30);
        drawCe(QStyle::CE_MenuBarEmptyArea, opt);
    }
    {
        QStyleOptionSizeGrip opt;
        opt.state = QStyle::State_Enabled;
        opt.corner = Qt::BottomRightCorner;
        opt.rect = QRect(0, 0, 20, 20);
        drawCe(QStyle::CE_SizeGrip, opt);
    }
    {
        QStyleOption opt;
        opt.state = QStyle::State_Enabled | QStyle::State_Horizontal;
        opt.rect = QRect(0, 0, 200, 5);
        drawCe(QStyle::CE_Splitter, opt);
    }
    {
        QStyleOptionToolBox opt;
        opt.state = QStyle::State_Enabled;
        opt.rect = QRect(0, 0, 100, 30);
        opt.text = QStringLiteral("Tab");
        drawCe(QStyle::CE_ToolBoxTab, opt);
    }
    {
        QStyleOptionProgressBar opt;
        opt.state = QStyle::State_Enabled;
        opt.rect = QRect(0, 0, 200, 20);
        opt.minimum = 0;
        opt.maximum = 100;
        opt.progress = 50;
        opt.text = QStringLiteral("50%");
        drawCe(QStyle::CE_ProgressBarContents, opt);
        drawCe(QStyle::CE_ProgressBarGroove, opt);
        drawCe(QStyle::CE_ProgressBarLabel, opt);
    }
    {
        QStyleOptionMenuItem opt;
        opt.state = QStyle::State_Enabled;
        opt.menuItemType = QStyleOptionMenuItem::Normal;
        opt.rect = QRect(0, 0, 200, 30);
        opt.text = QStringLiteral("Menu Item");
        drawCe(QStyle::CE_MenuItem, opt);
        opt.state |= QStyle::State_On;
        opt.checked = true;
        drawCe(QStyle::CE_MenuItem, opt);
        opt.state = QStyle::State_None;
        opt.menuItemType = QStyleOptionMenuItem::Separator;
        opt.rect = QRect(0, 0, 200, 10);
        drawCe(QStyle::CE_MenuItem, opt);
        opt.state = QStyle::State_Enabled;
        opt.menuItemType = QStyleOptionMenuItem::SubMenu;
        opt.text = QStringLiteral("Sub");
        opt.rect = QRect(0, 0, 200, 30);
        drawCe(QStyle::CE_MenuItem, opt);
    }
    {
        QStyleOptionToolButton opt;
        opt.state = QStyle::State_Enabled;
        opt.rect = QRect(0, 0, 80, 30);
        opt.text = QStringLiteral("Tool");
        opt.subControls = QStyle::SC_ToolButton;
        drawCe(QStyle::CE_ToolButtonLabel, opt);
    }
    {
        QStyleOptionButton opt;
        opt.state = QStyle::State_Enabled | QStyle::State_On;
        opt.rect = QRect(0, 0, 100, 20);
        opt.text = QStringLiteral("Radio");
        drawCe(QStyle::CE_RadioButton, opt);
        opt.state = QStyle::State_Enabled | QStyle::State_NoChange;
        opt.text = QStringLiteral("Check");
        drawCe(QStyle::CE_CheckBox, opt);
        opt.state = QStyle::State_Enabled | QStyle::State_Sunken;
        opt.text = QStringLiteral("Push");
        drawCe(QStyle::CE_PushButton, opt);
    }
    {
        QStyleOptionComboBox opt;
        opt.state = QStyle::State_Enabled;
        opt.rect = QRect(0, 0, 100, 30);
        opt.currentText = QStringLiteral("Combo");
        drawCe(QStyle::CE_ComboBoxLabel, opt);
    }
    {
        QStyleOptionHeader opt;
        opt.state = QStyle::State_Enabled | QStyle::State_Sunken;
        opt.rect = QRect(0, 0, 100, 30);
        opt.text = QStringLiteral("Header");
        opt.sortIndicator = QStyleOptionHeader::SortDown;
        drawCe(QStyle::CE_Header, opt);
        drawCe(QStyle::CE_HeaderSection, opt);
    }
    {
        QStyleOptionTab opt;
        opt.state = QStyle::State_Enabled | QStyle::State_Selected;
        opt.rect = QRect(0, 0, 80, 30);
        opt.text = QStringLiteral("Tab1");
        opt.position = QStyleOptionTab::Beginning;
        drawCe(QStyle::CE_TabBarTab, opt);
        opt.state = QStyle::State_Enabled;
        opt.position = QStyleOptionTab::Middle;
        drawCe(QStyle::CE_TabBarTabShape, opt);
        drawCe(QStyle::CE_TabBarTabLabel, opt);
    }
    {
        QStyleOptionDockWidget opt;
        opt.state = QStyle::State_Enabled;
        opt.rect = QRect(0, 0, 200, 30);
        opt.title = QStringLiteral("Dock");
        opt.closable = true;
        drawCe(QStyle::CE_DockWidgetTitle, opt);
    }
    {
        QStyleOptionSlider opt;
        opt.state = QStyle::State_Enabled;
        opt.rect = QRect(0, 0, 20, 100);
        opt.minimum = 0;
        opt.maximum = 100;
        opt.sliderValue = 50;
        opt.orientation = Qt::Vertical;
        drawCe(QStyle::CE_ScrollBarSlider, opt);
        drawCe(QStyle::CE_ScrollBarAddLine, opt);
        drawCe(QStyle::CE_ScrollBarSubLine, opt);
        opt.orientation = Qt::Horizontal;
        opt.rect = QRect(0, 0, 100, 20);
        drawCe(QStyle::CE_ScrollBarAddPage, opt);
        drawCe(QStyle::CE_ScrollBarSubPage, opt);
    }
    {
        QStyleOptionViewItem opt;
        opt.state = QStyle::State_Enabled | QStyle::State_Selected;
        opt.rect = QRect(0, 0, 120, 24);
        opt.text = QStringLiteral("row");
        drawCe(QStyle::CE_ItemViewItem, opt);
    }
}

void TestUKUIConfigStyleOffscreen::testDrawPrimitivesWithStates()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;

    const QStyle::State states[] = {
        QStyle::State_Enabled,
        QStyle::State_Enabled | QStyle::State_MouseOver,
        QStyle::State_Enabled | QStyle::State_Sunken,
        QStyle::State_Enabled | QStyle::State_On,
        QStyle::State_Enabled | QStyle::State_HasFocus,
        QStyle::State_Enabled | QStyle::State_Raised,
    };

    const QStyle::PrimitiveElement elements[] = {
        QStyle::PE_PanelButtonCommand,
        QStyle::PE_PanelButtonBevel,
        QStyle::PE_IndicatorCheckBox,
        QStyle::PE_IndicatorRadioButton,
        QStyle::PE_IndicatorArrowDown,
        QStyle::PE_FrameLineEdit,
        QStyle::PE_PanelMenu,
        QStyle::PE_IndicatorHeaderArrow,
        QStyle::PE_FrameTabWidget,
        QStyle::PE_FrameGroupBox,
    };

    for (QStyle::State st : states) {
        QStyleOption opt;
        opt.rect = QRect(0, 0, 80, 28);
        opt.state = st;
        opt.palette = style.standardPalette();
        for (QStyle::PrimitiveElement pe : elements) {
            QImage img = paintBuffer();
            QPainter p(&img);
            style.drawPrimitive(pe, &opt, &p, &widget);
        }
    }
}

void TestUKUIConfigStyleOffscreen::testMoreSizeFromContentsAndSubElements()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;
    const QSize base(120, 36);

    QStyleOptionProgressBar pbOpt;
    pbOpt.rect = QRect(0, 0, 200, 20);
    (void)style.sizeFromContents(QStyle::CT_ProgressBar, &pbOpt, base, &widget);

    QStyleOptionTabWidgetFrame tabFrame;
    tabFrame.rect = QRect(0, 0, 240, 120);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetLeftCorner, &tabFrame, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetRightCorner, &tabFrame, &widget);

    QStyleOptionGroupBox gbOpt;
    gbOpt.rect = QRect(0, 0, 160, 80);
    gbOpt.text = QStringLiteral("g");
    (void)style.sizeFromContents(QStyle::CT_GroupBox, &gbOpt, base, &widget);

    QStyleOptionSlider sliderOpt;
    sliderOpt.rect = QRect(0, 0, 200, 24);
    sliderOpt.orientation = Qt::Horizontal;
    QStyleOptionComplex ccOpt;
    ccOpt.rect = sliderOpt.rect;
    (void)style.subControlRect(QStyle::CC_Slider, &ccOpt, QStyle::SC_SliderHandle, &widget);
    (void)style.subControlRect(QStyle::CC_ScrollBar, &ccOpt, QStyle::SC_ScrollBarSlider, &widget);
}

void TestUKUIConfigStyleOffscreen::testDrawControlsScrollAndMenuExtra()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget host;

    auto drawCe = [&](QStyle::ControlElement ce, QStyleOption &opt) {
        QImage img = paintBuffer(220, 80);
        QPainter p(&img);
        style.drawControl(ce, &opt, &p, &host);
    };

    QStyleOptionSlider scrollOpt;
    scrollOpt.state = QStyle::State_Enabled;
    scrollOpt.minimum = 0;
    scrollOpt.maximum = 100;
    scrollOpt.sliderValue = 50;
    scrollOpt.rect = QRect(0, 0, 20, 20);
    scrollOpt.orientation = Qt::Vertical;
    drawCe(QStyle::CE_ScrollBarFirst, scrollOpt);
    drawCe(QStyle::CE_ScrollBarLast, scrollOpt);

    QStyleOptionMenuItem menuScroller;
    menuScroller.state = QStyle::State_Enabled;
    menuScroller.menuItemType = QStyleOptionMenuItem::Scroller;
    menuScroller.rect = QRect(0, 0, 200, 20);
    drawCe(QStyle::CE_MenuScroller, menuScroller);

    QStyleOptionMenuItem menuEmpty;
    menuEmpty.state = QStyle::State_Enabled;
    menuEmpty.menuItemType = QStyleOptionMenuItem::EmptyArea;
    menuEmpty.rect = QRect(0, 0, 200, 100);
    drawCe(QStyle::CE_MenuEmptyArea, menuEmpty);

    QStyleOptionProgressBar vpb;
    vpb.state = QStyle::State_Enabled;
    vpb.rect = QRect(0, 0, 20, 200);
    vpb.minimum = 0;
    vpb.maximum = 100;
    vpb.progress = 50;
    vpb.orientation = Qt::Vertical;
    drawCe(QStyle::CE_ProgressBarContents, vpb);

    QStyleOptionTab tabEnd;
    tabEnd.state = QStyle::State_Enabled;
    tabEnd.rect = QRect(0, 0, 80, 30);
    tabEnd.text = QStringLiteral("End");
    tabEnd.position = QStyleOptionTab::End;
    tabEnd.shape = QTabBar::RoundedNorth;
    drawCe(QStyle::CE_TabBarTab, tabEnd);

}

void TestUKUIConfigStyleOffscreen::testSubControlRectComprehensive()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;
    widget.setProperty("drawScrollBarGroove", true);

    auto sub = [&](QStyle::ComplexControl cc, QStyleOptionComplex *opt, QStyle::SubControl sc) {
        (void)style.subControlRect(cc, opt, sc, &widget);
    };

    QStyleOptionSlider hScroll;
    hScroll.orientation = Qt::Horizontal;
    hScroll.rect = QRect(0, 0, 300, 20);
    hScroll.minimum = 0;
    hScroll.maximum = 100;
    hScroll.pageStep = 10;
    hScroll.sliderPosition = 50;
    hScroll.state = QStyle::State_Enabled;
    sub(QStyle::CC_ScrollBar, &hScroll, QStyle::SC_ScrollBarSubLine);
    sub(QStyle::CC_ScrollBar, &hScroll, QStyle::SC_ScrollBarAddLine);
    sub(QStyle::CC_ScrollBar, &hScroll, QStyle::SC_ScrollBarFirst);
    sub(QStyle::CC_ScrollBar, &hScroll, QStyle::SC_ScrollBarLast);
    sub(QStyle::CC_ScrollBar, &hScroll, QStyle::SC_ScrollBarSubPage);
    sub(QStyle::CC_ScrollBar, &hScroll, QStyle::SC_ScrollBarAddPage);
    sub(QStyle::CC_ScrollBar, &hScroll, QStyle::SC_ScrollBarSlider);
    sub(QStyle::CC_ScrollBar, &hScroll, QStyle::SC_ScrollBarGroove);

    QStyleOptionSlider vScroll = hScroll;
    vScroll.orientation = Qt::Vertical;
    vScroll.rect = QRect(0, 0, 20, 300);
    sub(QStyle::CC_ScrollBar, &vScroll, QStyle::SC_ScrollBarSubLine);
    sub(QStyle::CC_ScrollBar, &vScroll, QStyle::SC_ScrollBarAddPage);

    widget.setProperty("drawScrollBarGroove", false);
    sub(QStyle::CC_ScrollBar, &hScroll, QStyle::SC_ScrollBarSubLine);

    QStyleOptionComboBox comboOpt;
    comboOpt.rect = QRect(0, 0, 200, 30);
    comboOpt.state = QStyle::State_Enabled;
    comboOpt.editable = true;
    comboOpt.frame = true;
    sub(QStyle::CC_ComboBox, &comboOpt, QStyle::SC_ComboBoxFrame);
    sub(QStyle::CC_ComboBox, &comboOpt, QStyle::SC_ComboBoxArrow);
    sub(QStyle::CC_ComboBox, &comboOpt, QStyle::SC_ComboBoxEditField);

    QStyleOptionSlider sliderOpt;
    sliderOpt.orientation = Qt::Horizontal;
    sliderOpt.rect = QRect(0, 0, 200, 30);
    sliderOpt.minimum = 0;
    sliderOpt.maximum = 100;
    sliderOpt.pageStep = 10;
    sliderOpt.sliderPosition = 50;
    sliderOpt.state = QStyle::State_Enabled;
    sub(QStyle::CC_Slider, &sliderOpt, QStyle::SC_SliderGroove);
    sub(QStyle::CC_Slider, &sliderOpt, QStyle::SC_SliderHandle);
    sub(QStyle::CC_Slider, &sliderOpt, QStyle::SC_SliderTickmarks);

    QStyleOptionSpinBox spinOpt;
    spinOpt.rect = QRect(0, 0, 150, 30);
    spinOpt.state = QStyle::State_Enabled;
    spinOpt.frame = true;
    spinOpt.buttonSymbols = QAbstractSpinBox::UpDownArrows;
    sub(QStyle::CC_SpinBox, &spinOpt, QStyle::SC_SpinBoxFrame);
    sub(QStyle::CC_SpinBox, &spinOpt, QStyle::SC_SpinBoxEditField);
    sub(QStyle::CC_SpinBox, &spinOpt, QStyle::SC_SpinBoxUp);
    sub(QStyle::CC_SpinBox, &spinOpt, QStyle::SC_SpinBoxDown);

    QStyleOptionToolButton toolOpt;
    toolOpt.rect = QRect(0, 0, 80, 32);
    toolOpt.state = QStyle::State_Enabled;
    toolOpt.features = QStyleOptionToolButton::HasMenu;
    sub(QStyle::CC_ToolButton, &toolOpt, QStyle::SC_ToolButton);
    sub(QStyle::CC_ToolButton, &toolOpt, QStyle::SC_ToolButtonMenu);
}

void TestUKUIConfigStyleOffscreen::testSubElementRectComprehensive()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;

    QStyleOptionTab tabOption;
    tabOption.direction = Qt::LeftToRight;
    tabOption.rect = QRect(0, 0, 100, 30);
    tabOption.shape = QTabBar::RoundedNorth;
    tabOption.leftButtonSize = QSize(10, 10);
    tabOption.rightButtonSize = QSize(10, 10);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabBarScrollLeftButton, &tabOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabBarScrollRightButton, &tabOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabBarTabLeftButton, &tabOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabBarTabRightButton, &tabOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabBarTabText, &tabOption, &widget);

    QStyleOptionTabWidgetFrame twf;
    twf.rect = QRect(0, 0, 240, 160);
    twf.shape = QTabBar::RoundedNorth;
    twf.lineWidth = 2;
    twf.tabBarSize = QSize(80, 24);
    twf.leftCornerWidgetSize = QSize(10, 10);
    twf.rightCornerWidgetSize = QSize(10, 10);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetTabPane, &twf, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetTabContents, &twf, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetTabBar, &twf, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetLeftCorner, &twf, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetRightCorner, &twf, &widget);

    QStyleOptionButton buttonOption;
    buttonOption.rect = QRect(0, 0, 80, 28);
    (void)style.subElementRect(UKUIConfigStyle::SE_RadioButtonIndicator, &buttonOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_RadioButtonContents, &buttonOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_CheckBoxIndicator, &buttonOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_CheckBoxContents, &buttonOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_PushButtonContents, &buttonOption, &widget);

    QStyleOptionFrame frameOption;
    frameOption.rect = QRect(0, 0, 120, 30);
    (void)style.subElementRect(UKUIConfigStyle::SE_LineEditContents, &frameOption, &widget);

    QStyleOptionProgressBar pbOption;
    pbOption.rect = QRect(0, 0, 200, 24);
    (void)style.subElementRect(UKUIConfigStyle::SE_ProgressBarGroove, &pbOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_ProgressBarLabel, &pbOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_ProgressBarContents, &pbOption, &widget);

    QStyleOptionHeader headerOption;
    headerOption.rect = QRect(0, 0, 100, 30);
    headerOption.sortIndicator = QStyleOptionHeader::SortDown;
    (void)style.subElementRect(UKUIConfigStyle::SE_HeaderLabel, &headerOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_HeaderArrow, &headerOption, &widget);

    QStyleOptionViewItem viewItemOption;
    viewItemOption.rect = QRect(0, 0, 100, 30);
    (void)style.subElementRect(UKUIConfigStyle::SE_ItemViewItemCheckIndicator, &viewItemOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_ItemViewItemText, &viewItemOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_ItemViewItemDecoration, &viewItemOption, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_ItemViewItemFocusRect, &viewItemOption, &widget);

    viewItemOption.direction = Qt::LeftToRight;
    (void)style.subElementRect(UKUIConfigStyle::SE_TreeViewDisclosureItem, &viewItemOption, &widget);
}

void TestUKUIConfigStyleOffscreen::testPixelMetricsWithWidgetVariants()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;
    QComboBox comboBox;
    comboBox.addItem(QStringLiteral("a"));
    QStyleOption opt;
    opt.rect = QRect(0, 0, 100, 30);

    const QStyle::PixelMetric metrics[] = {
        QStyle::PM_MenuHMargin,
        QStyle::PM_MenuVMargin,
        QStyle::PM_MenuPanelWidth,
        QStyle::PM_MenuDesktopFrameWidth,
        QStyle::PM_SubMenuOverlap,
        QStyle::PM_SliderThickness,
        QStyle::PM_SliderControlThickness,
        QStyle::PM_SliderLength,
        QStyle::PM_SliderTickmarkOffset,
        QStyle::PM_MenuScrollerHeight,
        QStyle::PM_DefaultFrameWidth,
        QStyle::PM_TabBarTabOverlap,
        QStyle::PM_TabBarIconSize,
        QStyle::PM_ProgressBarChunkWidth,
        QStyle::PM_HeaderMarkSize,
        QStyle::PM_ListViewIconSize,
        QStyle::PM_IconViewIconSize,
    };
    for (QStyle::PixelMetric pm : metrics) {
        (void)style.pixelMetric(pm, &opt, &widget);
        (void)style.pixelMetric(pm, &opt, &comboBox);
    }

    QStyleOptionSlider sliderOpt;
    sliderOpt.orientation = Qt::Horizontal;
    sliderOpt.rect = QRect(0, 0, 200, 30);
    sliderOpt.minimum = 0;
    sliderOpt.maximum = 100;
    sliderOpt.pageStep = 10;
    sliderOpt.sliderPosition = 50;
    (void)style.pixelMetric(QStyle::PM_SliderSpaceAvailable, &sliderOpt, &widget);

    QStyleOptionToolButton toolButtonOpt;
    (void)style.pixelMetric(QStyle::PM_DefaultFrameWidth, &toolButtonOpt, &widget);

    QStyleHintReturn hintRet;
    (void)style.styleHint(QStyle::SH_Menu_Scrollable, &opt, &widget, &hintRet);
    (void)style.styleHint(QStyle::SH_ScrollBar_Transient, &opt, &widget, &hintRet);
    (void)style.styleHint(QStyle::SH_ComboBox_Popup, &opt, &comboBox, &hintRet);
}

void TestUKUIConfigStyleOffscreen::testDrawComplexControlsExtended()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget host;
    host.setProperty("drawScrollBarGroove", true);

    auto drawCc = [&](QStyle::ComplexControl cc, QStyleOptionComplex &opt) {
        QImage img = paintBuffer(220, 64);
        QPainter p(&img);
        style.drawComplexControl(cc, &opt, &p, &host);
    };

    QToolButton toolBtn;
    toolBtn.setPopupMode(QToolButton::MenuButtonPopup);
    QMenu menu;
    toolBtn.setMenu(&menu);
    QStyleOptionToolButton tbOpt;
    tbOpt.initFrom(&toolBtn);
    tbOpt.features = QStyleOptionToolButton::HasMenu;
    tbOpt.subControls = QStyle::SC_ToolButton | QStyle::SC_ToolButtonMenu;
    drawCc(QStyle::CC_ToolButton, tbOpt);

    QStyleOptionGroupBox gbOpt;
    gbOpt.state = QStyle::State_Enabled;
    gbOpt.rect = QRect(0, 0, 200, 80);
    gbOpt.text = QStringLiteral("Group");
    drawCc(QStyle::CC_GroupBox, gbOpt);

    QStyleOptionSlider scrollOpt;
    scrollOpt.orientation = Qt::Vertical;
    scrollOpt.rect = QRect(0, 0, 20, 200);
    scrollOpt.minimum = 0;
    scrollOpt.maximum = 200;
    scrollOpt.pageStep = 20;
    scrollOpt.sliderPosition = 80;
    scrollOpt.state = QStyle::State_Enabled;
    scrollOpt.activeSubControls = QStyle::SC_ScrollBarSlider;
    drawCc(QStyle::CC_ScrollBar, scrollOpt);

    QStyleOptionComboBox comboOpt;
    comboOpt.rect = QRect(0, 0, 160, 32);
    comboOpt.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    comboOpt.editable = true;
    comboOpt.frame = true;
    comboOpt.currentText = QStringLiteral("item");
    comboOpt.activeSubControls = QStyle::SC_ComboBoxArrow;
    drawCc(QStyle::CC_ComboBox, comboOpt);
}

void TestUKUIConfigStyleOffscreen::testHitTestAndStandardPixmapVariants()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget w;
    w.resize(120, 80);

    QStyleOptionComplex ccOpt;
    ccOpt.rect = QRect(0, 0, 160, 32);
    ccOpt.state = QStyle::State_Enabled;
    (void)style.hitTestComplexControl(QStyle::CC_ComboBox, &ccOpt, QPoint(140, 16), &w);
    (void)style.hitTestComplexControl(QStyle::CC_Slider, &ccOpt, QPoint(80, 16), &w);
    (void)style.hitTestComplexControl(QStyle::CC_ScrollBar, &ccOpt, QPoint(10, 10), &w);

    for (int sp = int(QStyle::SP_ArrowUp); sp <= int(QStyle::SP_TitleBarShadeButton); ++sp) {
        (void)style.standardIcon(static_cast<QStyle::StandardPixmap>(sp), nullptr, &w);
        (void)style.standardPixmap(static_cast<QStyle::StandardPixmap>(sp), nullptr, &w);
    }

    QStyleOptionButton btnOpt;
    btnOpt.rect = QRect(0, 0, 100, 28);
    btnOpt.state = QStyle::State_Enabled | QStyle::State_Raised;
    (void)style.sizeFromContents(QStyle::CT_MenuItem, &btnOpt, QSize(100, 24), &w);
    (void)style.sizeFromContents(QStyle::CT_HeaderSection, &btnOpt, QSize(80, 24), &w);
    (void)style.sizeFromContents(QStyle::CT_TabBarTab, &btnOpt, QSize(60, 28), &w);
}

void TestUKUIConfigStyleOffscreen::testDrawWithUkuiWidgetProperties()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QImage surface(240, 240, QImage::Format_ARGB32);
    surface.fill(Qt::transparent);
    QPainter painter(&surface);

    auto drawPe = [&](QStyle::PrimitiveElement pe, QStyleOption &opt, QWidget &host) {
        style.drawPrimitive(pe, &opt, &painter, &host);
    };
    auto drawCc = [&](QStyle::ComplexControl cc, QStyleOptionComplex &opt, QWidget &host) {
        style.drawComplexControl(cc, &opt, &painter, &host);
    };

    QWidget scrollHost;
    scrollHost.setProperty("setRadius", 1);
    scrollHost.setProperty("setGrooveDefaultBrush", QBrush(QColor(Qt::red)));
    scrollHost.setProperty("setGrooveInactiveBrush", QBrush(QColor(Qt::red)));
    scrollHost.setProperty("setSliderDefaultBrush", QBrush(QColor(Qt::red)));
    scrollHost.setProperty("setSliderHoverBrush", QBrush(QColor(Qt::red)));
    scrollHost.setProperty("setSliderClickBrush", QBrush(QColor(Qt::red)));
    scrollHost.setProperty("setSliderDisableBrush", QBrush(QColor(Qt::red)));
    QStyleOptionSlider scrollOpt;
    scrollOpt.orientation = Qt::Horizontal;
    scrollOpt.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    scrollOpt.rect = QRect(0, 0, 200, 20);
    scrollOpt.minimum = 0;
    scrollOpt.maximum = 100;
    scrollOpt.sliderPosition = 40;
    drawCc(QStyle::CC_ScrollBar, scrollOpt, scrollHost);

    QWidget sliderHost;
    sliderHost.setProperty("setRadius", 1);
    sliderHost.setProperty("setGrooveRadius", QBrush(QColor(Qt::blue)));
    sliderHost.setProperty("setHandleDefaultBrush", QBrush(QColor(Qt::blue)));
    sliderHost.setProperty("setHandleHoverBrush", QBrush(QColor(Qt::blue)));
    sliderHost.setProperty("setHandleClickBrush", QBrush(QColor(Qt::blue)));
    sliderHost.setProperty("setHandleDisableBrush", QBrush(QColor(Qt::blue)));
    sliderHost.setProperty("setGrooveValueDefaultBrush", QBrush(QColor(Qt::blue)));
    sliderHost.setProperty("setGrooveValueHoverBrush", QBrush(QColor(Qt::blue)));
    sliderHost.setProperty("setGrooveValueDisableBrush", QBrush(QColor(Qt::blue)));
    sliderHost.setProperty("setGrooveUnvalueDefaultBrush", QBrush(QColor(Qt::blue)));
    sliderHost.setProperty("setGrooveUnvalueHoverBrush", QBrush(QColor(Qt::blue)));
    sliderHost.setProperty("setGrooveUnvalueDisableBrush", QBrush(QColor(Qt::blue)));
    QStyleOptionSlider sliderOpt;
    sliderOpt.orientation = Qt::Horizontal;
    sliderOpt.state = QStyle::State_Enabled | QStyle::State_Sunken;
    sliderOpt.rect = QRect(0, 0, 200, 24);
    sliderOpt.minimum = 0;
    sliderOpt.maximum = 100;
    sliderOpt.sliderPosition = 60;
    drawCc(QStyle::CC_Slider, sliderOpt, sliderHost);

    QWidget menuHost;
    menuHost.setProperty("setFrameradius", 1);
    menuHost.setProperty("setItemradius", 1);
    menuHost.setProperty("setMenuBackgroundBrush", QBrush(QColor(Qt::green)));
    menuHost.setProperty("setMenuBackgroundPen", QPen(QColor(Qt::green)));
    menuHost.setProperty("setItemSelectBrush", QBrush(QColor(Qt::green)));
    menuHost.setProperty("setMenuHoverPen", QPen(QColor(Qt::green)));
    menuHost.setProperty("setItemFocusPen", QPen(QColor(Qt::green)));
    QStyleOption menuOpt;
    menuOpt.rect = QRect(0, 0, 120, 120);
    menuOpt.state = QStyle::State_Enabled;
    menuOpt.palette = style.standardPalette();
    drawPe(QStyle::PE_PanelMenu, menuOpt, menuHost);
    drawPe(QStyle::PE_FrameMenu, menuOpt, menuHost);

    QWidget lineHost;
    lineHost.setProperty("setLineEditRadius", 6);
    lineHost.setProperty("setLineEditBackgroundBrush", QBrush(QColor(0xF0, 0xF0, 0xF0)));
    lineHost.setProperty("setLineEditBorderPen", QPen(QColor(Qt::gray)));
    QStyleOptionFrame frameOpt;
    frameOpt.rect = QRect(0, 0, 160, 28);
    frameOpt.state = QStyle::State_Enabled | QStyle::State_HasFocus;
    drawPe(QStyle::PE_PanelLineEdit, frameOpt, lineHost);

    QWidget btnHost;
    btnHost.setProperty("setButtonRadius", 8);
    btnHost.setProperty("setButtonBackgroundBrush", QBrush(QColor(Qt::cyan)));
    QStyleOptionButton btnOpt;
    btnOpt.rect = QRect(0, 0, 100, 32);
    btnOpt.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    btnOpt.features = QStyleOptionButton::DefaultButton;
    drawPe(QStyle::PE_PanelButtonCommand, btnOpt, btnHost);

    QStyleOptionTabWidgetFrame tabFrame;
    tabFrame.rect = QRect(0, 0, 200, 120);
    tabFrame.shape = QTabBar::RoundedNorth;
    drawPe(QStyle::PE_FrameTabWidget, tabFrame, menuHost);

    QStyleOptionViewItem viewOpt;
    viewOpt.rect = QRect(0, 0, 120, 24);
    viewOpt.state = QStyle::State_Enabled | QStyle::State_Selected;
    viewOpt.features = QStyleOptionViewItem::HasCheckIndicator;
    drawPe(QStyle::PE_IndicatorViewItemCheck, viewOpt, menuHost);

    QWidget comboHost;
    comboHost.setProperty("setRadius", 1);
    comboHost.setProperty("setDefaultBrush", QBrush(QColor(Qt::red)));
    comboHost.setProperty("setOnBrush", QBrush(QColor(Qt::red)));
    comboHost.setProperty("setHoverBrush", QBrush(QColor(Qt::red)));
    comboHost.setProperty("setDisableBrush", QBrush(QColor(Qt::red)));
    comboHost.setProperty("setEditBrush", QBrush(QColor(Qt::red)));
    comboHost.setProperty("setDefaultPen", QPen(QColor(Qt::red)));
    comboHost.setProperty("setHoverPen", QPen(QColor(Qt::red)));
    comboHost.setProperty("setDisablePen", QPen(QColor(Qt::red)));
    comboHost.setProperty("setOnPen", QPen(QColor(Qt::red)));
    comboHost.setProperty("setEditPen", QPen(QColor(Qt::red)));
    comboHost.setProperty("setFocusPen", QPen(QColor(Qt::red)));
    QStyleOptionComboBox comboOpt;
    comboOpt.rect = QRect(0, 0, 160, 32);
    comboOpt.state = QStyle::State_Enabled;
    drawCc(QStyle::CC_ComboBox, comboOpt, comboHost);

    QWidget spinHost;
    spinHost.setProperty("setRadius", 1);
    spinHost.setProperty("setDefaultBrush", QBrush(QColor(Qt::red)));
    spinHost.setProperty("setFocusBrush", QBrush(QColor(Qt::red)));
    spinHost.setProperty("setHoverBrush", QBrush(QColor(Qt::red)));
    spinHost.setProperty("setDisableBrush", QBrush(QColor(Qt::red)));
    spinHost.setProperty("setUpDefaultBrush", QBrush(QColor(Qt::red)));
    spinHost.setProperty("setDownDefaultBrush", QBrush(QColor(Qt::red)));
    QStyleOptionSpinBox spinOpt;
    spinOpt.rect = QRect(0, 0, 150, 30);
    spinOpt.state = QStyle::State_Enabled;
    drawCc(QStyle::CC_SpinBox, spinOpt, spinHost);
}

void TestUKUIConfigStyleOffscreen::testDrawPrimitivePropertyPaths()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QImage surface(128, 128, QImage::Format_ARGB32);
    surface.fill(Qt::transparent);
    QPainter painter(&surface);
    QWidget widget;

    auto drawPe = [&](QStyle::PrimitiveElement pe, QStyleOption &opt) {
        style.drawPrimitive(pe, &opt, &painter, &widget);
    };

    widget.setProperty("setRadius", 1);
    widget.setProperty("setDefaultBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setHoverBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setSelectBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setDisableBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setBranchDefaultBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setBranchHoverBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setBranchSelectBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setTextHoverPen", QPen(QColor(Qt::red)));
    widget.setProperty("setTextSelectPen", QPen(QColor(Qt::red)));
    widget.setProperty("setHoverPen", QPen(QColor(Qt::red)));
    widget.setProperty("setSelectPen", QPen(QColor(Qt::red)));
    widget.setProperty("setFocusPen", QPen(QColor(Qt::red)));
    QStyleOption focusOpt;
    focusOpt.rect = QRect(0, 0, 100, 100);
    focusOpt.state = QStyle::State_HasFocus;
    focusOpt.palette = style.standardPalette();
    drawPe(QStyle::PE_FrameFocusRect, focusOpt);

    QStyleOption baseOpt;
    baseOpt.rect = QRect(0, 0, 100, 100);
    baseOpt.state = QStyle::State_Enabled;
    baseOpt.palette = style.standardPalette();
    drawPe(QStyle::PE_Frame, baseOpt);
    drawPe(QStyle::PE_FrameStatusBar, baseOpt);
    drawPe(QStyle::PE_IndicatorButtonDropDown, baseOpt);
    drawPe(QStyle::PE_IndicatorTabClose, baseOpt);
    drawPe(QStyle::PE_FrameGroupBox, baseOpt);
    drawPe(QStyle::PE_PanelScrollAreaCorner, baseOpt);
    drawPe(QStyle::PE_IndicatorTabTearLeft, baseOpt);
    drawPe(QStyle::PE_IndicatorTabTearRight, baseOpt);

    widget.setProperty("setBackgroundBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setBackgroundPen", QPen(QColor(Qt::red)));
    drawPe(QStyle::PE_PanelTipLabel, baseOpt);

    QStyleOptionButton toolBtnOpt;
    toolBtnOpt.rect = QRect(0, 0, 100, 100);
    toolBtnOpt.state = QStyle::State_Enabled | QStyle::State_On;
    toolBtnOpt.palette = style.standardPalette();
    widget.setProperty("setClickBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setCheckBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setCheckHoverBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setDefaultPen", QPen(QColor(Qt::red)));
    widget.setProperty("setClickPen", QPen(QColor(Qt::red)));
    drawPe(QStyle::PE_PanelButtonTool, toolBtnOpt);

    QStyleOptionTabBarBase tabBarBase;
    tabBarBase.tabBarRect = QRect(0, 0, 100, 30);
    tabBarBase.state = QStyle::State_Enabled;
    tabBarBase.palette = style.standardPalette();
    drawPe(QStyle::PE_FrameTabBarBase, tabBarBase);

    QStyleOptionTabWidgetFrame tabWidgetFrame;
    tabWidgetFrame.rect = QRect(0, 0, 100, 100);
    tabWidgetFrame.state = QStyle::State_Enabled;
    tabWidgetFrame.palette = style.standardPalette();
    widget.setProperty("setWidgetBackgroundBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setTabBarBackgroundBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setTabBarDefaultBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setTabBarHoverBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setTabBarSelectBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setTabBarDefaultPen", QPen(QColor(Qt::red)));
    drawPe(QStyle::PE_FrameTabWidget, tabWidgetFrame);

    QStyleOptionFrame lineFrame;
    lineFrame.rect = QRect(0, 0, 100, 28);
    lineFrame.state = QStyle::State_Enabled | QStyle::State_HasFocus;
    lineFrame.palette = style.standardPalette();
    widget.setProperty("setFocusBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setDefaultPen", QPen(QColor(Qt::red)));
    drawPe(QStyle::PE_PanelLineEdit, lineFrame);

    const QStyle::PrimitiveElement arrows[] = {
        QStyle::PE_IndicatorArrowUp,
        QStyle::PE_IndicatorArrowDown,
        QStyle::PE_IndicatorArrowRight,
        QStyle::PE_IndicatorArrowLeft,
    };
    for (QStyle::PrimitiveElement pe : arrows)
        drawPe(pe, baseOpt);

    QStyleOptionButton radioOpt;
    radioOpt.rect = QRect(0, 0, 24, 24);
    radioOpt.state = QStyle::State_Enabled | QStyle::State_On;
    radioOpt.palette = style.standardPalette();
    widget.setProperty("setOnDefaultBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setOnHoverBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setOnClickBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setOnDefaultPen", QPen(QColor(Qt::red)));
    drawPe(QStyle::PE_IndicatorRadioButton, radioOpt);

    QStyleOptionButton checkOpt;
    checkOpt.rect = QRect(0, 0, 24, 24);
    checkOpt.state = QStyle::State_Enabled | QStyle::State_On;
    checkOpt.palette = style.standardPalette();
    widget.setProperty("setOnDefaultBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setPathBrush", QBrush(QColor(Qt::red)));
    widget.setProperty("setContentPen", QPen(QColor(Qt::red)));
    widget.setProperty("setOnHoverBrushIslinearGradient", true);
    drawPe(QStyle::PE_IndicatorCheckBox, checkOpt);

    QStyleOptionSlider dialOpt;
    dialOpt.rect = QRect(0, 0, 80, 80);
    dialOpt.state = QStyle::State_Enabled;
    dialOpt.minimum = 0;
    dialOpt.maximum = 100;
    dialOpt.sliderPosition = 30;
    style.drawComplexControl(QStyle::CC_Dial, &dialOpt, &painter, &widget);
}

void TestUKUIConfigStyleOffscreen::testDrawItemViewTreeTableList()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QImage surface(200, 200, QImage::Format_ARGB32);
    surface.fill(Qt::transparent);
    QPainter painter(&surface);

    auto drawItem = [&](QStyleOptionViewItem &opt, QWidget *w) {
        style.drawControl(QStyle::CE_ItemViewItem, &opt, &painter, w);
    };

    QTreeWidget tree;
    tree.setHeaderHidden(true);
    tree.addTopLevelItem(new QTreeWidgetItem(QStringList(QStringLiteral("node"))));
    tree.resize(180, 160);

    const QStyle::State states[] = {
        QStyle::State_Enabled,
        QStyle::State_Enabled | QStyle::State_Selected,
        QStyle::State_Enabled | QStyle::State_MouseOver,
        QStyle::State_Enabled | QStyle::State_Sunken,
    };
    const QStyleOptionViewItem::ViewItemPosition positions[] = {
        QStyleOptionViewItem::Beginning,
        QStyleOptionViewItem::Middle,
        QStyleOptionViewItem::End,
        QStyleOptionViewItem::OnlyOne,
    };

    for (QStyle::State st : states) {
        for (QStyleOptionViewItem::ViewItemPosition pos : positions) {
            QStyleOptionViewItem opt;
            opt.rect = QRect(0, 0, 180, 28);
            opt.state = st;
            opt.viewItemPosition = pos;
            opt.text = QStringLiteral("item");
            opt.index = tree.model()->index(0, 0);
            opt.widget = &tree;
            drawItem(opt, &tree);
        }
    }

    QListView list;
    list.setModel(tree.model());
    list.resize(180, 120);
    QStyleOptionViewItem listOpt;
    listOpt.rect = QRect(0, 0, 180, 24);
    listOpt.state = QStyle::State_Enabled | QStyle::State_Selected;
    listOpt.widget = &list;
    drawItem(listOpt, &list);

    QTableWidget table(2, 2);
    table.resize(180, 80);
    QStyleOptionViewItem tableOpt;
    tableOpt.rect = QRect(0, 0, 80, 24);
    tableOpt.state = QStyle::State_Enabled;
    tableOpt.widget = &table;
    drawItem(tableOpt, &table);

    QStyleOptionViewItem branchOpt;
    branchOpt.rect = QRect(0, 0, 20, 28);
    branchOpt.state = QStyle::State_Enabled;
    style.drawPrimitive(QStyle::PE_IndicatorBranch, &branchOpt, &painter, &tree);
    branchOpt.rect.setX(0);
    style.drawPrimitive(QStyle::PE_IndicatorBranch, &branchOpt, &painter, &tree);

    QHeaderView header(Qt::Horizontal);
    header.resize(180, 28);
    QStyleOptionHeader hdrOpt;
    hdrOpt.rect = QRect(0, 0, 80, 28);
    hdrOpt.state = QStyle::State_Enabled | QStyle::State_Sunken;
    hdrOpt.sortIndicator = QStyleOptionHeader::SortUp;
    hdrOpt.text = QStringLiteral("Col");
    style.drawControl(QStyle::CE_Header, &hdrOpt, &painter, &header);
    style.drawControl(QStyle::CE_HeaderSection, &hdrOpt, &painter, &header);
}

void TestUKUIConfigStyleOffscreen::testDrawComplexControlFullProperties()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QImage surface(240, 240, QImage::Format_ARGB32);
    surface.fill(Qt::transparent);
    QPainter painter(&surface);

    auto drawCc = [&](QStyle::ComplexControl cc, QStyleOptionComplex &opt, QWidget &host) {
        style.drawComplexControl(cc, &opt, &painter, &host);
    };

    QWidget sliderHost;
    sliderHost.setProperty("setRadius", 1);
    sliderHost.setProperty("setGrooveRadius", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setHandleDefaultBrush", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setHandleHoverBrush", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setHandleClickBrush", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setHandleDisableBrush", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setGrooveValueDefaultBrush", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setGrooveValueHoverBrush", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setGrooveValueDisableBrush", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setGrooveUnvalueDefaultBrush", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setGrooveUnvalueHoverBrush", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setGrooveUnvalueDisableBrush", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setHandleDefaultPen", QPen(QColor(Qt::red)));
    sliderHost.setProperty("setHandleHoverPen", QPen(QColor(Qt::red)));
    sliderHost.setProperty("setHandleClickPen", QPen(QColor(Qt::red)));
    sliderHost.setProperty("setHandleDisablePen", QPen(QColor(Qt::red)));
    sliderHost.setProperty("setSliderGroovePen", QPen(QColor(Qt::red)));
    sliderHost.setProperty("setFocusPen", QPen(QColor(Qt::red)));

    const QStyle::State sliderStates[] = {
        QStyle::State_Enabled,
        QStyle::State_Enabled | QStyle::State_MouseOver,
        QStyle::State_Enabled | QStyle::State_Sunken,
        QStyle::State_None,
    };
    for (QStyle::State st : sliderStates) {
        QStyleOptionSlider hSlider;
        hSlider.orientation = Qt::Horizontal;
        hSlider.rect = QRect(0, 0, 200, 24);
        hSlider.minimum = 0;
        hSlider.maximum = 100;
        hSlider.pageStep = 10;
        hSlider.sliderPosition = 55;
        hSlider.state = st;
        drawCc(QStyle::CC_Slider, hSlider, sliderHost);

        QStyleOptionSlider vSlider = hSlider;
        vSlider.orientation = Qt::Vertical;
        vSlider.rect = QRect(0, 0, 24, 200);
        drawCc(QStyle::CC_Slider, vSlider, sliderHost);
    }

    QWidget scrollHost;
    scrollHost.setProperty("setRadius", 1);
    scrollHost.setProperty("setGrooveDefaultBrush", QBrush(QColor(Qt::blue)));
    scrollHost.setProperty("setGrooveInactiveBrush", QBrush(QColor(Qt::blue)));
    scrollHost.setProperty("setSliderDefaultBrush", QBrush(QColor(Qt::blue)));
    scrollHost.setProperty("setSliderHoverBrush", QBrush(QColor(Qt::blue)));
    scrollHost.setProperty("setSliderClickBrush", QBrush(QColor(Qt::blue)));
    scrollHost.setProperty("setSliderDisableBrush", QBrush(QColor(Qt::blue)));
    scrollHost.setProperty("drawScrollBarGroove", true);

    QStyleOptionSlider vScroll;
    vScroll.orientation = Qt::Vertical;
    vScroll.rect = QRect(0, 0, 20, 220);
    vScroll.minimum = 0;
    vScroll.maximum = 200;
    vScroll.pageStep = 20;
    vScroll.sliderPosition = 90;
    vScroll.state = QStyle::State_Enabled | QStyle::State_Active;
    vScroll.activeSubControls = QStyle::SC_ScrollBarSlider;
    drawCc(QStyle::CC_ScrollBar, vScroll, scrollHost);

    QStyleOptionGroupBox gbOpt;
    gbOpt.rect = QRect(0, 0, 200, 100);
    gbOpt.state = QStyle::State_Enabled;
    gbOpt.text = QStringLiteral("Group title");
    gbOpt.lineWidth = 2;
    drawCc(QStyle::CC_GroupBox, gbOpt, scrollHost);

    QStyleOptionToolButton tbOpt;
    tbOpt.rect = QRect(0, 0, 90, 32);
    tbOpt.state = QStyle::State_Enabled | QStyle::State_On;
    tbOpt.features = QStyleOptionToolButton::HasMenu | QStyleOptionToolButton::MenuButtonPopup;
    tbOpt.subControls = QStyle::SC_ToolButton | QStyle::SC_ToolButtonMenu;
    drawCc(QStyle::CC_ToolButton, tbOpt, scrollHost);
}

void TestUKUIConfigStyleOffscreen::testSizeFromContentsUkuiTypes()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;
    const QSize base(100, 24);

    QStyleOptionMenuItem menuItem;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_MenuItem, &menuItem, base, &widget);

    QStyleOptionFrame frameOpt;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_LineEdit, &frameOpt, base, &widget);

    QStyleOptionTab tabOpt;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_TabBarTab, &tabOpt, base, &widget);

    QStyleOptionButton btnOpt;
    btnOpt.text = QStringLiteral("btn");
    (void)style.sizeFromContents(UKUIConfigStyle::CT_RadioButton, &btnOpt, base, &widget);
    (void)style.sizeFromContents(UKUIConfigStyle::CT_CheckBox, &btnOpt, base, &widget);
    (void)style.sizeFromContents(UKUIConfigStyle::CT_PushButton, &btnOpt, base, &widget);

    QStyleOptionToolButton toolOpt;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_ToolButton, &toolOpt, base, &widget);

    QStyleOptionComboBox comboOpt;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_ComboBox, &comboOpt, base, &widget);

    QStyleOptionSpinBox spinOpt;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_SpinBox, &spinOpt, base, &widget);

    QStyleOptionProgressBar pbOpt;
    pbOpt.text = QStringLiteral("50%");
    (void)style.sizeFromContents(UKUIConfigStyle::CT_ProgressBar, &pbOpt, base, &widget);

    QStyleOptionSlider sliderOpt;
    sliderOpt.orientation = Qt::Horizontal;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_Slider, &sliderOpt, base, &widget);

    QStyleOptionHeader headerOpt;
    headerOpt.text = QStringLiteral("H");
    (void)style.sizeFromContents(UKUIConfigStyle::CT_HeaderSection, &headerOpt, base, &widget);

    QStyleOptionViewItem viewOpt;
    viewOpt.text = QStringLiteral("row");
    (void)style.sizeFromContents(UKUIConfigStyle::CT_ItemViewItem, &viewOpt, base, &widget);
}

void TestUKUIConfigStyleOffscreen::testSubElementRectRtlAndVariants()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;

    QStyleOptionTab tabRtl;
    tabRtl.direction = Qt::RightToLeft;
    tabRtl.rect = QRect(0, 0, 120, 32);
    tabRtl.shape = QTabBar::RoundedSouth;
    tabRtl.leftButtonSize = QSize(12, 12);
    tabRtl.rightButtonSize = QSize(12, 12);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabBarScrollLeftButton, &tabRtl, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabBarScrollRightButton, &tabRtl, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabBarTabLeftButton, &tabRtl, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabBarTabRightButton, &tabRtl, &widget);

    QStyleOptionTabWidgetFrame twf;
    twf.direction = Qt::RightToLeft;
    twf.rect = QRect(0, 0, 260, 180);
    twf.shape = QTabBar::RoundedSouth;
    twf.lineWidth = 3;
    twf.tabBarSize = QSize(100, 28);
    twf.leftCornerWidgetSize = QSize(16, 16);
    twf.rightCornerWidgetSize = QSize(16, 16);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetTabPane, &twf, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetTabBar, &twf, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetLeftCorner, &twf, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TabWidgetRightCorner, &twf, &widget);

    QStyleOptionHeader headerUp;
    headerUp.rect = QRect(0, 0, 100, 30);
    headerUp.sortIndicator = QStyleOptionHeader::SortUp;
    (void)style.subElementRect(UKUIConfigStyle::SE_HeaderArrow, &headerUp, &widget);

    QStyleOptionHeader headerDown;
    headerDown.rect = QRect(0, 0, 100, 30);
    headerDown.sortIndicator = QStyleOptionHeader::SortDown;
    (void)style.subElementRect(UKUIConfigStyle::SE_HeaderArrow, &headerDown, &widget);

    QStyleOptionProgressBar pb;
    pb.rect = QRect(0, 0, 200, 24);
    pb.text = QStringLiteral("progress");
    pb.textVisible = true;
    (void)style.subElementRect(UKUIConfigStyle::SE_ProgressBarLabel, &pb, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_ProgressBarContents, &pb, &widget);

    QStyleOptionViewItem viewRtl;
    viewRtl.direction = Qt::RightToLeft;
    viewRtl.rect = QRect(0, 0, 120, 28);
    viewRtl.features = QStyleOptionViewItem::HasCheckIndicator | QStyleOptionViewItem::HasDisplay;
    (void)style.subElementRect(UKUIConfigStyle::SE_ItemViewItemCheckIndicator, &viewRtl, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_ItemViewItemDecoration, &viewRtl, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_ItemViewItemText, &viewRtl, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_ItemViewItemFocusRect, &viewRtl, &widget);
    (void)style.subElementRect(UKUIConfigStyle::SE_TreeViewDisclosureItem, &viewRtl, &widget);
}

void TestUKUIConfigStyleOffscreen::testDrawListTableItemViewDeep()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QImage surface(320, 480, QImage::Format_ARGB32);
    surface.fill(Qt::transparent);
    QPainter painter(&surface);

    auto drawCe = [&](const QStyleOptionViewItem &opt, QWidget *w) {
        style.drawControl(QStyle::CE_ItemViewItem, &opt, &painter, w);
    };
    auto drawPeRow = [&](const QStyleOptionViewItem &opt, QWidget *w) {
        style.drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, &painter, w);
    };

    QListWidget listWidget;
    listWidget.setAlternatingRowColors(true);
    listWidget.setSelectionMode(QAbstractItemView::ExtendedSelection);
    for (int i = 0; i < 5; ++i)
        listWidget.addItem(QStringLiteral("row-%1").arg(i));
    listWidget.resize(280, 200);
    listWidget.setCurrentRow(0);

    for (int row = 0; row < listWidget.count(); ++row) {
        QStyleOptionViewItem rowOpt;
        rowOpt.rect = listWidget.visualRect(listWidget.model()->index(row, 0));
        rowOpt.state = QStyle::State_Enabled;
        rowOpt.widget = &listWidget;
        if (row % 2 == 1)
            rowOpt.features |= QStyleOptionViewItem::Alternate;
        drawPeRow(rowOpt, &listWidget);
    }

    listWidget.clearSelection();
    for (int row = 0; row < 3; ++row)
        listWidget.item(row)->setSelected(true);

    for (int row = 0; row < listWidget.count(); ++row) {
        QStyleOptionViewItem opt;
        opt.rect = listWidget.visualRect(listWidget.model()->index(row, 0));
        opt.state = QStyle::State_Enabled;
        opt.text = listWidget.item(row)->text();
        opt.index = listWidget.model()->index(row, 0);
        opt.widget = &listWidget;
        if (listWidget.item(row)->isSelected())
            opt.state |= QStyle::State_Selected;
        if (row == 1)
            opt.state |= QStyle::State_MouseOver | QStyle::State_HasFocus;
        drawCe(opt, &listWidget);
    }

    QListWidget singleList;
    singleList.addItem(QStringLiteral("only"));
    singleList.resize(200, 48);
    singleList.item(0)->setSelected(true);
    QStyleOptionViewItem singleOpt;
    singleOpt.rect = singleList.visualRect(singleList.model()->index(0, 0));
    singleOpt.state = QStyle::State_Enabled | QStyle::State_Selected;
    singleOpt.text = singleList.item(0)->text();
    singleOpt.index = singleList.model()->index(0, 0);
    singleOpt.widget = &singleList;
    drawCe(singleOpt, &singleList);

    QListView iconList;
    QStandardItemModel iconModel(3, 1);
    for (int i = 0; i < 3; ++i) {
        auto *item = new QStandardItem(QIcon::fromTheme(QStringLiteral("folder")), QStringLiteral("I%1").arg(i));
        iconModel.setItem(i, 0, item);
    }
    iconList.setModel(&iconModel);
    iconList.setViewMode(QListView::IconMode);
    iconList.setGridSize(QSize(88, 96));
    iconList.resize(280, 120);
    iconList.setCurrentIndex(iconModel.index(1, 0));
    iconList.selectionModel()->select(iconModel.index(1, 0), QItemSelectionModel::Select);

    QStyleOptionViewItem iconOpt;
    iconOpt.rect = iconList.visualRect(iconModel.index(1, 0));
    iconOpt.state = QStyle::State_Enabled | QStyle::State_Selected;
    iconOpt.decorationPosition = QStyleOptionViewItem::Top;
    iconOpt.decorationSize = QSize(48, 48);
    iconOpt.icon = iconModel.item(1)->icon();
    iconOpt.text = iconModel.item(1)->text();
    iconOpt.index = iconModel.index(1, 0);
    iconOpt.widget = &iconList;
    drawCe(iconOpt, &iconList);

    iconOpt.decorationPosition = QStyleOptionViewItem::Bottom;
    iconOpt.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    drawCe(iconOpt, &iconList);

    QTableWidget table(4, 3);
    table.setAlternatingRowColors(true);
    table.setHorizontalHeaderLabels({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")});
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 3; ++c)
            table.setItem(r, c, new QTableWidgetItem(QStringLiteral("%1-%2").arg(r).arg(c)));
    }
    table.resize(280, 140);
    table.selectRow(1);

    for (int r = 0; r < table.rowCount(); ++r) {
        QStyleOptionViewItem rowOpt;
        rowOpt.rect = table.visualRect(table.model()->index(r, 0));
        rowOpt.state = QStyle::State_Enabled;
        rowOpt.widget = &table;
        if (r % 2 == 1)
            rowOpt.features |= QStyleOptionViewItem::Alternate;
        drawPeRow(rowOpt, &table);
    }

    for (int r = 0; r < table.rowCount(); ++r) {
        for (int c = 0; c < table.columnCount(); ++c) {
            QStyleOptionViewItem cellOpt;
            cellOpt.rect = table.visualRect(table.model()->index(r, c));
            cellOpt.state = QStyle::State_Enabled;
            cellOpt.text = table.item(r, c)->text();
            cellOpt.index = table.model()->index(r, c);
            cellOpt.widget = &table;
            if (table.item(r, c)->isSelected())
                cellOpt.state |= QStyle::State_Selected;
            if (r == 1 && c == 1)
                cellOpt.state |= QStyle::State_MouseOver | QStyle::State_HasFocus;
            drawCe(cellOpt, &table);
        }
    }

    QListWidget disabledList;
    disabledList.addItem(QStringLiteral("off"));
    disabledList.setEnabled(false);
    disabledList.resize(160, 40);
    QStyleOptionViewItem disOpt;
    disOpt.rect = disabledList.visualRect(disabledList.model()->index(0, 0));
    disOpt.state = QStyle::State_None;
    disOpt.widget = &disabledList;
    drawCe(disOpt, &disabledList);

    listWidget.setFocus(Qt::OtherFocusReason);
    QStyleOptionViewItem focusListOpt;
    focusListOpt.rect = listWidget.visualRect(listWidget.model()->index(1, 0));
    focusListOpt.state = QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_HasFocus;
    focusListOpt.text = listWidget.item(1)->text();
    focusListOpt.index = listWidget.model()->index(1, 0);
    focusListOpt.widget = &listWidget;
    drawCe(focusListOpt, &listWidget);

    iconList.setProperty("highlightMode", true);
    QStyleOptionViewItem hlOpt = iconOpt;
    hlOpt.state = QStyle::State_Enabled | QStyle::State_Selected;
    drawCe(hlOpt, &iconList);

    QStyleOptionViewItem checkOpt;
    checkOpt.rect = listWidget.visualRect(listWidget.model()->index(0, 0));
    checkOpt.state = QStyle::State_Enabled;
    checkOpt.features |= QStyleOptionViewItem::HasCheckIndicator;
    checkOpt.checkState = Qt::PartiallyChecked;
    checkOpt.widget = &listWidget;
    checkOpt.index = listWidget.model()->index(0, 0);
    drawCe(checkOpt, &listWidget);

    QStyleOptionViewItem bgOpt;
    bgOpt.rect = iconList.visualRect(iconModel.index(0, 0));
    bgOpt.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    bgOpt.decorationPosition = QStyleOptionViewItem::Top;
    bgOpt.decorationSize = QSize(48, 48);
    bgOpt.icon = iconModel.item(0)->icon();
    bgOpt.text = iconModel.item(0)->text();
    bgOpt.backgroundBrush = QBrush(QColor(220, 220, 220));
    bgOpt.widget = &iconList;
    bgOpt.index = iconModel.index(0, 0);
    drawCe(bgOpt, &iconList);
}

void TestUKUIConfigStyleOffscreen::testItemViewExhaustiveStates()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QImage surface(420, 520, QImage::Format_ARGB32);
    surface.fill(Qt::transparent);
    QPainter painter(&surface);
    const QIcon folderIcon = QIcon::fromTheme(QStringLiteral("folder"));

    const QStyle::State stateMatrix[] = {
        QStyle::State_Enabled,
        QStyle::State_Enabled | QStyle::State_Selected,
        QStyle::State_Enabled | QStyle::State_MouseOver,
        QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_MouseOver,
        QStyle::State_Enabled | QStyle::State_HasFocus,
        QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_HasFocus,
        QStyle::State_None,
    };
    const QStyleOptionViewItem::ViewItemPosition positions[] = {
        QStyleOptionViewItem::Beginning,
        QStyleOptionViewItem::Middle,
        QStyleOptionViewItem::End,
        QStyleOptionViewItem::OnlyOne,
    };

    auto drawItemOn = [&](QStyleOptionViewItem &opt, QWidget *w) {
        style.drawControl(QStyle::CE_ItemViewItem, &opt, &painter, w);
    };

    QTreeWidget tree;
    tree.setHeaderHidden(true);
    QTreeWidgetItem *root = new QTreeWidgetItem(&tree, {QStringLiteral("root")});
    new QTreeWidgetItem(root, {QStringLiteral("child-a")});
    QTreeWidgetItem *childB = new QTreeWidgetItem(root, {QStringLiteral("child-b")});
    new QTreeWidgetItem(childB, {QStringLiteral("leaf-b1")});
    tree.expandAll();
    tree.resize(360, 240);
    tree.setAlternatingRowColors(true);
    tree.setFocus();

    const auto walkTree = [&](const std::function<void(const QModelIndex &)> &fn) {
        std::function<void(const QModelIndex &)> dfs = [&](const QModelIndex &parent) {
            const int rows = tree.model()->rowCount(parent);
            for (int r = 0; r < rows; ++r) {
                const QModelIndex idx = tree.model()->index(r, 0, parent);
                fn(idx);
                if (tree.model()->hasChildren(idx))
                    dfs(idx);
            }
        };
        dfs(QModelIndex());
    };

    walkTree([&](const QModelIndex &idx) {
        QStyleOptionViewItem opt;
        opt.rect = tree.visualRect(idx);
        if (!opt.rect.isValid())
            return;
        opt.text = tree.model()->data(idx, Qt::DisplayRole).toString();
        opt.index = idx;
        opt.widget = &tree;
        opt.icon = folderIcon;
        opt.decorationSize = QSize(16, 16);
        for (QStyle::State st : stateMatrix) {
            opt.state = st;
            for (QStyleOptionViewItem::ViewItemPosition pos : positions) {
                opt.viewItemPosition = pos;
                drawItemOn(opt, &tree);
            }
        }
        QStyleOptionViewItem rowOpt;
        rowOpt.rect = opt.rect;
        rowOpt.state = QStyle::State_Enabled;
        rowOpt.widget = &tree;
        rowOpt.features |= QStyleOptionViewItem::Alternate;
        style.drawPrimitive(QStyle::PE_PanelItemViewRow, &rowOpt, &painter, &tree);
    });

    tree.selectionModel()->select(tree.model()->index(0, 0), QItemSelectionModel::ClearAndSelect);
    tree.selectionModel()->select(tree.model()->index(1, 0), QItemSelectionModel::Select);
    QStyleOptionViewItem selHover;
    selHover.rect = tree.visualRect(tree.model()->index(1, 0));
    selHover.state = QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_MouseOver;
    selHover.viewItemPosition = QStyleOptionViewItem::Middle;
    selHover.text = QStringLiteral("child-a");
    selHover.index = tree.model()->index(1, 0);
    selHover.widget = &tree;
    drawItemOn(selHover, &tree);
}

void TestUKUIConfigStyleOffscreen::testDrawControlRichBulk()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;
    widget.resize(200, 200);
    const QIcon systemIcon = QIcon::fromTheme(QStringLiteral("application-x-desktop"));

    auto drawCe = [&](QStyle::ControlElement ce, QStyleOption &opt) {
        QImage img = paintBuffer(220, 80);
        QPainter p(&img);
        style.drawControl(ce, &opt, &p, &widget);
    };

    {
        QStyleOptionButton btn;
        btn.state = QStyle::State_Enabled | QStyle::State_Raised;
        btn.text = QStringLiteral("Test");
        btn.icon = systemIcon;
        drawCe(QStyle::CE_PushButton, btn);
    }

    {
        QStyleOptionSlider scroll;
        scroll.orientation = Qt::Horizontal;
        scroll.state = QStyle::State_Enabled | QStyle::State_MouseOver;
        scroll.minimum = 0;
        scroll.maximum = 100;
        scroll.sliderPosition = 40;
        drawCe(QStyle::CE_ScrollBarAddLine, scroll);
        drawCe(QStyle::CE_ScrollBarSubLine, scroll);
        drawCe(QStyle::CE_ScrollBarSlider, scroll);
    }

    {
        QStyleOptionTab tab;
        tab.state = QStyle::State_Selected;
        tab.text = QStringLiteral("Tab");
        tab.icon = systemIcon;
        widget.setProperty("setRadius", 1);
        widget.setProperty("setTabBarSelectBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setTabBarHoverPen", QPen(QColor(Qt::red)));
        drawCe(QStyle::CE_TabBarTabShape, tab);
        drawCe(QStyle::CE_TabBarTabLabel, tab);
        drawCe(QStyle::CE_TabBarTab, tab);
    }

    {
        QStyleOptionProgressBar pb;
        pb.minimum = 0;
        pb.maximum = 100;
        pb.progress = 65;
        pb.textVisible = true;
        pb.text = QStringLiteral("65%");
        drawCe(QStyle::CE_ProgressBar, pb);
        drawCe(QStyle::CE_ProgressBarLabel, pb);
        drawCe(QStyle::CE_ProgressBarContents, pb);
    }

    {
        QStyleOptionHeader hdr;
        hdr.state = QStyle::State_Enabled;
        hdr.text = QStringLiteral("Header");
        hdr.icon = systemIcon;
        hdr.sortIndicator = QStyleOptionHeader::SortDown;
        drawCe(QStyle::CE_Header, hdr);
        drawCe(QStyle::CE_HeaderSection, hdr);
        drawCe(QStyle::CE_HeaderLabel, hdr);
        drawCe(QStyle::CE_HeaderEmptyArea, hdr);
    }

    {
        QStyleOptionMenuItem menu;
        menu.state = QStyle::State_Enabled | QStyle::State_Selected;
        menu.text = QStringLiteral("Menu");
        menu.icon = systemIcon;
        widget.setProperty("setFrameradius", 2);
        widget.setProperty("setItemradius", 2);
        widget.setProperty("setMenuBackgroundBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setItemSelectBrush", QBrush(QColor(Qt::red)));
        drawCe(QStyle::CE_MenuItem, menu);
    }

    {
        QStyleOptionToolButton tool;
        tool.state = QStyle::State_Enabled | QStyle::State_MouseOver;
        tool.text = QStringLiteral("Tool");
        tool.icon = systemIcon;
        widget.setProperty("setIconHighlight", true);
        widget.setProperty("setTextHighlight", true);
        widget.setProperty("setTextDefaultColor", QColor(Qt::red));
        drawCe(QStyle::CE_ToolButtonLabel, tool);
    }

    {
        QStyleOptionButton rtlBtn;
        rtlBtn.state = QStyle::State_Enabled;
        rtlBtn.text = QStringLiteral("RTL");
        rtlBtn.icon = systemIcon;
        QImage ltr(200, 50, QImage::Format_ARGB32);
        QImage rtlImg(200, 50, QImage::Format_ARGB32);
        ltr.fill(Qt::transparent);
        rtlImg.fill(Qt::transparent);
        QPainter ltrP(&ltr);
        QPainter rtlP(&rtlImg);
        QGuiApplication::setLayoutDirection(Qt::LeftToRight);
        style.drawControl(QStyle::CE_PushButton, &rtlBtn, &ltrP, &widget);
        QGuiApplication::setLayoutDirection(Qt::RightToLeft);
        style.drawControl(QStyle::CE_PushButton, &rtlBtn, &rtlP, &widget);
        QGuiApplication::setLayoutDirection(Qt::LeftToRight);
    }

    {
        QStyleOptionFrame frame;
        frame.frameShape = QFrame::Box;
        frame.state = QStyle::State_Raised;
        drawCe(QStyle::CE_ShapedFrame, frame);
    }

    {
        QStyleOptionComboBox combo;
        combo.state = QStyle::State_Enabled;
        combo.currentText = QStringLiteral("Option");
        drawCe(QStyle::CE_ComboBoxLabel, combo);
    }
}

void TestUKUIConfigStyleOffscreen::testDrawControlMegaSuite()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;
    widget.resize(200, 200);
    const QIcon systemIcon = QIcon::fromTheme(QStringLiteral("application-x-desktop"));

    auto drawCe = [&](QStyle::ControlElement ce, QStyleOption &opt) {
        QImage img = paintBuffer(220, 64);
        QPainter p(&img);
        style.drawControl(ce, &opt, &p, &widget);
    };

    {
        QStyleOptionButton check;
        check.state = QStyle::State_Enabled | QStyle::State_On;
        check.text = QStringLiteral("Check");
        check.icon = systemIcon;
        drawCe(QStyle::CE_CheckBox, check);
        drawCe(QStyle::CE_CheckBoxLabel, check);
    }
    {
        QStyleOptionButton radio;
        radio.state = QStyle::State_Enabled | QStyle::State_On;
        radio.text = QStringLiteral("Radio");
        radio.icon = systemIcon;
        drawCe(QStyle::CE_RadioButton, radio);
    }
    {
        QStyleOptionMenuItem menuBar;
        menuBar.state = QStyle::State_Enabled | QStyle::State_Selected;
        menuBar.text = QStringLiteral("MenuBar");
        menuBar.icon = systemIcon;
        drawCe(QStyle::CE_MenuBarItem, menuBar);
    }
    {
        QStyleOptionToolBox toolBox;
        toolBox.state = QStyle::State_Enabled | QStyle::State_Selected;
        toolBox.text = QStringLiteral("ToolBox");
        toolBox.icon = systemIcon;
        drawCe(QStyle::CE_ToolBoxTabShape, toolBox);
        drawCe(QStyle::CE_ToolBoxTabLabel, toolBox);
        drawCe(QStyle::CE_ToolBoxTab, toolBox);
    }
    {
        QStyleOptionProgressBar pb;
        pb.minimum = 0;
        pb.maximum = 100;
        pb.progress = 50;
        pb.textVisible = true;
        pb.text = QStringLiteral("50%");
        drawCe(QStyle::CE_ProgressBarGroove, pb);
        drawCe(QStyle::CE_ProgressBarContents, pb);
        drawCe(QStyle::CE_ProgressBarLabel, pb);
        drawCe(QStyle::CE_ProgressBar, pb);
    }
    {
        QStyleOptionComboBox combo;
        combo.state = QStyle::State_Enabled;
        combo.currentText = QStringLiteral("Combo");
        drawCe(QStyle::CE_ComboBoxLabel, combo);
    }
    {
        QStyleOptionToolButton tool;
        tool.state = QStyle::State_Enabled | QStyle::State_MouseOver;
        tool.text = QStringLiteral("Tool");
        tool.icon = systemIcon;
        widget.setProperty("setIconHighlight", true);
        widget.setProperty("setTextHighlight", true);
        widget.setProperty("setTextHoverColor", QColor(Qt::red));
        drawCe(QStyle::CE_ToolButtonLabel, tool);
    }
    {
        QStyleOptionTab tab;
        tab.state = QStyle::State_Selected;
        tab.text = QStringLiteral("Tab");
        tab.icon = systemIcon;
        widget.setProperty("setWidgetBackgroundBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setTabBarBackgroundBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setTabBarDefaultBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setTabBarHoverBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setTabBarClickBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setTabBarSelectBrush", QBrush(QColor(Qt::red)));
        drawCe(QStyle::CE_TabBarTabShape, tab);
    }
}

void TestUKUIConfigStyleOffscreen::testSizeFromContentsAllTypes()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;
    const QSize base(100, 24);

    QStyleOptionMenuItem menuItem;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_MenuItem, &menuItem, base, &widget);

    QStyleOptionFrame frame;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_LineEdit, &frame, base, &widget);

    QStyleOptionTab tab;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_TabBarTab, &tab, base, &widget);

    QStyleOptionButton radio;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_RadioButton, &radio, base, &widget);

    QStyleOptionButton check;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_CheckBox, &check, base, &widget);

    QStyleOptionToolButton tool;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_ToolButton, &tool, base, &widget);

    QStyleOptionButton push;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_PushButton, &push, base, &widget);

    QStyleOptionComboBox combo;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_ComboBox, &combo, base, &widget);

    QStyleOptionSpinBox spin;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_SpinBox, &spin, base, &widget);

    QStyleOptionProgressBar pb;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_ProgressBar, &pb, base, &widget);

    QStyleOptionSlider slider;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_Slider, &slider, base, &widget);

    QStyleOptionHeader hdr;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_HeaderSection, &hdr, base, &widget);

    QStyleOptionViewItem vi;
    (void)style.sizeFromContents(UKUIConfigStyle::CT_ItemViewItem, &vi, base, &widget);
}

void TestUKUIConfigStyleOffscreen::testComplexControlPropertyPaths()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QWidget widget;

    auto drawCc = [&](QStyle::ComplexControl cc, QStyleOptionComplex &opt) {
        QImage img = paintBuffer(240, 48);
        QPainter p(&img);
        style.drawComplexControl(cc, &opt, &p, &widget);
    };

    {
        QStyleOptionSlider scroll;
        scroll.orientation = Qt::Horizontal;
        scroll.state = QStyle::State_Enabled | QStyle::State_MouseOver;
        scroll.minimum = 0;
        scroll.maximum = 100;
        scroll.sliderPosition = 30;
        widget.setProperty("setRadius", 1);
        widget.setProperty("setGrooveDefaultBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setGrooveInactiveBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setSliderDefaultBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setSliderHoverBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setSliderClickBrush", QBrush(QColor(Qt::red)));
        drawCc(QStyle::CC_ScrollBar, scroll);
    }
    {
        QStyleOptionSlider slider;
        slider.orientation = Qt::Horizontal;
        slider.state = QStyle::State_Enabled;
        slider.minimum = 0;
        slider.maximum = 100;
        slider.sliderPosition = 55;
        widget.setProperty("setGrooveRadius", 2);
        widget.setProperty("setHandleDefaultBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setHandleHoverBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setGrooveValueDefaultBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setGrooveUnvalueDefaultBrush", QBrush(QColor(Qt::red)));
        drawCc(QStyle::CC_Slider, slider);
    }
    {
        QStyleOptionComboBox combo;
        combo.state = QStyle::State_Enabled | QStyle::State_MouseOver;
        combo.currentText = QStringLiteral("x");
        widget.setProperty("setComboRadius", 2);
        drawCc(QStyle::CC_ComboBox, combo);
    }
    {
        QStyleOptionSpinBox spin;
        spin.state = QStyle::State_Enabled;
        drawCc(QStyle::CC_SpinBox, spin);
    }
    {
        QStyleOptionGroupBox group;
        group.state = QStyle::State_Enabled;
        group.text = QStringLiteral("Group");
        drawCc(QStyle::CC_GroupBox, group);
    }
}

void TestUKUIConfigStyleOffscreen::testIconModeSelectedFullPath()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QListWidget iconList;
    iconList.setViewMode(QListView::IconMode);
    iconList.setGridSize(QSize(96, 96));
    iconList.setIconSize(QSize(48, 48));
    iconList.setMovement(QListView::Static);
    iconList.setSpacing(4);
    const QIcon folder = QIcon::fromTheme(QStringLiteral("folder"));
    for (int i = 0; i < 4; ++i) {
        QListWidgetItem *item = new QListWidgetItem(folder, QStringLiteral("Item %1").arg(i));
        item->setTextAlignment(Qt::AlignHCenter);
        iconList.addItem(item);
    }
    iconList.resize(320, 200);
    iconList.setCurrentRow(0);
    iconList.item(0)->setSelected(true);
    iconList.setFocus();

    QImage img(340, 220, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);

    for (int row = 0; row < iconList.count(); ++row) {
        const QModelIndex idx = iconList.model()->index(row, 0);
        QStyleOptionViewItem opt;
        opt.initFrom(&iconList);
        opt.rect = iconList.visualRect(idx);
        opt.state = QStyle::State_Enabled;
        if (iconList.item(row)->isSelected())
            opt.state |= QStyle::State_Selected;
        if (row == 0)
            opt.state |= QStyle::State_MouseOver;
        opt.icon = iconList.item(row)->icon();
        opt.text = iconList.item(row)->text();
        opt.decorationSize = iconList.iconSize();
        opt.decorationPosition = QStyleOptionViewItem::Top;
        opt.index = idx;
        opt.widget = &iconList;
        style.drawControl(QStyle::CE_ItemViewItem, &opt, &p, &iconList);
    }
}

void TestUKUIConfigStyleOffscreen::testReadConfigAndHighlightPaths()
{
    UKUIGlobalDTConfig::GlobalDTConfig dt;
    dt.init();
    (void)dt.appPalette();
    (void)dt.normalline();
    (void)dt.focusline();
    (void)dt.kradiusMin();
    (void)dt.kradiusMenu();
    (void)dt.kradiusNormal();
    (void)dt.kradiusWindow();
    (void)dt.kradiusMax();

    const QString tokenUri = QStringLiteral("file:///usr/share/config/themeconfig/token/kfashion-light.css");
    if (QFile::exists(QUrl(tokenUri).toLocalFile()))
        (void)dt.loadDesignTokenFile(tokenUri);

    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QListWidget list;
    list.addItem(QStringLiteral("hl"));
    list.setProperty("highlightMode", true);
    list.resize(120, 40);
    QStyleOptionViewItem opt;
    opt.rect = list.visualRect(list.model()->index(0, 0));
    opt.state = QStyle::State_Enabled | QStyle::State_Selected;
    opt.icon = QIcon::fromTheme(QStringLiteral("folder"));
    opt.decorationSize = QSize(32, 32);
    opt.widget = &list;
    opt.index = list.model()->index(0, 0);
    QImage img(140, 48, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);
    style.drawControl(QStyle::CE_ItemViewItem, &opt, &p, &list);

    QPixmap pix(16, 16);
    pix.fill(QColor(40, 40, 40));
    QStyleOption optBase;
    optBase.rect = QRect(0, 0, 16, 16);
    (void)HighLightEffect::ordinaryGeneratePixmap(pix, &optBase, &list);
    (void)HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pix, &optBase, &list, HighLightEffect::BothDefaultAndHighlit);
}

void TestUKUIConfigStyleOffscreen::testPrimitiveItemViewPePaths()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QImage img(400, 300, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter painter(&img);

    QTreeWidget tree;
    tree.setHeaderHidden(true);
    new QTreeWidgetItem(&tree, {QStringLiteral("t-root")});
    tree.expandAll();
    tree.resize(200, 120);
    tree.setFocus();

    QListWidget list;
    list.addItems({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")});
    list.setSelectionMode(QAbstractItemView::ExtendedSelection);
    list.item(0)->setSelected(true);
    list.item(1)->setSelected(true);
    list.setFocus();
    list.resize(180, 90);

    const QStyle::State states[] = {
        QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_MouseOver,
        QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_HasFocus,
        QStyle::State_Enabled | QStyle::State_MouseOver,
    };

    for (int r = 0; r < tree.topLevelItemCount(); ++r) {
        const QModelIndex idx = tree.model()->index(r, 0);
        QStyleOptionViewItem opt;
        opt.rect = tree.visualRect(idx);
        opt.index = idx;
        opt.widget = &tree;
        opt.text = tree.model()->data(idx).toString();
        opt.icon = QIcon::fromTheme(QStringLiteral("folder"));
        opt.decorationSize = QSize(16, 16);
        for (QStyle::State st : states) {
            opt.state = st;
            style.drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, &painter, &tree);
            style.drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, &painter, &tree);
        }
    }
    for (int r = 0; r < list.count(); ++r) {
        const QModelIndex idx = list.model()->index(r, 0);
        QStyleOptionViewItem opt;
        opt.rect = list.visualRect(idx);
        opt.index = idx;
        opt.widget = &list;
        opt.text = list.item(r)->text();
        opt.icon = QIcon::fromTheme(QStringLiteral("folder"));
        opt.decorationSize = QSize(16, 16);
        for (QStyle::State st : states) {
            opt.state = st;
            style.drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, &painter, &list);
            style.drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, &painter, &list);
        }
    }
}

void TestUKUIConfigStyleOffscreen::testReadConfigAllTokenGetters()
{
    UKUIGlobalDTConfig::GlobalDTConfig dt;
    dt.init();
#define CALL_GETTER(M) (void)dt.M()
    CALL_GETTER(windowTextActive);
    CALL_GETTER(windowTextInactive);
    CALL_GETTER(windowTextDisable);
    CALL_GETTER(buttonActive);
    CALL_GETTER(buttonInactive);
    CALL_GETTER(buttonDisable);
    CALL_GETTER(lightActive);
    CALL_GETTER(lightInactive);
    CALL_GETTER(lightDisable);
    CALL_GETTER(midlightActive);
    CALL_GETTER(midlightInactive);
    CALL_GETTER(midlightDisable);
    CALL_GETTER(darkActive);
    CALL_GETTER(darkInactive);
    CALL_GETTER(darkDisable);
    CALL_GETTER(midActive);
    CALL_GETTER(midInactive);
    CALL_GETTER(midDisable);
    CALL_GETTER(textActive);
    CALL_GETTER(textInactive);
    CALL_GETTER(textDisable);
    CALL_GETTER(brightTextActive);
    CALL_GETTER(brightTextInactive);
    CALL_GETTER(brightTextDisable);
    CALL_GETTER(buttonTextActive);
    CALL_GETTER(buttonTextInactive);
    CALL_GETTER(buttonTextDisable);
    CALL_GETTER(baseActive);
    CALL_GETTER(baseInactive);
    CALL_GETTER(baseDisable);
    CALL_GETTER(windowActive);
    CALL_GETTER(windowInactive);
    CALL_GETTER(windowDisable);
    CALL_GETTER(shadowActive);
    CALL_GETTER(shadowInactive);
    CALL_GETTER(shadowDisable);
    CALL_GETTER(highLightActive);
    CALL_GETTER(highLightInactive);
    CALL_GETTER(highLightDisable);
    CALL_GETTER(highLightedTextActive);
    CALL_GETTER(highLightedTextInactive);
    CALL_GETTER(highLightedTextDisable);
    CALL_GETTER(linkActive);
    CALL_GETTER(linkInactive);
    CALL_GETTER(linkDisable);
    CALL_GETTER(linkVisitedActive);
    CALL_GETTER(linkVisitedInactive);
    CALL_GETTER(linkVisitedDisable);
    CALL_GETTER(alternateBaseActive);
    CALL_GETTER(alternateBaseInactive);
    CALL_GETTER(alternateBaseDisable);
    CALL_GETTER(noRoleActive);
    CALL_GETTER(noRoleInactive);
    CALL_GETTER(noRoleDisable);
    CALL_GETTER(toolTipBaseActive);
    CALL_GETTER(toolTipBaseInactive);
    CALL_GETTER(toolTipBaseDisable);
    CALL_GETTER(toolTipTextActive);
    CALL_GETTER(toolTipTextInactive);
    CALL_GETTER(toolTipTextDisable);
    CALL_GETTER(placeholderTextActive);
    CALL_GETTER(placeholderTextInactive);
    CALL_GETTER(placeholderTextDisable);
    CALL_GETTER(kBlack);
    CALL_GETTER(kBrand1);
    CALL_GETTER(kBrand2);
    CALL_GETTER(kBrand3);
    CALL_GETTER(kBrand4);
    CALL_GETTER(kBrand5);
    CALL_GETTER(kBrand6);
    CALL_GETTER(kBrand7);
    CALL_GETTER(kBrand8);
    CALL_GETTER(kBrand9);
    CALL_GETTER(kBrandClick);
    CALL_GETTER(kBrandDisable);
    CALL_GETTER(kBrandFocus);
    CALL_GETTER(kBrandHover);
    CALL_GETTER(kBrandNormal);
    CALL_GETTER(kComponentAlphaClick);
    CALL_GETTER(kComponentAlphaDisable);
    CALL_GETTER(kComponentAlphaHover);
    CALL_GETTER(kComponentAlphaNormal);
    CALL_GETTER(kComponentClick);
    CALL_GETTER(kComponentDisable);
    CALL_GETTER(kComponentHover);
    CALL_GETTER(kComponentNormal);
    CALL_GETTER(kComponentInput);
    CALL_GETTER(kComponentInputAlpha);
    CALL_GETTER(kComponentSelectedAlphaDisable);
    CALL_GETTER(kComponentSelectedAlphaNormal);
    CALL_GETTER(kComponentSelectedAlphaHover);
    CALL_GETTER(kComponentSelectedAlphaClick);
    CALL_GETTER(kComponentSelectedDisable);
    CALL_GETTER(kComponentSelectedNormal);
    CALL_GETTER(kComponentSelectedHover);
    CALL_GETTER(kComponentSelectedClick);
    CALL_GETTER(kContainAlphaClick);
    CALL_GETTER(kContainAlphaHover);
    CALL_GETTER(kContainClick);
    CALL_GETTER(kContainGeneralAlphaNormal);
    CALL_GETTER(kContainGeneralInactive);
    CALL_GETTER(kContainGeneralNormal);
    CALL_GETTER(kContainHover);
    CALL_GETTER(kContainSecondaryAlphaNormal);
    CALL_GETTER(kContainSecondaryInactive);
    CALL_GETTER(kContainSecondaryNormal);
    CALL_GETTER(kDivider);
    CALL_GETTER(kDividerWhite);
    CALL_GETTER(kErrorClick);
    CALL_GETTER(kErrorDisable);
    CALL_GETTER(kErrorHover);
    CALL_GETTER(kErrorNormal);
    CALL_GETTER(kFontPlaceholdertextDisable);
    CALL_GETTER(kFontPlaceholdertext);
    CALL_GETTER(kFontPrimaryDisable);
    CALL_GETTER(kFontPrimary);
    CALL_GETTER(kFontSecondaryDisable);
    CALL_GETTER(kFontSecondary);
    CALL_GETTER(kFontStrong);
    CALL_GETTER(kFontWhiteDisable);
    CALL_GETTER(kFontWhitePlaceholdertextDisable);
    CALL_GETTER(kFontWhiteplaceholdertext);
    CALL_GETTER(kFontWhite);
    CALL_GETTER(kFontWhiteSecondaryDisable);
    CALL_GETTER(kFontWhiteSecondary);
    CALL_GETTER(kGray0);
    CALL_GETTER(kGray1);
    CALL_GETTER(kGray2);
    CALL_GETTER(kGray3);
    CALL_GETTER(kGray4);
    CALL_GETTER(kGray5);
    CALL_GETTER(kGray6);
    CALL_GETTER(kGray7);
    CALL_GETTER(kGray8);
    CALL_GETTER(kGray9);
    CALL_GETTER(kGray10);
    CALL_GETTER(kGray11);
    CALL_GETTER(kGray12);
    CALL_GETTER(kGray13);
    CALL_GETTER(kGray14);
    CALL_GETTER(kGray15);
    CALL_GETTER(kGray16);
    CALL_GETTER(kGray17);
    CALL_GETTER(kGrayAlpha0);
    CALL_GETTER(kGrayAlpha1);
    CALL_GETTER(kGrayAlpha2);
    CALL_GETTER(kGrayAlpha3);
    CALL_GETTER(kGrayAlpha4);
    CALL_GETTER(kGrayAlpha5);
    CALL_GETTER(kGrayAlpha6);
    CALL_GETTER(kGrayAlpha7);
    CALL_GETTER(kGrayAlpha8);
    CALL_GETTER(kGrayAlpha9);
    CALL_GETTER(kGrayAlpha10);
    CALL_GETTER(kGrayAlpha11);
    CALL_GETTER(kGrayAlpha12);
    CALL_GETTER(kGrayAlpha13);
    CALL_GETTER(kGreen1);
    CALL_GETTER(kLineBrandClick);
    CALL_GETTER(kLineBrandDisable);
    CALL_GETTER(kLineBrandHover);
    CALL_GETTER(kLineBrandNormal);
    CALL_GETTER(kLineComponentClick);
    CALL_GETTER(kLineComponentDisable);
    CALL_GETTER(kLineComponentHover);
    CALL_GETTER(kLineComponentNormal);
    CALL_GETTER(kLineDisable);
    CALL_GETTER(kLineDock);
    CALL_GETTER(kLineErrorClick);
    CALL_GETTER(kLineErrorDisable);
    CALL_GETTER(kLineErrorHover);
    CALL_GETTER(kLineErrorNormal);
    CALL_GETTER(kLineInputClick);
    CALL_GETTER(kLineInputDisable);
    CALL_GETTER(kLineInputHover);
    CALL_GETTER(kLineInputNormal);
    CALL_GETTER(kLineMenu);
    CALL_GETTER(kLineNormal);
    CALL_GETTER(kLineSelectboxClick);
    CALL_GETTER(kLineSelectboxDisable);
    CALL_GETTER(kLineSelectboxHover);
    CALL_GETTER(kLineSelectboxNormal);
    CALL_GETTER(kLineSelectboxSelected);
    CALL_GETTER(kLineSuccessClick);
    CALL_GETTER(kLineSuccessDisable);
    CALL_GETTER(kLineSuccessHover);
    CALL_GETTER(kLineSuccessNormal);
    CALL_GETTER(kLineTable);
    CALL_GETTER(kLineWarningClick);
    CALL_GETTER(kLineWarningDisable);
    CALL_GETTER(kLineWarningHover);
    CALL_GETTER(kLineWarningNormal);
    CALL_GETTER(kLineWindowActive);
    CALL_GETTER(kLineWindowInactive);
    CALL_GETTER(kModalmask);
    CALL_GETTER(kMenu);
    CALL_GETTER(kOrange1);
    CALL_GETTER(kRed1);
    CALL_GETTER(kSelectAlphaWhiteClick);
    CALL_GETTER(kSelectAlphaWhiteDisable);
    CALL_GETTER(kSelectAlphaWhiteHover);
    CALL_GETTER(kSelectAlphaWhite);
    CALL_GETTER(kshadowComponent);
    CALL_GETTER(kshadowMenu);
    CALL_GETTER(kshadowMin);
    CALL_GETTER(kshadowPrimaryActive);
    CALL_GETTER(kshadowPrimaryInactive);
    CALL_GETTER(kshadowSecondaryActive);
    CALL_GETTER(kshadowSecondaryInactive);
    CALL_GETTER(kSuccessClick);
    CALL_GETTER(kSuccessDisable);
    CALL_GETTER(kSuccessHover);
    CALL_GETTER(kSuccessNormal);
    CALL_GETTER(kWarningClick);
    CALL_GETTER(kWarningDisable);
    CALL_GETTER(kWarningHover);
    CALL_GETTER(kWarningNormal);
    CALL_GETTER(kWhite);
    CALL_GETTER(shadow);
    CALL_GETTER(normalline);
    CALL_GETTER(focusline);
    CALL_GETTER(kradiusMin);
    CALL_GETTER(kradiusMenu);
    CALL_GETTER(kradiusNormal);
    CALL_GETTER(kradiusWindow);
    CALL_GETTER(kradiusMax);
#undef CALL_GETTER
    (void)dt.appPalette();
}

void TestUKUIConfigStyleOffscreen::testReadConfigDeepCoverage()
{
    using UKUIGlobalDTConfig::GlobalDTConfig;
    using UKUIGlobalDTConfig::GlobalDTConfigPrivate;

    GlobalDTConfigPrivate priv;
    QVERIFY(!priv.load(QStringLiteral("/nonexistent/ukui-readconfig-missing.css")));

    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString fixturePath = tmp.path() + QStringLiteral("/readconfig-fixture.css");
    {
        QFile fixture(fixturePath);
        QVERIFY(fixture.open(QIODevice::WriteOnly | QIODevice::Text));
        fixture.write(
            "body {\n"
            "--kblack: rgba(0, 0, 0, 1);\n"
            "--kbrand1: #aabbcc;\n"
            "--kbrand-normal: var(--kbrand1);\n"
            "--kbrand-hover: linear-gradient(0deg, rgba(10, 20, 30, 1), rgba(40, 50, 60, 1));\n"
            "--kbrand-click: linear-gradient(rgba(0, 0, 0, 0.2)), rgba(76, 175, 80, 1);\n"
            "--shadow: linear-gradient(180deg, rgba(255, 0, 0, 1), rgba(0, 255, 0, 1), rgba(0, 0, 255, 1));\n"
            "--kerror-normal: rgba(245, 63, 63, 0.5);\n"
            "--focusline: 4px;\n"
            "--normalline: 2px;\n"
            "--kradius-min: 2px;\n"
            "--kradius-normal: 6px;\n"
            "}\n");
        fixture.close();
    }
    QVERIFY(priv.load(fixturePath));
    priv.m_ukui_dt_loadConfig = true;
    priv.initUKUIGlobalThemeParameters();
    QVERIFY(priv.kBlack.style() != Qt::NoBrush || priv.kBrand1.style() != Qt::NoBrush);

    const QString hoverKey = priv.getKeyValue(QStringLiteral("kbrand-hover"));
    const QString clickKey = priv.getKeyValue(QStringLiteral("kbrand-click"));
    priv.m_lightMode = QStringLiteral("light");
    priv.adjustHighlightColor(priv.highLightActive, hoverKey, clickKey);
    priv.m_lightMode = QStringLiteral("dark");
    priv.adjustHighlightColor(priv.highLightActive, hoverKey, clickKey);
    priv.adjustHighlightColor(priv.highLightActive, QStringLiteral(": rgba(1, 2, 3, 1);"),
                            QStringLiteral(": rgba(4, 5, 6, 1);"));

    priv.updateColorsBasedOnAccentColor(QColor(55, 144, 250));
    priv.updateColorsBasedOnAccentColor(QColor(120, 115, 245));

    const QColor mixed = priv.processCSSColor(
        QStringLiteral("linear-gradient(rgba(0, 0, 0, 0.05)), rgba(55, 144, 250, 1)"),
        QColor(100, 150, 200));
    QVERIFY(mixed.isValid());

    const QColor composed = priv.composeColor(QColor(255, 0, 0, 128), QColor(0, 0, 255, 255));
    QVERIFY(composed.isValid());
    const QColor mixedList = priv.mixColor({QColor(10, 20, 30), QColor(40, 50, 60)});
    QVERIFY(mixedList.isValid());

    (void)priv.configMixColor(Qt::red, Qt::blue, 0.0);
    (void)priv.configMixColor(Qt::red, Qt::blue, 1.0);
    (void)priv.configMixColor(Qt::red, Qt::blue, 0.5);
    (void)priv.configMixColor(Qt::red, Qt::blue, qQNaN());

    GlobalDTConfig dt;
    QSignalSpy tokenSpy(&dt, &GlobalDTConfig::tokenChanged);
    QVERIFY(dt.loadDesignTokenFile(fixturePath));
    QCOMPARE(tokenSpy.count(), 1);
    (void)dt.appPalette();

    const QStringList systemTokens = {
        QStringLiteral("file:///usr/share/config/themeconfig/token/kdefault-light.css"),
        QStringLiteral("file:///usr/share/config/themeconfig/token/kdefault-dark.css"),
        QStringLiteral("file:///usr/share/config/themeconfig/token/kfashion-light.css"),
        QStringLiteral("file:///usr/share/config/themeconfig/token/korangeglow-light.css"),
    };
    for (const QString &uri : systemTokens) {
        if (!QFile::exists(QUrl(uri).toLocalFile()))
            continue;
        GlobalDTConfig themed;
        QVERIFY(themed.loadDesignTokenFile(uri));
        (void)themed.appPalette();
    }

    QVariant unset;
    qApp->setProperty("useDarkTheme", unset);
    qApp->setProperty("useLightTheme", unset);
    GlobalDTConfig darkDt;
    qApp->setProperty("useDarkTheme", true);
    darkDt.init();
    (void)darkDt.appPalette();
    qApp->setProperty("useLightTheme", true);
    GlobalDTConfig lightDt;
    lightDt.init();
    (void)lightDt.appPalette();
    qApp->setProperty("useDarkTheme", unset);
    qApp->setProperty("useLightTheme", unset);
}

void TestUKUIConfigStyleOffscreen::testReadConfigParseEdgeCases()
{
    using UKUIGlobalDTConfig::GlobalDTConfigPrivate;

    GlobalDTConfigPrivate empty;
    empty.m_dt_configData.clear();
    empty.m_ukui_dt_loadConfig = false;
    QBrush brush;
    QVERIFY(!empty.getColorValue(brush, QStringLiteral("kblack")));
    int px = 0;
    QVERIFY(!empty.getValue(px, QStringLiteral("focusline")));
    QVERIFY(empty.getKeyValue(QStringLiteral("kblack")).isEmpty());

    GlobalDTConfigPrivate parseOnly;
    parseOnly.m_dt_configData = QByteArray(
        "body {\n"
        "--parse-hex: rgba(255, 0, 170, 1);\n"
        "--parse-alpha-f: rgba(1, 2, 3, 0.5);\n"
        "--parse-alpha-mul-a: rgba(10, 20, 30, 255.0*0.5);\n"
        "--parse-alpha-mul-b: rgba(10, 20, 30, 0.5*255.0);\n"
        "--parse-alpha-255: rgba(10, 20, 30, 255);\n"
        "--parse-bad-var: var(--no-such-token);\n"
        "--parse-bad-rgba: rgba(;\n"
        "}\n");
    QVERIFY(parseOnly.getColorValue(brush, QStringLiteral("parse-hex")));
    QVERIFY(parseOnly.getColorValue(brush, QStringLiteral("parse-alpha-f")));
    QVERIFY(parseOnly.getColorValue(brush, QStringLiteral("parse-alpha-mul-a")));
    QVERIFY(parseOnly.getColorValue(brush, QStringLiteral("parse-alpha-mul-b")));
    QVERIFY(parseOnly.getColorValue(brush, QStringLiteral("parse-alpha-255")));
    (void)parseOnly.getColorValue(brush, QStringLiteral("parse-bad-var"));
    (void)parseOnly.getColorValue(brush, QStringLiteral("parse-bad-rgba"));
    QVERIFY(!parseOnly.getColorValue(brush, QStringLiteral("missing-key-xyz")));

    QVERIFY(parseOnly.stringToColor(QStringLiteral("255, 0, 170, 0.5")).isValid());
    QVERIFY(parseOnly.stringToColor(QStringLiteral("255, 0, 170, 255.0*0.5")).isValid());
    QVERIFY(parseOnly.stringToColor(QStringLiteral("255, 0, 170, 0.5*255.0")).isValid());
    QVERIFY(parseOnly.stringToColor(QStringLiteral("255, 0, 170, 255")).isValid());
    (void)parseOnly.stringToColor(QStringLiteral("#aabbcc"));

    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString dirPath = tmp.path() + QStringLiteral("/not-a-file");
    QVERIFY(QDir().mkpath(dirPath));
    GlobalDTConfigPrivate loadFail;
    QVERIFY(!loadFail.load(dirPath));

    const QByteArray savedXdg = qgetenv("XDG_DATA_DIRS");
    qunsetenv("XDG_DATA_DIRS");
    GlobalDTConfigPrivate noXdg;
    noXdg.init();
    if (!savedXdg.isEmpty())
        qputenv("XDG_DATA_DIRS", savedXdg);

    const QString savedApp = QCoreApplication::applicationName();
    QCoreApplication::setApplicationName(QStringLiteral("ukui-screensaver-dialog"));
    UKUIGlobalDTConfig::GlobalDTConfig lightAppDt;
    lightAppDt.init();
    (void)lightAppDt.appPalette();
    QCoreApplication::setApplicationName(savedApp);

    GlobalDTConfigPrivate accent;
    const QString kdefaultLight = QStringLiteral("file:///usr/share/config/themeconfig/token/kdefault-light.css");
    if (!QFile::exists(QUrl(kdefaultLight).toLocalFile()))
        return;
    accent.m_ukui_dt_loadConfig = true;
    QVERIFY(accent.load(kdefaultLight));
    accent.initUKUIGlobalThemeParameters();
    accent.updateColorsBasedOnAccentColor(QColor(235, 48, 150));
    accent.updateColorsBasedOnAccentColor(QColor(243, 34, 45));
    accent.updateColorsBasedOnAccentColor(QColor(246, 140, 39));
    accent.updateColorsBasedOnAccentColor(QColor(249, 197, 61));
    accent.updateColorsBasedOnAccentColor(QColor(82, 196, 41));
    accent.updateColorsBasedOnAccentColor(QColor());
}

void TestUKUIConfigStyleOffscreen::testItemViewDelegateStyleOptionPe()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QImage canvas(520, 720, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    ExposedStyledItemDelegate delegate;
    const QIcon folderIcon = QIcon::fromTheme(QStringLiteral("folder"));

    const QStyle::State extraStates[] = {
        QStyle::State_None,
        QStyle::State_MouseOver,
        QStyle::State_Selected,
        QStyle::State_Selected | QStyle::State_MouseOver,
        QStyle::State_Selected | QStyle::State_HasFocus,
        QStyle::State_HasFocus,
    };

    auto paintViewIndex = [&](QAbstractItemView *view, const QModelIndex &idx, QStyle::State extra) {
        if (!idx.isValid())
            return;
        QStyleOptionViewItem opt;
        opt.initFrom(view);
        delegate.initStyleOption(&opt, idx);
        opt.state |= extra;
        opt.icon = folderIcon;
        if (opt.decorationSize.isEmpty())
            opt.decorationSize = QSize(16, 16);
        style.drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, &painter, view);
        style.drawControl(QStyle::CE_ItemViewItem, &opt, &painter, view);

        QStyleOptionViewItem rowOpt = opt;
        if (idx.row() % 2 == 1)
            rowOpt.features |= QStyleOptionViewItem::Alternate;
        style.drawPrimitive(QStyle::PE_PanelItemViewRow, &rowOpt, &painter, view);

        if (qobject_cast<QTreeView *>(view)) {
            const QRect vr = view->visualRect(idx);
            QStyleOptionViewItem branchOpt = opt;
            branchOpt.rect = QRect(vr.left() > 8 ? 0 : vr.left(), vr.top(), qMax(18, vr.left() + 2), vr.height());
            const QStyle::State branchStates[] = {
                QStyle::State_Enabled,
                QStyle::State(QStyle::State_Enabled | QStyle::State_Selected),
                QStyle::State(QStyle::State_Enabled | QStyle::State_MouseOver),
                QStyle::State(QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_MouseOver),
            };
            for (QStyle::State br : branchStates) {
                branchOpt.state = br;
                style.drawPrimitive(QStyle::PE_IndicatorBranch, &branchOpt, &painter, view);
            }
        }
    };

    QTreeWidget tree;
    tree.setHeaderHidden(true);
    tree.setAlternatingRowColors(true);
    tree.setRootIsDecorated(true);
    tree.setProperty("highlightMode", true);
    QTreeWidgetItem *root = new QTreeWidgetItem(&tree, {QStringLiteral("root")});
    QTreeWidgetItem *branchA = new QTreeWidgetItem(root, {QStringLiteral("branch-a")});
    new QTreeWidgetItem(branchA, {QStringLiteral("leaf-a1")});
    new QTreeWidgetItem(branchA, {QStringLiteral("leaf-a2")});
    QTreeWidgetItem *branchB = new QTreeWidgetItem(root, {QStringLiteral("branch-b")});
    new QTreeWidgetItem(branchB, {QStringLiteral("leaf-b1")});
    tree.expandAll();
    tree.resize(360, 280);
    tree.setFocus();
    tree.selectionModel()->select(tree.model()->index(1, 0), QItemSelectionModel::ClearAndSelect);
    tree.selectionModel()->select(tree.model()->index(2, 0), QItemSelectionModel::Select);

    std::function<void(const QModelIndex &)> walkTree = [&](const QModelIndex &parent) {
        const int rows = tree.model()->rowCount(parent);
        for (int r = 0; r < rows; ++r) {
            const QModelIndex idx = tree.model()->index(r, 0, parent);
            for (QStyle::State extra : extraStates)
                paintViewIndex(&tree, idx, extra);
            if (tree.model()->hasChildren(idx))
                walkTree(idx);
        }
    };
    walkTree(QModelIndex());

    QListWidget list;
    list.setAlternatingRowColors(true);
    list.setSelectionMode(QAbstractItemView::ExtendedSelection);
    list.setProperty("highlightMode", true);
    for (int i = 0; i < 6; ++i) {
        auto *item = new QListWidgetItem(folderIcon, QStringLiteral("L%1").arg(i));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(i % 2 ? Qt::Checked : Qt::PartiallyChecked);
        list.addItem(item);
    }
    list.resize(320, 220);
    list.setFocus(Qt::OtherFocusReason);
    for (int row = 0; row < 3; ++row)
        list.item(row)->setSelected(true);
    for (int row = 0; row < list.count(); ++row) {
        const QModelIndex idx = list.model()->index(row, 0);
        for (QStyle::State extra : extraStates)
            paintViewIndex(&list, idx, extra);
    }

    QListWidget iconList;
    iconList.setViewMode(QListView::IconMode);
    iconList.setMovement(QListView::Static);
    iconList.setGridSize(QSize(96, 104));
    iconList.setIconSize(QSize(48, 48));
    iconList.setSpacing(6);
    iconList.setProperty("highlightMode", true);
    iconList.setWrapping(true);
    for (int i = 0; i < 6; ++i) {
        auto *item = new QListWidgetItem(folderIcon, QStringLiteral("icon-%1").arg(i));
        item->setTextAlignment(Qt::AlignHCenter);
        iconList.addItem(item);
    }
    iconList.resize(400, 240);
    iconList.setCurrentRow(0);
    iconList.item(0)->setSelected(true);
    iconList.item(1)->setSelected(true);
    for (int row = 0; row < iconList.count(); ++row) {
        const QModelIndex idx = iconList.model()->index(row, 0);
        paintViewIndex(&iconList, idx, QStyle::State_Selected | QStyle::State_MouseOver);
        paintViewIndex(&iconList, idx, QStyle::State_Selected);
        paintViewIndex(&iconList, idx, QStyle::State_MouseOver);

        QStyleOptionViewItem bgOpt;
        bgOpt.initFrom(&iconList);
        delegate.initStyleOption(&bgOpt, idx);
        bgOpt.state = QStyle::State_Enabled | QStyle::State_MouseOver;
        bgOpt.backgroundBrush = QBrush(QColor(200, 210, 220));
        style.drawPrimitive(QStyle::PE_PanelItemViewItem, &bgOpt, &painter, &iconList);
        style.drawControl(QStyle::CE_ItemViewItem, &bgOpt, &painter, &iconList);
    }

    QTableWidget table(5, 2);
    table.setAlternatingRowColors(true);
    table.setHorizontalHeaderLabels({QStringLiteral("C0"), QStringLiteral("C1")});
    for (int r = 0; r < table.rowCount(); ++r) {
        for (int c = 0; c < table.columnCount(); ++c)
            table.setItem(r, c, new QTableWidgetItem(QStringLiteral("%1-%2").arg(r).arg(c)));
    }
    table.resize(300, 180);
    table.selectRow(2);
    table.setFocus();
    for (int r = 0; r < table.rowCount(); ++r) {
        for (int c = 0; c < table.columnCount(); ++c) {
            const QModelIndex idx = table.model()->index(r, c);
            paintViewIndex(&table, idx, QStyle::State_Selected | QStyle::State_HasFocus);
            paintViewIndex(&table, idx, QStyle::State_MouseOver);
        }
    }

    QListWidget singleRow;
    singleRow.addItem(QStringLiteral("only-one"));
    singleRow.item(0)->setSelected(true);
    singleRow.resize(200, 44);
    singleRow.setFocus();
    paintViewIndex(&singleRow, singleRow.model()->index(0, 0),
                   QStyle::State_Selected | QStyle::State_HasFocus);
}

void TestUKUIConfigStyleOffscreen::testDrawPrimitiveSettingsFullPort()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QImage surface(128, 128, QImage::Format_ARGB32);
    surface.fill(Qt::transparent);
    QPainter painter(&surface);
    QWidget widget;
    widget.resize(100, 100);

    auto drawPe = [&](QStyle::PrimitiveElement pe, QStyleOption &opt) {
        style.drawPrimitive(pe, &opt, &painter, &widget);
    };

    {
        QStyleOption menuOpt;
        menuOpt.rect = QRect(0, 0, 100, 100);
        menuOpt.state = QStyle::State_Enabled;
        menuOpt.palette = widget.palette();
        widget.setProperty("setFrameradius", 2);
        widget.setProperty("setItemradius", 2);
        widget.setProperty("setMenuBackgroundBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setMenuBackgroundPen", QPen(QColor(Qt::red)));
        widget.setProperty("setItemSelectBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setMenuHoverPen", QPen(QColor(Qt::red)));
        widget.setProperty("setItemFocusPen", QPen(QColor(Qt::red)));
        drawPe(QStyle::PE_PanelMenu, menuOpt);
        drawPe(QStyle::PE_FrameMenu, menuOpt);
    }

    {
        QStyleOption focusOpt;
        focusOpt.rect = QRect(0, 0, 100, 100);
        focusOpt.state = QStyle::State_HasFocus;
        focusOpt.palette = widget.palette();
        widget.setProperty("setRadius", 1);
        widget.setProperty("setDefaultBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setHoverBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setSelectBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setBranchDefaultBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setBranchHoverBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setBranchSelectBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setHoverPen", QPen(QColor(Qt::red)));
        widget.setProperty("setSelectPen", QPen(QColor(Qt::red)));
        widget.setProperty("setFocusPen", QPen(QColor(Qt::red)));
        drawPe(QStyle::PE_FrameFocusRect, focusOpt);
    }

    {
        QStyleOptionButton cmd;
        cmd.rect = QRect(0, 0, 100, 36);
        cmd.palette = widget.palette();
        const QStyle::State cmdStates[] = {
            QStyle::State_Enabled,
            QStyle::State(QStyle::State_Enabled | QStyle::State_MouseOver),
            QStyle::State(QStyle::State_Enabled | QStyle::State_Sunken),
            QStyle::State(QStyle::State_Enabled | QStyle::State_On),
            QStyle::State_None,
        };
        widget.setProperty("setRadius", 2);
        widget.setProperty("setDefaultBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setHoverBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setClickBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setDisableBrush", QBrush(QColor(Qt::red)));
        for (QStyle::State st : cmdStates) {
            cmd.state = st;
            drawPe(QStyle::PE_PanelButtonCommand, cmd);
            drawPe(QStyle::PE_PanelButtonBevel, cmd);
        }
    }

    {
        QStyleOptionButton toolOpt;
        toolOpt.rect = QRect(0, 0, 100, 36);
        toolOpt.palette = widget.palette();
        widget.setProperty("setCheckBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setCheckHoverBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setCheckClickBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setCheckDisableBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setCheckPen", QPen(QColor(Qt::red)));
        toolOpt.state = QStyle::State(QStyle::State_Enabled | QStyle::State_On);
        drawPe(QStyle::PE_PanelButtonTool, toolOpt);
        toolOpt.state = QStyle::State(QStyle::State_Enabled | QStyle::State_MouseOver);
        drawPe(QStyle::PE_PanelButtonTool, toolOpt);
    }

    {
        QStyleOptionFrame lineEdit;
        lineEdit.rect = QRect(0, 0, 100, 28);
        lineEdit.palette = widget.palette();
        widget.setProperty("setFocusBrush", QBrush(QColor(Qt::red)));
        lineEdit.state = QStyle::State(QStyle::State_Enabled | QStyle::State_HasFocus);
        drawPe(QStyle::PE_PanelLineEdit, lineEdit);
        lineEdit.state = QStyle::State(QStyle::State_Enabled | QStyle::State_MouseOver);
        drawPe(QStyle::PE_PanelLineEdit, lineEdit);
    }

    {
        QStyleOptionTabWidgetFrame tabFrame;
        tabFrame.rect = QRect(0, 0, 100, 100);
        tabFrame.state = QStyle::State_Enabled;
        tabFrame.palette = widget.palette();
        widget.setProperty("setWidgetBackgroundBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setTabBarBackgroundBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setTabBarSelectBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setTabBarHoverPen", QPen(QColor(Qt::red)));
        drawPe(QStyle::PE_FrameTabWidget, tabFrame);
        QStyleOptionTabBarBase tabBarBase;
        tabBarBase.tabBarRect = QRect(0, 0, 100, 28);
        tabBarBase.state = QStyle::State_Enabled;
        tabBarBase.palette = widget.palette();
        drawPe(QStyle::PE_FrameTabBarBase, tabBarBase);
    }

    {
        QStyleOptionButton checkOpt;
        checkOpt.rect = QRect(0, 0, 24, 24);
        checkOpt.palette = widget.palette();
        widget.setProperty("setPathBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setPathDisableBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setContentPen", QPen(QColor(Qt::red)));
        widget.setProperty("setOnHoverBrushIslinearGradient", true);
        const QStyle::State checkStates[] = {
            QStyle::State_Enabled | QStyle::State_Off,
            QStyle::State_Enabled | QStyle::State_On,
            QStyle::State_Enabled | QStyle::State_NoChange,
            QStyle::State(QStyle::State_Enabled | QStyle::State_On | QStyle::State_MouseOver),
            QStyle::State(QStyle::State_Enabled | QStyle::State_On | QStyle::State_Sunken),
            QStyle::State_None,
        };
        for (QStyle::State st : checkStates) {
            checkOpt.state = st;
            drawPe(QStyle::PE_IndicatorCheckBox, checkOpt);
        }
    }

    {
        QStyleOptionButton radioOpt;
        radioOpt.rect = QRect(0, 0, 24, 24);
        radioOpt.palette = widget.palette();
        widget.setProperty("setChildrenOnDefaultBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setChildrenOnHoverBrush", QBrush(QColor(Qt::red)));
        widget.setProperty("setOnDefaultPen", QPen(QColor(Qt::red)));
        const QStyle::State radioStates[] = {
            QStyle::State_Enabled | QStyle::State_Off,
            QStyle::State_Enabled | QStyle::State_On,
            QStyle::State(QStyle::State_Enabled | QStyle::State_MouseOver),
            QStyle::State_None,
        };
        for (QStyle::State st : radioStates) {
            radioOpt.state = st;
            drawPe(QStyle::PE_IndicatorRadioButton, radioOpt);
        }
    }

    {
        QStyleOption opt;
        opt.rect = QRect(0, 0, 20, 20);
        opt.palette = widget.palette();
        opt.state = QStyle::State_Enabled;
        drawPe(QStyle::PE_IndicatorArrowUp, opt);
        drawPe(QStyle::PE_IndicatorArrowDown, opt);
        drawPe(QStyle::PE_IndicatorArrowLeft, opt);
        drawPe(QStyle::PE_IndicatorArrowRight, opt);
        drawPe(QStyle::PE_IndicatorButtonDropDown, opt);
        drawPe(QStyle::PE_IndicatorTabClose, opt);
        drawPe(QStyle::PE_Frame, opt);
        drawPe(QStyle::PE_FrameGroupBox, opt);
        drawPe(QStyle::PE_FrameStatusBar, opt);
        drawPe(QStyle::PE_PanelScrollAreaCorner, opt);
        widget.setProperty("setBackgroundBrush", QBrush(QColor(Qt::red)));
        drawPe(QStyle::PE_PanelTipLabel, opt);
        QStyleOptionHeader hdr;
        hdr.rect = opt.rect;
        hdr.state = QStyle::State_Enabled;
        hdr.sortIndicator = QStyleOptionHeader::SortDown;
        drawPe(QStyle::PE_IndicatorHeaderArrow, hdr);
    }
}

void TestUKUIConfigStyleOffscreen::testPolishEventFilterAndRealWidgetDraw()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QImage canvas(480, 400, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);

    auto polishDrawPe = [&](QWidget &w, QStyle::PrimitiveElement pe) {
        style.polish(&w);
        QStyleOption opt;
        opt.initFrom(&w);
        opt.rect = w.rect().adjusted(0, 0, -1, -1);
        style.drawPrimitive(pe, &opt, &painter, &w);
        style.unpolish(&w);
    };

    auto polishDrawCe = [&](QWidget &w, QStyle::ControlElement ce) {
        style.polish(&w);
        QStyleOption opt;
        opt.initFrom(&w);
        opt.rect = w.rect();
        style.drawControl(ce, &opt, &painter, &w);
    };

    QPushButton pushBtn(QStringLiteral("Push"));
    pushBtn.setCheckable(true);
    pushBtn.setChecked(true);
    pushBtn.resize(120, 32);
    polishDrawPe(pushBtn, QStyle::PE_PanelButtonCommand);
    polishDrawCe(pushBtn, QStyle::CE_PushButton);

    QToolButton toolBtn;
    toolBtn.setText(QStringLiteral("Tool"));
    toolBtn.setCheckable(true);
    toolBtn.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBtn.resize(100, 32);
    polishDrawPe(toolBtn, QStyle::PE_PanelButtonTool);
    polishDrawCe(toolBtn, QStyle::CE_ToolButtonLabel);

    QCheckBox checkBox(QStringLiteral("Check"));
    checkBox.setTristate(true);
    checkBox.setCheckState(Qt::PartiallyChecked);
    checkBox.resize(120, 24);
    polishDrawPe(checkBox, QStyle::PE_IndicatorCheckBox);
    polishDrawCe(checkBox, QStyle::CE_CheckBox);

    QRadioButton radio(QStringLiteral("Radio"));
    radio.setChecked(true);
    radio.resize(120, 24);
    polishDrawPe(radio, QStyle::PE_IndicatorRadioButton);
    polishDrawCe(radio, QStyle::CE_RadioButton);

    QLineEdit lineEdit(QStringLiteral("edit"));
    lineEdit.resize(160, 28);
    polishDrawPe(lineEdit, QStyle::PE_PanelLineEdit);

    QComboBox combo;
    combo.setEditable(true);
    combo.addItems({QStringLiteral("a"), QStringLiteral("b")});
    combo.resize(140, 28);
    style.polish(&combo);
    QStyleOptionComboBox comboOpt;
    comboOpt.initFrom(&combo);
    comboOpt.rect = combo.rect();
    style.drawComplexControl(QStyle::CC_ComboBox, &comboOpt, &painter, &combo);
    style.drawControl(QStyle::CE_ComboBoxLabel, &comboOpt, &painter, &combo);

    QSpinBox spin;
    spin.setRange(0, 99);
    spin.setValue(7);
    spin.resize(100, 28);
    style.polish(&spin);
    QStyleOptionSpinBox spinOpt;
    spinOpt.initFrom(&spin);
    spinOpt.rect = spin.rect();
    style.drawComplexControl(QStyle::CC_SpinBox, &spinOpt, &painter, &spin);

    QScrollBar scroll(Qt::Vertical);
    scroll.setRange(0, 200);
    scroll.setValue(50);
    scroll.resize(20, 200);
    scroll.setProperty("drawScrollBarGroove", true);
    style.polish(&scroll);
    QStyleOptionSlider scrollOpt;
    scrollOpt.initFrom(&scroll);
    scrollOpt.rect = scroll.rect();
    scrollOpt.orientation = Qt::Vertical;
    style.drawComplexControl(QStyle::CC_ScrollBar, &scrollOpt, &painter, &scroll);

    QSlider slider(Qt::Horizontal);
    slider.setRange(0, 100);
    slider.setValue(40);
    slider.resize(200, 24);
    style.polish(&slider);
    QStyleOptionSlider sliderOpt;
    sliderOpt.initFrom(&slider);
    sliderOpt.rect = slider.rect();
    style.drawComplexControl(QStyle::CC_Slider, &sliderOpt, &painter, &slider);

    QProgressBar progress;
    progress.setRange(0, 100);
    progress.setValue(65);
    progress.setTextVisible(true);
    progress.resize(200, 24);
    style.polish(&progress);
    QStyleOptionProgressBar pbOpt;
    pbOpt.initFrom(&progress);
    pbOpt.rect = progress.rect();
    style.drawControl(QStyle::CE_ProgressBar, &pbOpt, &painter, &progress);
    style.drawControl(QStyle::CE_ProgressBarContents, &pbOpt, &painter, &progress);
    style.drawControl(QStyle::CE_ProgressBarGroove, &pbOpt, &painter, &progress);

    QGroupBox group(QStringLiteral("Group"));
    group.setCheckable(true);
    group.setChecked(true);
    group.resize(200, 100);
    style.polish(&group);
    QStyleOptionGroupBox gbOpt;
    gbOpt.initFrom(&group);
    gbOpt.rect = group.rect();
    style.drawComplexControl(QStyle::CC_GroupBox, &gbOpt, &painter, &group);

    QTabWidget tabs;
    tabs.addTab(new QWidget, QStringLiteral("T1"));
    tabs.addTab(new QWidget, QStringLiteral("T2"));
    tabs.resize(240, 160);
    style.polish(&tabs);
    QStyleOptionTabWidgetFrame twf;
    twf.initFrom(&tabs);
    twf.rect = tabs.rect();
    style.drawPrimitive(QStyle::PE_FrameTabWidget, &twf, &painter, &tabs);

    QMenu menu;
    menu.addAction(QStringLiteral("Act1"));
    menu.addAction(QStringLiteral("Act2"));
    QWidget menuHost;
    menuHost.resize(180, 80);
    menuHost.setProperty("setFrameradius", 2);
    menuHost.setProperty("setMenuBackgroundBrush", QBrush(QColor(Qt::red)));
    style.polish(&menuHost);
    QStyleOption menuPanel;
    menuPanel.rect = menuHost.rect();
    menuPanel.state = QStyle::State_Enabled;
    style.drawPrimitive(QStyle::PE_PanelMenu, &menuPanel, &painter, &menuHost);

    QFocusEvent focusIn(QEvent::FocusIn);
    style.eventFilter(&pushBtn, &focusIn);
    QWheelEvent wheel(QPointF(10, 10), 120, Qt::NoButton, Qt::NoModifier);
    style.eventFilter(&slider, &wheel);
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    style.eventFilter(&lineEdit, &keyPress);
}

void TestUKUIConfigStyleOffscreen::testComplexControlSubControlsExhaustive()
{
    UKUIConfigStyle style(QStringLiteral("ukui-offscreen"));
    QImage img(320, 320, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter painter(&img);

    QWidget scrollHost;
    scrollHost.setProperty("setRadius", 2);
    scrollHost.setProperty("setGrooveDefaultBrush", QBrush(QColor(80, 80, 80)));
    scrollHost.setProperty("setSliderDefaultBrush", QBrush(QColor(120, 120, 120)));
    scrollHost.setProperty("setSliderHoverBrush", QBrush(QColor(140, 140, 140)));
    scrollHost.setProperty("drawScrollBarGroove", true);

    QStyleOptionSlider hScroll;
    hScroll.orientation = Qt::Horizontal;
    hScroll.rect = QRect(0, 0, 280, 20);
    hScroll.minimum = 0;
    hScroll.maximum = 500;
    hScroll.pageStep = 50;
    hScroll.sliderPosition = 120;
    hScroll.state = QStyle::State_Enabled | QStyle::State_Active;

    const QStyle::SubControl scrollSubs[] = {
        QStyle::SC_ScrollBarAddLine,
        QStyle::SC_ScrollBarSubLine,
        QStyle::SC_ScrollBarSlider,
        QStyle::SC_ScrollBarGroove,
        QStyle::SC_ScrollBarAddPage,
        QStyle::SC_ScrollBarSubPage,
    };
    for (QStyle::SubControl sc : scrollSubs) {
        hScroll.activeSubControls = sc;
        style.drawComplexControl(QStyle::CC_ScrollBar, &hScroll, &painter, &scrollHost);
        (void)style.subControlRect(QStyle::CC_ScrollBar, &hScroll, sc, &scrollHost);
        (void)style.hitTestComplexControl(QStyle::CC_ScrollBar, &hScroll, hScroll.rect.center(), &scrollHost);
    }

    QWidget sliderHost;
    sliderHost.setProperty("setHandleDefaultBrush", QBrush(QColor(Qt::red)));
    sliderHost.setProperty("setGrooveValueDefaultBrush", QBrush(QColor(Qt::blue)));
    QStyleOptionSlider slider;
    slider.orientation = Qt::Horizontal;
    slider.rect = QRect(0, 0, 280, 28);
    slider.minimum = 0;
    slider.maximum = 100;
    slider.sliderPosition = 60;
    slider.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    const QStyle::SubControl sliderSubs[] = {
        QStyle::SC_SliderGroove,
        QStyle::SC_SliderHandle,
        QStyle::SC_SliderTickmarks,
    };
    for (QStyle::SubControl sc : sliderSubs) {
        slider.activeSubControls = sc;
        style.drawComplexControl(QStyle::CC_Slider, &slider, &painter, &sliderHost);
        (void)style.subControlRect(QStyle::CC_Slider, &slider, sc, &sliderHost);
    }

    QWidget comboHost;
    comboHost.setProperty("setComboRadius", 4);
    QStyleOptionComboBox combo;
    combo.rect = QRect(0, 0, 160, 28);
    combo.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    combo.currentText = QStringLiteral("item");
    combo.activeSubControls = QStyle::SC_ComboBoxArrow;
    style.drawComplexControl(QStyle::CC_ComboBox, &combo, &painter, &comboHost);
    combo.activeSubControls = QStyle::SC_ComboBoxEditField;
    style.drawComplexControl(QStyle::CC_ComboBox, &combo, &painter, &comboHost);
    combo.activeSubControls = QStyle::SC_ComboBoxListBoxPopup;
    style.drawComplexControl(QStyle::CC_ComboBox, &combo, &painter, &comboHost);

    QStyleOptionSpinBox spin;
    spin.rect = QRect(0, 0, 120, 28);
    spin.state = QStyle::State_Enabled;
    spin.activeSubControls = QStyle::SC_SpinBoxUp;
    style.drawComplexControl(QStyle::CC_SpinBox, &spin, &painter, &comboHost);
    spin.activeSubControls = QStyle::SC_SpinBoxDown;
    style.drawComplexControl(QStyle::CC_SpinBox, &spin, &painter, &comboHost);
    spin.activeSubControls = QStyle::SC_SpinBoxFrame;
    style.drawComplexControl(QStyle::CC_SpinBox, &spin, &painter, &comboHost);

    QStyleOptionToolButton tb;
    tb.rect = QRect(0, 0, 90, 32);
    tb.state = QStyle::State_Enabled | QStyle::State_On;
    tb.features = QStyleOptionToolButton::MenuButtonPopup;
    tb.activeSubControls = QStyle::SC_ToolButton | QStyle::SC_ToolButtonMenu;
    style.drawComplexControl(QStyle::CC_ToolButton, &tb, &painter, &comboHost);
}

void TestUKUIConfigStyleOffscreen::testHighlightEffectExhaustiveOffscreen()
{
    QWidget widget;
    widget.resize(64, 64);
    HighLightEffect::setSkipEffect(&widget, true);
    HighLightEffect::setSkipEffect(&widget, false);

    QPixmap pure(8, 8);
    pure.fill(QColor(200, 200, 200));
    QImage mixedImg(8, 8, QImage::Format_ARGB32);
    mixedImg.fill(Qt::red);
    mixedImg.setPixel(0, 0, qRgb(10, 20, 30));
    const QPixmap mixed = QPixmap::fromImage(mixedImg);
    (void)HighLightEffect::isPixmapPureColor(pure);
    (void)HighLightEffect::isPixmapPureColor(mixed);
    (void)HighLightEffect::isSymbolicColor(pure);

    QMenu menu;
    HighLightEffect::setMenuIconHighlightEffect(&menu, HighLightEffect::HighlightEffect,
                                                HighLightEffect::BothDefaultAndHighlit);
    (void)HighLightEffect::isWidgetIconUseHighlightEffect(&menu);

    QListWidget view;
    view.addItem(QStringLiteral("x"));
    HighLightEffect::setViewItemIconHighlightEffect(&view, HighLightEffect::hoverHighLight,
                                                    HighLightEffect::HighlightOnly);

    HighLightEffect::setSymoblicColor(QColor(80, 120, 200));
    (void)HighLightEffect::getCurrentSymbolicColor();
    HighLightEffect::setWidgetIconFillSymbolicColor(&widget, true);
    HighLightEffect::setWidgetIconFillSymbolicColor(&widget, false);

    QStyleOption opt;
    opt.rect = QRect(0, 0, 16, 16);
    opt.palette = qApp->palette();
    (void)HighLightEffect::defaultStyleDark(&opt);

    QPixmap pix(16, 16);
    pix.fill(QColor(40, 40, 40));
    widget.setProperty("useIconHighlightEffect", HighLightEffect::ordinaryHighLight);
    (void)HighLightEffect::generatePixmap(pix, &opt, &widget, false);
    widget.setProperty("useIconHighlightEffect", HighLightEffect::hoverHighLight);
    (void)HighLightEffect::hoverGeneratePixmap(pix, &opt, &widget);
    widget.setProperty("useIconHighlightEffect", HighLightEffect::filledSymbolicColorHighLight);
    (void)HighLightEffect::filledSymbolicColoredGeneratePixmap(pix, &opt, &widget);
    (void)HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pix, &opt, &widget,
                                                              HighLightEffect::BothDefaultAndHighlit);
}

QTEST_MAIN(TestUKUIConfigStyleOffscreen)
#include "test_ukui_config_style_offscreen.moc"
