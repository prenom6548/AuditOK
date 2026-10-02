/*
 * pathbar.cpp / sidebar.cpp / menutoolbutoon.cpp / filedialogplugin.cpp — offscreen.
 */

#include <QTest>
#include <QCoreApplication>
#include <QToolButton>
#include <QMenu>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QImage>
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionViewItem>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QStyleOptionToolButton>

#include "pathbar.h"
#include "sidebar.h"
#include "menutoolbutoon.h"
#include "filedialogplugin.h"

using namespace UKUIFileDialog;

class TestFileDialogComponentsOffscreen : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testPathBar();
    void testSideBarAndStyle();
    void testMenuToolButton();
    void testFileDialogPlugin();
    void testMenuToolButtonStyleDraw();
    void testSideBarStyleExtendedDraw();
    void testPathBarAndSideBarDeepCoverage();
};

void TestFileDialogComponentsOffscreen::testPathBar()
{
    FileDialogPathBar bar;
    QVERIFY(bar.getPathBar() != nullptr);
    bar.updateTableModel(true);
    bar.updateTableModel(false);
    bar.updatePath(QStringLiteral("file:///tmp"));
    bar.updatePath(QStringLiteral("/home"));
    bar.updatePath(QString());
    QResizeEvent ev(QSize(400, 48), QSize(300, 40));
    QCoreApplication::sendEvent(&bar, &ev);
    QPaintEvent pe(bar.rect());
    QCoreApplication::sendEvent(&bar, &pe);
}

void TestFileDialogComponentsOffscreen::testSideBarAndStyle()
{
    FileDialogSideBar sidebar;
    sidebar.tableModeChanged(true);
    sidebar.tableModeChanged(false);
    sidebar.menuRequest(QPoint(8, 8));

    SideBarStyle *style = new SideBarStyle();
    sidebar.setStyle(style);

    QWidget widget;
    widget.setProperty("normalRadius", 8);
    QImage surface(120, 120, QImage::Format_ARGB32);
    surface.fill(Qt::transparent);
    QPainter painter(&surface);
    QStyleOption option;
    option.rect = QRect(0, 0, 100, 100);
    option.state = QStyle::State_None;
    option.palette = qApp->style()->standardPalette();
    style->drawPrimitive(QStyle::PE_PanelMenu, &option, &painter, &widget);
    style->drawPrimitive(QStyle::PE_IndicatorItemViewItemDrop, &option, &painter, &widget);

    QStyleOptionViewItem itemOpt;
    itemOpt.rect = option.rect;
    itemOpt.state = QStyle::State_None;
    itemOpt.palette = option.palette;
    itemOpt.state |= QStyle::State_Sunken;
    style->drawPrimitive(QStyle::PE_IndicatorBranch, &itemOpt, &painter, &widget);
    itemOpt.state |= QStyle::State_MouseOver;
    style->drawPrimitive(QStyle::PE_IndicatorBranch, &itemOpt, &painter, &widget);
    itemOpt.rect.setX(0);
    style->drawPrimitive(QStyle::PE_IndicatorBranch, &itemOpt, &painter, &widget);
    style->drawPrimitive(QStyle::PE_PanelItemViewRow, &option, &painter, &widget);
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, &painter, &widget);
    style->drawControl(QStyle::CE_ItemViewItem, &itemOpt, &painter, &widget);

    QPaintEvent paintEv(sidebar.rect());
    QCoreApplication::sendEvent(&sidebar, &paintEv);
    QResizeEvent resizeEv(QSize(200, 400), QSize(180, 300));
    QCoreApplication::sendEvent(&sidebar, &resizeEv);

    FileDialogComboBox combo;
    combo.tableModeChanged(true);
    combo.tableModeChanged(false);
    (void)combo.minimumSizeHint();

    delete style;
}

void TestFileDialogComponentsOffscreen::testMenuToolButton()
{
    MenuToolButton mtb;
    mtb.tableModeChanged(true);
    mtb.tableModeChanged(false);
    mtb.update();
    mtb.setPopupMode(QToolButton::DelayedPopup);
    mtb.setIcon(QIcon::fromTheme("document-open"));
    mtb.setText(QStringLiteral("Open"));
    mtb.setToolButtonStyle(Qt::ToolButtonIconOnly);
    QMenu menu;
    mtb.setMenu(&menu);
}

