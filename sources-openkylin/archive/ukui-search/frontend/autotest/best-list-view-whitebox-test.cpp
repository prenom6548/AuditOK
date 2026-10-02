#include <QApplication>
#include <QHelpEvent>
#include <QSignalSpy>
#include <QStyleFactory>
#include <QTest>
#include <QTouchDevice>
#include <QTouchEvent>

#include "global-settings.h"
#include "icon-loader.h"

#define private public
#define protected public
#include "best-list-view.h"
#undef protected
#undef private

using namespace UkuiSearch;

namespace UkuiSearch {

namespace {
int g_openActionCalls = 0;
QString g_lastActionKey;

class DummySearchPlugin : public SearchPluginIface
{
public:
    PluginType pluginType() override { return PluginType::SearchPlugin; }
    const QString name() override { return QStringLiteral("File"); }
    const QString description() override { return QStringLiteral("dummy"); }
    const QIcon icon() override { return QIcon(); }
    void setEnable(bool enable) override { m_enabled = enable; }
    bool isEnable() override { return m_enabled; }
    QString getPluginName() override { return QStringLiteral("Dummy File"); }
    size_t KeywordSearch(QString keyword, DataQueue<ResultInfo> *searchResult) override
    {
        Q_UNUSED(keyword)
        Q_UNUSED(searchResult)
        return 0;
    }
    void stopSearch() override {}
    QList<Actioninfo> getActioninfo(int type) override
    {
        Q_UNUSED(type)
        return {};
    }
    void openAction(int actionkey, QString key, int type) override
    {
        Q_UNUSED(actionkey)
        Q_UNUSED(type)
        ++g_openActionCalls;
        g_lastActionKey = key;
    }
    QWidget *detailPage(const ResultInfo &ri) override
    {
        Q_UNUSED(ri)
        return new QWidget;
    }

private:
    bool m_enabled = true;
};
}

SearchPluginManager *SearchPluginManager::getInstance()
{
    static SearchPluginManager *manager = reinterpret_cast<SearchPluginManager *>(new QObject);
    return manager;
}

const QList<PluginInfo> SearchPluginManager::getPluginIds()
{
    return {
        PluginInfo(QStringLiteral("File"), true, 1, false, false),
        PluginInfo(QStringLiteral("App"), true, 2, false, false),
    };
}

SearchPluginIface *SearchPluginManager::getPlugin(const QString &pluginId)
{
    static DummySearchPlugin plugin;
    if (pluginId == QStringLiteral("File") || pluginId == QStringLiteral("App")) {
        return &plugin;
    }
    return nullptr;
}

SearchPluginManager::~SearchPluginManager() = default;

GlobalSettings &GlobalSettings::getInstance()
{
    static GlobalSettings *settings = reinterpret_cast<GlobalSettings *>(new QObject);
    return *settings;
}

const QVariant GlobalSettings::getValue(const QString &key)
{
    if (key == FONT_SIZE_KEY) {
        return 11;
    }
    if (key == FONT_KEY) {
        return QStringLiteral("Sans");
    }
    if (key == TRANSPARENCY_KEY) {
        return 1.0;
    }
    return {};
}

QIcon IconLoader::loadIconQt(const QString &name, const QIcon &fallback)
{
    Q_UNUSED(name)
    return fallback;
}

QIcon IconLoader::loadIconXdg(const QString &name, const QIcon &fallback)
{
    Q_UNUSED(name)
    return fallback;
}

}

namespace {
SearchPluginIface::ResultInfo makeResultInfo(const QString &name,
                                             const QString &resourceType = QStringLiteral("file"))
{
    return SearchPluginIface::ResultInfo(
        QIcon(),
        name,
        QStringLiteral("tooltip-") + name,
        resourceType,
        {},
        QStringLiteral("action-") + name,
        1,
        true,
        false,
        0,
        QStringLiteral("icon-") + name);
}

void appendRawItem(BestListView &view,
                   const QString &pluginId,
                   const SearchPluginIface::ResultInfo &info)
{
    view.m_model->m_pluginIdList.append(pluginId);
    view.m_model->m_resourceTypes.append(info.resourceType);
    view.m_model->m_items.append(new OneResult(info));
}

QPoint firstRowPoint(BestListView &view)
{
    const QModelIndex index = view.model()->index(0, 0, QModelIndex());
    const QRect rect = view.visualRect(index);
    return rect.isValid() ? rect.center() : QPoint(10, 10);
}

void sendMouseSequence(BestListView &view, const QPoint &point)
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
}

class BestListViewWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void coversSelectionExpansionMouseAndTooltipBranches();
    void coversExpandedHeightSameIndexAndFallbackViewportBranches();
    void coversBestListWidgetFacadeAndSignalBranches();
    void coversBestListWidgetPaletteAndCurrentIndexPreservationBranches();
    void coversPluginOpenAndManagerSignalBranches();
    void coversStartStopMissingPluginAndEmptyActionBranches();
    void coversTouchViewportBranches();
};

void BestListViewWhiteboxTest::initTestCase()
{
    qRegisterMetaType<SearchPluginIface::ResultInfo>("SearchPluginIface::ResultInfo");
    if (auto *style = QStyleFactory::create(QStringLiteral("Fusion"))) {
        qApp->setStyle(style);
    }
}

void BestListViewWhiteboxTest::coversSelectionExpansionMouseAndTooltipBranches()
{
    BestListView view;
    view.resize(420, 160);
    view.show();
    qApp->processEvents();

    QSignalSpy listLengthSpy(&view, SIGNAL(listLengthChanged(int)));
    QSignalSpy currentRowSpy(&view, SIGNAL(currentRowChanged(QString,SearchPluginIface::ResultInfo)));
    QSignalSpy clickedSpy(&view, SIGNAL(clicked(QModelIndex)));
    QVERIFY(listLengthSpy.isValid());
    QVERIFY(currentRowSpy.isValid());
    QVERIFY(clickedSpy.isValid());

    appendRawItem(view, QStringLiteral("File"), makeResultInfo(QStringLiteral("best"), QStringLiteral("file")));
    view.onItemListChanged(1);

    QCOMPARE(view.getResultNum(), 1);
    QVERIFY(!view.isExpanded());
    QVERIFY(view.showHeight() >= 0);
    QVERIFY(view.getResultHeight() >= 0);

    QModelIndex first = view.getModlIndex(0, 0);
    QVERIFY(first.isValid());
    QCOMPARE(view.getPluginInfo(first), QStringLiteral("File"));
    QCOMPARE(view.getIndexResultInfo(first).name, QStringLiteral("best"));

    view.onRowSelectedSlot(first);
    QVERIFY(view.isSelected());
    QCOMPARE(view.currentIndex(), first);
    QVERIFY(!currentRowSpy.isEmpty());

    view.clearSelectedRow();
    QCOMPARE(view.currentIndex(), first);
    view.m_is_selected = false;
    view.clearSelectedRow();
    QVERIFY(!view.currentIndex().isValid());

    view.setExpanded(true);
    QVERIFY(view.isExpanded());
    view.setExpanded(false);
    QVERIFY(!view.isExpanded());

    const QPoint point = firstRowPoint(view);
    sendMouseSequence(view, point);
    QVERIFY(clickedSpy.count() >= 1);

    QVERIFY(view.m_model->setData(first, true, ShowToolTip));
    QHelpEvent showTip(QEvent::ToolTip, point, view.viewport()->mapToGlobal(point));
    QVERIFY(QApplication::sendEvent(view.viewport(), &showTip));

    QVERIFY(view.m_model->setData(first, false, ShowToolTip));
    QHelpEvent hideTip(QEvent::ToolTip, point, view.viewport()->mapToGlobal(point));
    QVERIFY(QApplication::sendEvent(view.viewport(), &hideTip));

    QHelpEvent invalidTip(QEvent::ToolTip, QPoint(400, 140), view.viewport()->mapToGlobal(QPoint(400, 140)));
    QApplication::sendEvent(view.viewport(), &invalidTip);
}

