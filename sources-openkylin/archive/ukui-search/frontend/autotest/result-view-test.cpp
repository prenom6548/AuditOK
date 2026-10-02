#include <QApplication>
#include <QAbstractTableModel>
#include <QImage>
#include <QHelpEvent>
#include <QLocale>
#include <QStandardItemModel>
#include <QSignalSpy>
#include <QStyleFactory>
#include <QTest>
#include <QTextDocument>
#include <QTouchDevice>
#include <QTouchEvent>

#define private public
#define protected public
#include "result-view.h"
#undef protected
#undef private

#define protected public
#include "result-view-delegate.h"
#undef protected

using namespace UkuiSearch;

namespace {
class HighlightHelperHarness : public HightLightEffectHelper
{
public:
    using HightLightEffectHelper::HightLightEffectHelper;
    using HightLightEffectHelper::highlightBlock;
};

SearchPluginIface::ResultInfo makeResultInfo(const QString &name,
                                             const QString &pluginOrType = QStringLiteral("file"),
                                             const size_t searchId = 0)
{
    return SearchPluginIface::ResultInfo(
        QIcon(),
        name,
        QStringLiteral("tooltip-") + name,
        pluginOrType,
        {},
        QStringLiteral("action-") + name,
        1,
        true,
        false,
        searchId,
        QStringLiteral("icon-") + name);
}

QPoint firstRowPoint(QTreeView &view)
{
    const QModelIndex index = view.model()->index(0, 0, QModelIndex());
    QRect rect = view.visualRect(index);
    if (!rect.isValid()) {
        view.resize(400, 80);
        view.show();
        qApp->processEvents();
        rect = view.visualRect(index);
    }
    return rect.isValid() ? rect.center() : QPoint(10, 10);
}

void sendMouseSequence(ResultView &view, const QPoint &point)
{
    QMouseEvent press(QEvent::MouseButtonPress,
                      point,
                      view.viewport()->mapToGlobal(point),
                      Qt::LeftButton,
                      Qt::LeftButton,
                      Qt::NoModifier);
    view.mousePressEvent(&press);

    QMouseEvent move(QEvent::MouseMove,
                     point + QPoint(1, 1),
                     view.viewport()->mapToGlobal(point + QPoint(1, 1)),
                     Qt::LeftButton,
                     Qt::LeftButton,
                     Qt::NoModifier);
    view.mouseMoveEvent(&move);

    QMouseEvent release(QEvent::MouseButtonRelease,
                        point,
                        view.viewport()->mapToGlobal(point),
                        Qt::LeftButton,
                        Qt::LeftButton,
                        Qt::NoModifier);
    view.mouseReleaseEvent(&release);
}

QTouchEvent makeTouchEvent(QEvent::Type type, const QPointF &point, Qt::TouchPointStates states)
{
    static QTouchDevice *device = []() {
        auto *touchDevice = new QTouchDevice;
        touchDevice->setType(QTouchDevice::TouchScreen);
        touchDevice->setCapabilities(QTouchDevice::Position);
        return touchDevice;
    }();

    QTouchEvent::TouchPoint touchPoint(1);
    touchPoint.setState(states.testFlag(Qt::TouchPointReleased) ? Qt::TouchPointReleased :
                        states.testFlag(Qt::TouchPointMoved) ? Qt::TouchPointMoved :
                        Qt::TouchPointPressed);
    touchPoint.setPos(point);
    touchPoint.setScreenPos(point);
    touchPoint.setScenePos(point);
    QList<QTouchEvent::TouchPoint> points;
    points.append(touchPoint);
    return QTouchEvent(type, device, Qt::NoModifier, states, points);
}

class OneRowDelegateModel : public QAbstractTableModel
{
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : 1;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : 1;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid()) {
            return {};
        }
        if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
            return QStringLiteral("x");
        }
        if (role == Qt::DecorationRole) {
            QPixmap pixmap(16, 16);
            pixmap.fill(Qt::blue);
            return QIcon(pixmap);
        }
        if (role == IconName) {
            return QStringLiteral("missing-full-text-icon");
        }
        if (role == DisplayNameAsUrl) {
            return false;
        }
        if (role == ShowToolTip) {
            return m_showToolTip;
        }
        return {};
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        if (!index.isValid() || role != ShowToolTip) {
            return false;
        }
        m_showToolTip = value;
        return true;
    }

    QVariant m_showToolTip;
};

}

class ResultViewTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void delegateAcceptsSearchKeywordUpdate();
    void highlightHelperFormatsMatchesOnlyWhenBoldEnabled();
    void highlightHelperHandlesCaseInsensitiveMatch();
    void styleHintDisablesSingleClickActivation();
    void singletonStyleReturnsStableInstance();
    void delegateCoversFontChangeDefaultStyleHintAndEmptyHighlight();
    void delegatePaintUpdatesTooltipRoleForElidedAndFullText();
    void delegatePaintMarksTooltipUnneededWhenFullTextFits();
    void delegatePaintCoversSelectedAndUrlTextBranches();
    void delegatePaintCoversFallbackIconUnselectedAndNullViewBranches();
    void delegatePaintCoversRightToLeftAndSelectedFallbackPen();
    void resultViewCoversSelectionExpansionAndMouseBranches();
    void resultViewCoversTooltipAndClearSelectionBranches();
    void resultViewCoversEmptyMouseAndViewportFallbackBranches();
    void resultViewCoversInvalidSelectionStopAndMenuBranches();
    void resultViewCoversTouchViewportBranches();
};

void ResultViewTest::initTestCase()
{
    qRegisterMetaType<SearchPluginIface::ResultInfo>("SearchPluginIface::ResultInfo");
    if (auto *style = QStyleFactory::create(QStringLiteral("Fusion"))) {
        qApp->setStyle(style);
    }
}

void ResultViewTest::delegateAcceptsSearchKeywordUpdate()
{
    ResultViewDelegate delegate;
    delegate.setSearchKeyword(QStringLiteral("example"));
    delegate.setSearchKeyword(QStringLiteral("another"));
    QVERIFY(true);
}

void ResultViewTest::highlightHelperFormatsMatchesOnlyWhenBoldEnabled()
{
    QTextDocument document;
    document.setPlainText(QStringLiteral("alpha beta alpha"));

    HighlightHelperHarness helper(&document);
    helper.setTextColor(QBrush(Qt::blue));
    helper.setExpression(QStringLiteral("alpha"));
    helper.setDocument(&document);

    helper.setBold(true);
    helper.rehighlight();

    QTextBlock block = document.begin();
    auto formats = block.layout()->formats();
    QVERIFY(!formats.isEmpty());

    bool foundBoldRange = false;
    bool foundBlueRange = false;
    for (const auto &range : formats) {
        if (range.format.fontWeight() == QFont::Bold) {
            foundBoldRange = true;
        }
        if (range.format.foreground().color() == Qt::blue) {
            foundBlueRange = true;
        }
    }
    QVERIFY(foundBoldRange);
    QVERIFY(foundBlueRange);

    helper.setBold(false);
    helper.rehighlight();

    block = document.begin();
    formats = block.layout()->formats();
    QVERIFY(!formats.isEmpty());
    for (const auto &range : formats) {
        QVERIFY(range.format.fontWeight() != QFont::Bold);
    }
}

void ResultViewTest::highlightHelperHandlesCaseInsensitiveMatch()
{
    QTextDocument document;
    document.setPlainText(QStringLiteral("Alpha beta ALPHA gamma"));

    HighlightHelperHarness helper(&document);
    helper.setTextColor(QBrush(Qt::darkGreen));
    helper.setExpression(QStringLiteral("alpha"));
    helper.setDocument(&document);
    helper.setBold(true);
    helper.rehighlight();

    QTextBlock block = document.begin();
    const auto formats = block.layout()->formats();
    int boldRangeCount = 0;
    for (const auto &range : formats) {
        if (range.format.fontWeight() == QFont::Bold) {
            ++boldRangeCount;
            QCOMPARE(range.format.foreground().color(), Qt::darkGreen);
        }
    }
    QVERIFY(boldRangeCount >= 2);
}

void ResultViewTest::styleHintDisablesSingleClickActivation()
{
    QWidget widget;
    const int hint = ResultItemStyle::getStyle()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick,
                                                            nullptr,
                                                            &widget,
                                                            nullptr);
    QCOMPARE(hint, 0);
}

void ResultViewTest::singletonStyleReturnsStableInstance()
{
    ResultItemStyle *first = ResultItemStyle::getStyle();
    ResultItemStyle *second = ResultItemStyle::getStyle();

    QVERIFY(first != nullptr);
    QCOMPARE(first, second);
}