void TestFileDialogComponentsOffscreen::testMenuToolButtonStyleDraw()
{
    MenuToolButton mtb;
    QMenu menu;
    mtb.setMenu(&menu);
    mtb.setProperty("isOptionButton", false);

    QStyleOptionToolButton opt;
    opt.initFrom(&mtb);
    opt.rect = QRect(0, 0, 48, 32);
    opt.features = QStyleOptionToolButton::HasMenu;
    opt.subControls = QStyle::SC_ToolButton;

    QImage surface(64, 48, QImage::Format_ARGB32);
    surface.fill(Qt::transparent);
    QPainter painter(&surface);
    mtb.style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &painter, &mtb);
    (void)mtb.style()->pixelMetric(QStyle::PM_ToolBarIconSize, &opt, &mtb);
}

void TestFileDialogComponentsOffscreen::testFileDialogPlugin()
{
    FileDialogPlugin plugin;
    QPlatformDialogHelper *helper = plugin.create();
    QVERIFY(helper != nullptr);
    delete helper;
}

void TestFileDialogComponentsOffscreen::testSideBarStyleExtendedDraw()
{
    SideBarStyle style;
    QWidget widget;
    widget.setProperty("normalRadius", 10);
    widget.setProperty("hoverRadius", 12);

    QImage surface(160, 160, QImage::Format_ARGB32);
    surface.fill(Qt::transparent);
    QPainter painter(&surface);

    const QStyle::State states[] = {
        QStyle::State_None,
        QStyle::State_Enabled,
        QStyle::State_Enabled | QStyle::State_Sunken,
        QStyle::State_Enabled | QStyle::State_MouseOver,
        QStyle::State_Enabled | QStyle::State_Selected,
    };

    for (QStyle::State st : states) {
        QStyleOption opt;
        opt.rect = QRect(4, 4, 140, 140);
        opt.state = st;
        opt.palette = qApp->style()->standardPalette();
        style.drawPrimitive(QStyle::PE_PanelMenu, &opt, &painter, &widget);
        style.drawPrimitive(QStyle::PE_PanelItemViewRow, &opt, &painter, &widget);
        style.drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, &painter, &widget);
        style.drawPrimitive(QStyle::PE_IndicatorItemViewItemDrop, &opt, &painter, &widget);

        QStyleOptionViewItem itemOpt;
        itemOpt.rect = opt.rect;
        itemOpt.state = st;
        itemOpt.palette = opt.palette;
        style.drawPrimitive(QStyle::PE_IndicatorBranch, &itemOpt, &painter, &widget);
        style.drawControl(QStyle::CE_ItemViewItem, &itemOpt, &painter, &widget);
    }
}

void TestFileDialogComponentsOffscreen::testPathBarAndSideBarDeepCoverage()
{
    FileDialogPathBar bar;
    bar.updatePath(QStringLiteral("file:///home"));
    bar.updatePath(QStringLiteral("file:///tmp"));
    bar.updateTableModel(true);
    bar.updateTableModel(false);
    if (auto *pathBar = bar.getPathBar()) {
        pathBar->resize(360, 40);
        QResizeEvent pathResize(QSize(360, 40), QSize(300, 40));
        QCoreApplication::sendEvent(pathBar, &pathResize);
    }

    FileDialogSideBar sidebar;
    sidebar.tableModeChanged(true);
    sidebar.tableModeChanged(false);
    sidebar.menuRequest(QPoint(4, 4));

    QResizeEvent sideResize(QSize(220, 420), QSize(200, 400));
    QCoreApplication::sendEvent(&sidebar, &sideResize);
    QPaintEvent paintEv(sidebar.rect());
    QCoreApplication::sendEvent(&sidebar, &paintEv);
}

QTEST_MAIN(TestFileDialogComponentsOffscreen)
#include "test_filedialog_components_offscreen.moc"