void BestListViewWhiteboxTest::coversExpandedHeightSameIndexAndFallbackViewportBranches()
{
    BestListView view;
    view.resize(420, 220);
    view.show();
    qApp->processEvents();

    for (int i = 0; i < 7; ++i) {
        appendRawItem(view, QStringLiteral("File"), makeResultInfo(QStringLiteral("row-%1").arg(i)));
    }
    view.onItemListChanged(7);

    const int collapsedHeight = view.showHeight();
    view.setExpanded(true);
    const int expandedHeight = view.showHeight();
    QVERIFY(expandedHeight >= collapsedHeight);
    view.setExpanded(false);

    QModelIndex first = view.getModlIndex(0, 0);
    QVERIFY(first.isValid());
    view.setCurrentIndex(first);

    QSignalSpy clickedSpy(&view, SIGNAL(clicked(QModelIndex)));
    QVERIFY(clickedSpy.isValid());
    const QPoint point = firstRowPoint(view);
    QMouseEvent samePress(QEvent::MouseButtonPress,
                          point,
                          view.viewport()->mapToGlobal(point),
                          Qt::LeftButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
    view.mousePressEvent(&samePress);
    QCOMPARE(clickedSpy.count(), 0);

    QMouseEvent synthesizedMove(QEvent::MouseMove,
                                point + QPoint(2, 2),
                                point + QPoint(2, 2),
                                view.viewport()->mapToGlobal(point + QPoint(2, 2)),
                                Qt::LeftButton,
                                Qt::LeftButton,
                                Qt::NoModifier,
                                Qt::MouseEventSynthesizedByQt);
    view.mouseMoveEvent(&synthesizedMove);
    QCOMPARE(clickedSpy.count(), 0);

    view.onRowSelectedSlot(QModelIndex());
    QCOMPARE(view.currentIndex(), first);

    QEvent resizeEvent(QEvent::Resize);
    view.viewportEvent(&resizeEvent);

    QHelpEvent invalidTip(QEvent::ToolTip, QPoint(410, 210), view.viewport()->mapToGlobal(QPoint(410, 210)));
    QCOMPARE(view.viewportEvent(&invalidTip), false);
}

void BestListViewWhiteboxTest::coversBestListWidgetFacadeAndSignalBranches()
{
    BestListWidget widget;
    widget.resize(420, 180);
    widget.show();
    qApp->processEvents();

    QSignalSpy sizeSpy(&widget, SIGNAL(sizeChanged()));
    QSignalSpy currentRowSpy(&widget, SIGNAL(currentRowChanged(QString,SearchPluginIface::ResultInfo)));
    QVERIFY(sizeSpy.isValid());
    QVERIFY(currentRowSpy.isValid());

    widget.setEnabled(false);
    QCOMPARE(widget.m_enabled, false);
    Q_EMIT widget.sendBestListData(QStringLiteral("File"), makeResultInfo(QStringLiteral("facade")));
    widget.m_bestListView->onItemListChanged(1);
    widget.onListLengthChanged(1);

    QCOMPARE(widget.getResultNum(), 1);
    QVERIFY(widget.isVisible());
    QVERIFY(!sizeSpy.isEmpty());
    QModelIndex first = widget.getModlIndex(0, 0);
    QVERIFY(first.isValid());
    QCOMPARE(widget.getIndexResultInfo(first).name, QStringLiteral("facade"));
    QCOMPARE(widget.getPluginInfo(first), QStringLiteral("File"));
    QVERIFY(widget.getResultHeight() >= 0);

    widget.setResultSelection(first);
    QCOMPARE(widget.getCurrentSelection(), first);
    widget.m_bestListView->onRowSelectedSlot(first);
    QVERIFY(!currentRowSpy.isEmpty());

    widget.expandListSlot();
    QVERIFY(widget.getExpandState());
    widget.reduceListSlot();
    QVERIFY(!widget.getExpandState());

    widget.clearResultSelection();
    QVERIFY(!widget.getCurrentSelection().isValid());
    Q_EMIT widget.clearSelectedRow();
    QVERIFY(!widget.getCurrentSelection().isValid());

    widget.clearResult();
    QVERIFY(!widget.isVisible());
    QCOMPARE(widget.height(), 0);
    widget.onListLengthChanged(0);
    QCOMPARE(widget.height(), 0);
}

void BestListViewWhiteboxTest::coversBestListWidgetPaletteAndCurrentIndexPreservationBranches()
{
    BestListWidget widget;
    widget.resize(420, 220);
    widget.show();
    qApp->processEvents();

    QSignalSpy sizeSpy(&widget, SIGNAL(sizeChanged()));
    QVERIFY(sizeSpy.isValid());

    Q_EMIT widget.sendBestListData(QStringLiteral("File"), makeResultInfo(QStringLiteral("keep-current")));
    widget.m_bestListView->onItemListChanged(1);
    QModelIndex first = widget.getModlIndex(0, 0);
    QVERIFY(first.isValid());
    widget.setResultSelection(first);

    Q_EMIT widget.sendBestListData(QStringLiteral("App"), makeResultInfo(QStringLiteral("append-while-selected")));
    QCOMPARE(widget.getCurrentSelection(), first);

    const int beforePaletteVisible = sizeSpy.count();
    Q_EMIT qApp->paletteChanged(qApp->palette());
    QVERIFY(sizeSpy.count() > beforePaletteVisible);

    widget.clearResult();
    const int beforePaletteHidden = sizeSpy.count();
    Q_EMIT qApp->paletteChanged(qApp->palette());
    QVERIFY(sizeSpy.count() > beforePaletteHidden);
    QCOMPARE(widget.height(), 0);
}

void BestListViewWhiteboxTest::coversPluginOpenAndManagerSignalBranches()
{
    BestListView view;
    appendRawItem(view, QStringLiteral("File"), makeResultInfo(QStringLiteral("open")));
    view.onItemListChanged(1);
    QModelIndex first = view.getModlIndex(0, 0);
    QVERIFY(first.isValid());

    g_openActionCalls = 0;
    g_lastActionKey.clear();
    view.onRowDoubleClickedSlot(first);
    QCOMPARE(g_openActionCalls, 1);
    QCOMPARE(g_lastActionKey, QStringLiteral("action-open"));

    Q_EMIT view.sendBestListData(QStringLiteral("App"), makeResultInfo(QStringLiteral("new-app")));
    QCOMPARE(view.m_model->m_items.size(), 2);
    view.setCurrentIndex(first);

    SearchPluginManager *manager = SearchPluginManager::getInstance();
    Q_EMIT manager->changePos(QStringLiteral("App"), 1);
    QCOMPARE(view.getPluginInfo(view.getModlIndex(0, 0)), QStringLiteral("App"));
    Q_EMIT manager->unregistered(QStringLiteral("App"));
    QCOMPARE(view.m_model->m_items.size(), 1);
}

void BestListViewWhiteboxTest::coversStartStopMissingPluginAndEmptyActionBranches()
{
    BestListView view;
    view.resize(420, 160);
    view.show();
    qApp->processEvents();

    QSignalSpy listLengthSpy(&view, SIGNAL(listLengthChanged(int)));
    QSignalSpy clickedSpy(&view, SIGNAL(clicked(QModelIndex)));
    QVERIFY(listLengthSpy.isValid());
    QVERIFY(clickedSpy.isValid());

    Q_EMIT view.startSearch(QStringLiteral("best keyword"));
    appendRawItem(view, QStringLiteral("MissingPlugin"), makeResultInfo(QStringLiteral("missing-plugin")));
    SearchPluginIface::ResultInfo emptyAction = makeResultInfo(QStringLiteral("empty-action"));
    emptyAction.actionKey.clear();
    appendRawItem(view, QStringLiteral("File"), emptyAction);
    view.onItemListChanged(2);

    const QModelIndex missingPluginIndex = view.getModlIndex(0, 0);
    const QModelIndex emptyActionIndex = view.getModlIndex(1, 0);
    QVERIFY(missingPluginIndex.isValid());
    QVERIFY(emptyActionIndex.isValid());

    g_openActionCalls = 0;
    view.onRowDoubleClickedSlot(missingPluginIndex);
    view.onRowDoubleClickedSlot(emptyActionIndex);
    QCOMPARE(g_openActionCalls, 0);

    view.setCurrentIndex(emptyActionIndex);
    QMouseEvent releaseOutside(QEvent::MouseButtonRelease,
                               QPoint(390, 140),
                               view.viewport()->mapToGlobal(QPoint(390, 140)),
                               Qt::LeftButton,
                               Qt::LeftButton,
                               Qt::NoModifier);
    view.mouseReleaseEvent(&releaseOutside);
    QVERIFY(clickedSpy.count() >= 1);

    Q_EMIT view.stopSearch();
    QVERIFY(listLengthSpy.count() >= 1);
    QVERIFY(!view.getModlIndex(0, 0).isValid());
}

void BestListViewWhiteboxTest::coversTouchViewportBranches()
{
    QWidget parent;
    BestListView view(&parent);
    parent.resize(460, 220);
    view.resize(420, 160);
    parent.show();
    view.show();
    qApp->processEvents();

    appendRawItem(view, QStringLiteral("File"), makeResultInfo(QStringLiteral("touch")));
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

QTEST_MAIN(BestListViewWhiteboxTest)

#include "best-list-view-whitebox-test.moc"