void ResultViewTest::delegateCoversFontChangeDefaultStyleHintAndEmptyHighlight()
{
    ResultViewDelegate delegate;
    const QFont before = delegate.m_textDoc->defaultFont();
    QFont changed = before;
    changed.setPointSize(before.pointSize() + 1);
    qApp->setFont(changed);
    QCOMPARE(delegate.m_textDoc->defaultFont().pointSize(), changed.pointSize());
    qApp->setFont(before);

    QWidget widget;
    const int defaultHint = ResultItemStyle::getStyle()->styleHint(QStyle::SH_Widget_Animate,
                                                                   nullptr,
                                                                   &widget,
                                                                   nullptr);
    QCOMPARE(defaultHint, qApp->style()->styleHint(QStyle::SH_Widget_Animate, nullptr, &widget, nullptr));

    QTextDocument document;
    document.setPlainText(QStringLiteral("plain text"));
    HighlightHelperHarness helper(&document);
    helper.setTextColor(QBrush(Qt::black));
    helper.setExpression(QString());
    helper.setDocument(&document);
    helper.setBold(false);
    helper.rehighlight();
    QVERIFY(!document.begin().layout()->formats().isEmpty());
}

void ResultViewTest::delegatePaintUpdatesTooltipRoleForElidedAndFullText()
{
    QStandardItemModel model;
	auto *item = new QStandardItem(QStringLiteral("short"));
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::blue);
    item->setIcon(QIcon(pixmap));
    item->setData(QStringLiteral("short"), Qt::ToolTipRole);
    item->setData(QStringLiteral("application-x-desktop"), IconName);
    model.appendRow(item);

    QTreeView view;
    ResultViewDelegate delegate(&view);
    view.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    view.setModel(&model);
    view.setItemDelegate(&delegate);
    view.resize(500, 80);

    QStyleOptionViewItem option;
    option.initFrom(&view);
    option.widget = &view;
    option.rect = QRect(0, 0, 480, 40);
    option.state = QStyle::State_Enabled;

    QImage image(260, 80, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    delegate.setSearchKeyword(QStringLiteral("short"));
    delegate.paint(&painter, option, model.index(0, 0));
    painter.end();
    QVERIFY(model.index(0, 0).data(ShowToolTip).isValid());

    model.setData(model.index(0, 0), QString(200, QLatin1Char('x')), Qt::DisplayRole);
    model.setData(model.index(0, 0), QString(200, QLatin1Char('x')), Qt::ToolTipRole);
    QPainter painter2(&image);
    delegate.setSearchKeyword(QStringLiteral("x"));
    delegate.paint(&painter2, option, model.index(0, 0));
    painter2.end();
    QCOMPARE(model.index(0, 0).data(ShowToolTip).toBool(), true);
}

void ResultViewTest::delegatePaintMarksTooltipUnneededWhenFullTextFits()
{
    OneRowDelegateModel model;

    QTreeView view;
    ResultViewDelegate delegate(&view);
    view.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    view.setModel(&model);
    view.resize(900, 80);
    view.setColumnWidth(0, 860);

    QStyleOptionViewItem option;
    option.initFrom(&view);
    option.widget = &view;
    option.rect = QRect(0, 0, 860, 40);
    option.state = QStyle::State_Enabled;

    QImage image(900, 80, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    delegate.setSearchKeyword(QStringLiteral("x"));
    delegate.paint(&painter, option, model.index(0, 0));
    painter.end();

    QCOMPARE(model.m_showToolTip.toBool(), false);
}

void ResultViewTest::delegatePaintCoversSelectedAndUrlTextBranches()
{
    QStandardItemModel model;
    auto *item = new QStandardItem(QStringLiteral("https://example.test/result"));
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::green);
    item->setIcon(QIcon(pixmap));
    item->setData(QStringLiteral("https://example.test/result"), Qt::ToolTipRole);
    item->setData(true, DisplayNameAsUrl);
    item->setData(QStringLiteral("missing-icon-name"), IconName);
    model.appendRow(item);

    QTreeView view;
    ResultViewDelegate delegate(&view);
    view.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    view.setModel(&model);
    view.setItemDelegate(&delegate);
    view.setProperty("textSelectPen", QVariant::fromValue(QPen(Qt::red)));
    view.resize(260, 80);

    QStyleOptionViewItem option;
    option.initFrom(&view);
    option.widget = &view;
    option.rect = QRect(0, 0, 240, 40);
    option.state = QStyle::State_Enabled | QStyle::State_Selected;

    QImage image(280, 80, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    delegate.setSearchKeyword(QStringLiteral("example"));
    delegate.paint(&painter, option, model.index(0, 0));
    painter.end();

    QVERIFY(model.index(0, 0).data(ShowToolTip).isValid());
}

void ResultViewTest::delegatePaintCoversFallbackIconUnselectedAndNullViewBranches()
{
    QStandardItemModel model;
    auto *urlItem = new QStandardItem(QStringLiteral("https://kylin.test/branch"));
    urlItem->setData(QStringLiteral("https://kylin.test/branch"), Qt::ToolTipRole);
    urlItem->setData(true, DisplayNameAsUrl);
    urlItem->setData(QStringLiteral("missing-url-icon"), IconName);
    model.appendRow(urlItem);

    auto *plainItem = new QStandardItem(QStringLiteral("plain branch text"));
    plainItem->setData(QStringLiteral("plain branch text"), Qt::ToolTipRole);
    plainItem->setData(false, DisplayNameAsUrl);
    plainItem->setData(QStringLiteral("missing-plain-icon"), IconName);
    model.appendRow(plainItem);

    QTreeView view;
    ResultViewDelegate delegate(&view);
    view.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    view.setModel(&model);
    view.setItemDelegate(&delegate);
    view.resize(360, 100);

    QStyleOptionViewItem option;
    option.initFrom(&view);
    option.widget = &view;
    option.rect = QRect(0, 0, 320, 36);
    option.state = QStyle::State_Enabled;

    QImage image(360, 100, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    delegate.setSearchKeyword(QStringLiteral("branch"));
    delegate.paint(&painter, option, model.index(0, 0));
    delegate.setSearchKeyword(QStringLiteral("plain"));
    delegate.paint(&painter, option, model.index(1, 0));
    painter.end();

    QVERIFY(model.index(0, 0).data(ShowToolTip).isValid());
    QVERIFY(model.index(1, 0).data(ShowToolTip).isValid());

    ResultViewDelegate orphanDelegate;
    QStyleOptionViewItem orphanOption = option;
    orphanOption.widget = &view;
    QPainter orphanPainter(&image);
    orphanDelegate.paint(&orphanPainter, orphanOption, model.index(0, 0));
    orphanPainter.end();
}

void ResultViewTest::delegatePaintCoversRightToLeftAndSelectedFallbackPen()
{
    QStandardItemModel model;
    auto *item = new QStandardItem(QStringLiteral("rtl selected branch text"));
    item->setData(QStringLiteral("rtl selected branch text"), Qt::ToolTipRole);
    item->setData(false, DisplayNameAsUrl);
    item->setData(QStringLiteral("missing-rtl-icon"), IconName);
    model.appendRow(item);

    QTreeView view;
    ResultViewDelegate delegate(&view);
    view.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    view.setModel(&model);
    view.setItemDelegate(&delegate);
    view.setLayoutDirection(Qt::RightToLeft);
    view.setProperty("textSelectPen", QVariant(QStringLiteral("not-a-pen")));
    view.resize(260, 80);

    QStyleOptionViewItem option;
    option.initFrom(&view);
    option.widget = &view;
    option.rect = QRect(0, 0, 240, 40);
    option.state = QStyle::State_Enabled | QStyle::State_Selected;
    option.direction = Qt::RightToLeft;

    QImage image(280, 80, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    delegate.setSearchKeyword(QStringLiteral("branch"));
    delegate.paint(&painter, option, model.index(0, 0));
    painter.end();

    QVERIFY(model.index(0, 0).data(ShowToolTip).isValid());
}

void ResultViewTest::resultViewCoversSelectionExpansionAndMouseBranches()
{
    ResultView view(QStringLiteral("File"));
    view.resize(420, 160);
    view.show();
    qApp->processEvents();

    QSignalSpy listLengthSpy(&view, SIGNAL(listLengthChanged(int)));
    QSignalSpy currentRowSpy(&view, SIGNAL(currentRowChanged(QString,SearchPluginIface::ResultInfo)));
    QSignalSpy clickedSpy(&view, SIGNAL(clicked(QModelIndex)));
    QVERIFY(listLengthSpy.isValid());
    QVERIFY(currentRowSpy.isValid());
    QVERIFY(clickedSpy.isValid());

    view.m_model->appendInfo(makeResultInfo(QStringLiteral("alpha")));
    view.m_model->appendInfo(makeResultInfo(QStringLiteral("beta")));
    view.onItemListChanged(2);

    QCOMPARE(view.getResultNum(), 2);
    QVERIFY(!view.isExpanded());
    QVERIFY(view.showHeight() >= 0);

    QModelIndex first = view.getModlIndex(0, 0);
    QVERIFY(first.isValid());
    QCOMPARE(view.getIndexResultInfo(first).name, QStringLiteral("alpha"));

    view.onRowSelectedSlot(first);
    QVERIFY(view.isSelected());
    QCOMPARE(view.currentIndex(), first);
    QVERIFY(!currentRowSpy.isEmpty());

    view.clearSelectedRow();
    QVERIFY(!view.isSelected());
    view.clearSelectedRow();
    QVERIFY(!view.currentIndex().isValid());

    view.setExpanded(true);
    QVERIFY(view.isExpanded());
    view.setExpanded(false);
    QVERIFY(!view.isExpanded());

    const QPoint point = firstRowPoint(view);
    sendMouseSequence(view, point);
    QVERIFY(clickedSpy.count() >= 1);
}

void ResultViewTest::resultViewCoversTooltipAndClearSelectionBranches()
{
    ResultView view(QStringLiteral("File"));
    view.resize(420, 160);
    view.show();
    qApp->processEvents();

    view.m_model->appendInfo(makeResultInfo(QStringLiteral("tooltip")));
    view.onItemListChanged(1);

    QModelIndex first = view.getModlIndex(0, 0);
    QVERIFY(first.isValid());
    QVERIFY(view.m_model->setData(first, true, ShowToolTip));

    const QPoint point = firstRowPoint(view);
    QHelpEvent showTip(QEvent::ToolTip, point, view.viewport()->mapToGlobal(point));
    QVERIFY(QApplication::sendEvent(view.viewport(), &showTip));

    QVERIFY(view.m_model->setData(first, false, ShowToolTip));
    QHelpEvent hideTip(QEvent::ToolTip, point, view.viewport()->mapToGlobal(point));
    QVERIFY(QApplication::sendEvent(view.viewport(), &hideTip));

    QHelpEvent invalidTip(QEvent::ToolTip, QPoint(400, 140), view.viewport()->mapToGlobal(QPoint(400, 140)));
    QApplication::sendEvent(view.viewport(), &invalidTip);

}

void ResultViewTest::resultViewCoversEmptyMouseAndViewportFallbackBranches()
{
    ResultView emptyView(QStringLiteral("File"));
    emptyView.resize(420, 160);
    emptyView.show();
    qApp->processEvents();

    QCOMPARE(emptyView.showHeight(), 0);
    emptyView.setExpanded(true);
    QCOMPARE(emptyView.showHeight(), 0);
    emptyView.onRowSelectedSlot(QModelIndex());
    QVERIFY(!emptyView.isSelected());

    QSignalSpy emptyClickedSpy(&emptyView, SIGNAL(clicked(QModelIndex)));
    QVERIFY(emptyClickedSpy.isValid());
    QMouseEvent emptyPress(QEvent::MouseButtonPress,
                           QPoint(390, 140),
                           emptyView.viewport()->mapToGlobal(QPoint(390, 140)),
                           Qt::LeftButton,
                           Qt::LeftButton,
                           Qt::NoModifier);
    emptyView.mousePressEvent(&emptyPress);
    QMouseEvent emptyMove(QEvent::MouseMove,
                          QPoint(391, 141),
                          emptyView.viewport()->mapToGlobal(QPoint(391, 141)),
                          Qt::LeftButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
    emptyView.mouseMoveEvent(&emptyMove);
    QMouseEvent emptyRelease(QEvent::MouseButtonRelease,
                             QPoint(390, 140),
                             emptyView.viewport()->mapToGlobal(QPoint(390, 140)),
                             Qt::LeftButton,
                             Qt::LeftButton,
                             Qt::NoModifier);
    emptyView.mouseReleaseEvent(&emptyRelease);
    QCOMPARE(emptyClickedSpy.count(), 1);

    ResultView view(QStringLiteral("File"));
    view.resize(420, 160);
    view.show();
    qApp->processEvents();
    view.m_model->appendInfo(makeResultInfo(QStringLiteral("row-a")));
    view.m_model->appendInfo(makeResultInfo(QStringLiteral("row-b")));
    view.onItemListChanged(2);
    const QModelIndex first = view.getModlIndex(0, 0);
    QVERIFY(first.isValid());
    view.setCurrentIndex(first);

    QSignalSpy clickedSpy(&view, SIGNAL(clicked(QModelIndex)));
    QVERIFY(clickedSpy.isValid());
    const QPoint firstPoint = firstRowPoint(view);
    QMouseEvent samePress(QEvent::MouseButtonPress,
                          firstPoint,
                          view.viewport()->mapToGlobal(firstPoint),
                          Qt::LeftButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
    view.mousePressEvent(&samePress);
    QCOMPARE(clickedSpy.count(), 0);

    QEvent resizeEvent(QEvent::Resize);
    view.viewportEvent(&resizeEvent);

    QHelpEvent invalidTip(QEvent::ToolTip, QPoint(410, 150), view.viewport()->mapToGlobal(QPoint(410, 150)));
    QCOMPARE(view.viewportEvent(&invalidTip), false);
}

void ResultViewTest::resultViewCoversInvalidSelectionStopAndMenuBranches()
{
    ResultView view(QStringLiteral("whitebox-missing-plugin"));
    view.resize(420, 160);
    view.show();
    qApp->processEvents();

    QSignalSpy currentRowSpy(&view, SIGNAL(currentRowChanged(QString,SearchPluginIface::ResultInfo)));
    QSignalSpy clickedSpy(&view, SIGNAL(clicked(QModelIndex)));
    QSignalSpy listLengthSpy(&view, SIGNAL(listLengthChanged(int)));
    QVERIFY(currentRowSpy.isValid());
    QVERIFY(clickedSpy.isValid());
    QVERIFY(listLengthSpy.isValid());

    view.m_model->appendInfo(makeResultInfo(QStringLiteral("only")));
    view.onItemListChanged(1);
    const QModelIndex first = view.getModlIndex(0, 0);
    QVERIFY(first.isValid());

    view.onRowSelectedSlot(QModelIndex());
    QCOMPARE(currentRowSpy.count(), 0);
    QVERIFY(!view.isSelected());

    QMouseEvent releaseOutside(QEvent::MouseButtonRelease,
                               QPoint(400, 150),
                               view.viewport()->mapToGlobal(QPoint(400, 150)),
                               Qt::LeftButton,
                               Qt::LeftButton,
                               Qt::NoModifier);
    view.mouseReleaseEvent(&releaseOutside);
    QVERIFY(clickedSpy.count() >= 1);

    view.setCurrentIndex(first);
    QMouseEvent synthesizedMove(QEvent::MouseMove,
                                firstRowPoint(view) + QPoint(2, 2),
                                firstRowPoint(view) + QPoint(2, 2),
                                view.viewport()->mapToGlobal(firstRowPoint(view) + QPoint(2, 2)),
                                Qt::LeftButton,
                                Qt::LeftButton,
                                Qt::NoModifier,
                                Qt::MouseEventSynthesizedByQt);
    const int beforeSynthesizedMove = clickedSpy.count();
    view.mouseMoveEvent(&synthesizedMove);
    QCOMPARE(clickedSpy.count(), beforeSynthesizedMove);

    QObject::disconnect(view.m_model, SIGNAL(stopSearch()), nullptr, nullptr);
    view.m_model->stopSearchSlot();
    QVERIFY(listLengthSpy.count() >= 1);
    QVERIFY(!view.getModlIndex(0, 0).isValid());
}

void ResultViewTest::resultViewCoversTouchViewportBranches()
{
    QWidget parent;
    ResultView view(QStringLiteral("File"), &parent);
    parent.resize(460, 220);
    view.resize(420, 160);
    parent.show();
    view.show();
    qApp->processEvents();

    view.m_model->appendInfo(makeResultInfo(QStringLiteral("touch")));
    view.onItemListChanged(1);

    QTouchEvent begin = makeTouchEvent(QEvent::TouchBegin, QPointF(12, 12), Qt::TouchPointPressed);
    QVERIFY(view.viewportEvent(&begin));
    QVERIFY(begin.isAccepted());
    QVERIFY(view.m_touchTimer->isActive());

    QTouchEvent update = makeTouchEvent(QEvent::TouchUpdate, QPointF(16, 18), Qt::TouchPointMoved);
    QVERIFY(view.viewportEvent(&update));

    QTouchEvent end = makeTouchEvent(QEvent::TouchEnd, QPointF(16, 18), Qt::TouchPointReleased);
    QVERIFY(view.viewportEvent(&end));
}

QTEST_MAIN(ResultViewTest)

#include "result-view-test.moc"
