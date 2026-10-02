#include <QApplication>
#include <QSignalSpy>
#include <QStyleFactory>
#include <QTest>

#include "global-settings.h"
#include "icon-loader.h"

#define private public
#define protected public
#include "search-result-page.h"
#undef protected
#undef private

using namespace UkuiSearch;

QString escapeHtml(const QString &str);

namespace UkuiSearch {

namespace {
class DummySearchPlugin : public SearchPluginIface
{
public:
    explicit DummySearchPlugin(const QString &pluginId) : m_pluginId(pluginId) {}

    PluginType pluginType() override { return PluginType::SearchPlugin; }
    const QString name() override { return m_pluginId; }
    const QString description() override { return QStringLiteral("dummy"); }
    const QIcon icon() override { return QIcon(); }
    void setEnable(bool enable) override { m_enabled = enable; }
    bool isEnable() override { return m_enabled; }
    QString getPluginName() override { return QStringLiteral("Plugin ") + m_pluginId; }
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
        ++m_openActionCalls;
        m_lastActionKey = key;
    }
    QWidget *detailPage(const ResultInfo &ri) override
    {
        Q_UNUSED(ri)
        ++m_detailPageCalls;
        auto *label = new QLabel(QStringLiteral("detail-") + m_pluginId);
        label->setObjectName(QStringLiteral("detail-") + m_pluginId);
        return label;
    }

    int m_openActionCalls = 0;
    int m_detailPageCalls = 0;
    QString m_lastActionKey;

private:
    QString m_pluginId;
    bool m_enabled = true;
};

QHash<QString, DummySearchPlugin *> &plugins()
{
    static QHash<QString, DummySearchPlugin *> instances;
    return instances;
}

DummySearchPlugin *pluginFor(const QString &pluginId)
{
    auto &map = plugins();
    if (!map.contains(pluginId)) {
        map.insert(pluginId, new DummySearchPlugin(pluginId));
    }
    return map.value(pluginId);
}
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
        PluginInfo(QStringLiteral("Web Page"), true, 8, false, false),
    };
}

SearchPluginIface *SearchPluginManager::getPlugin(const QString &pluginId)
{
    return pluginFor(pluginId);
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
    if (key == WINDOW_RADIUS_KEY) {
        return 6;
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
                                             const QString &resourceType = QStringLiteral("file"),
                                             size_t searchId = 0)
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
        searchId,
        QStringLiteral("icon-") + name);
}

void appendBestItem(ResultArea &area, const QString &pluginId, const SearchPluginIface::ResultInfo &info)
{
    Q_EMIT area.m_bestListWidget->sendBestListData(pluginId, info);
    area.m_bestListWidget->m_bestListView->onItemListChanged(area.m_bestListWidget->m_bestListView->m_model->m_items.size());
    area.m_bestListWidget->onListLengthChanged(area.m_bestListWidget->getResultNum());
}

void appendResultItem(ResultWidget *widget, const SearchPluginIface::ResultInfo &info)
{
    auto item = info;
    item.searchID = widget->m_resultView->m_model->m_searchID;
    widget->m_resultView->m_model->appendInfo(item);
    widget->m_resultView->onItemListChanged(widget->m_resultView->m_model->m_items.size());
    widget->onListLengthChanged(widget->getResultNum());
}
}

class StackPagesWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void resultAreaCoversWidgetManagementAndKeyboardBranches();
    void resultAreaCoversKeyboardBoundaryBranches();
    void resultAreaCoversAdditionalSelectionAndFloatingBranches();
    void resultAreaCoversEmptyBestLastPluginAndInvalidSelectionBranches();
    void resultAreaCoversFloatingTitleScrollbarAndDetailBranches();
    void resultAreaCoversMoreBoundaryMouseAndScrollBranches();
    void resultAreaCoversBestListWithinBoundsAndNonBestActivationBranches();
    void resultAreaCoversBestListSignalsAndWidgetRetractBranches();
    void searchResultPageCoversPluginOrderingResizeAndDetailBranches();
    void searchResultPageCoversOrderedAppendRowClickAndUtilityBranches();
};

void StackPagesWhiteboxTest::initTestCase()
{
    qRegisterMetaType<SearchPluginIface::ResultInfo>("SearchPluginIface::ResultInfo");
    if (auto *style = QStyleFactory::create(QStringLiteral("Fusion"))) {
        qApp->setStyle(style);
    }
}

void StackPagesWhiteboxTest::resultAreaCoversWidgetManagementAndKeyboardBranches()
{
    ResultArea area;
    area.resize(656, 516);
    area.show();
    qApp->processEvents();

    QSignalSpy resizeSpy(&area, SIGNAL(resizeHeight(int)));
    QSignalSpy keySpy(&area, SIGNAL(keyPressChanged(QString,SearchPluginIface::ResultInfo)));
    QVERIFY(resizeSpy.isValid());
    QVERIFY(keySpy.isValid());

    auto *fileWidget = new ResultWidget(QStringLiteral("File"), &area);
    auto *webWidget = new ResultWidget(QStringLiteral("Web Page"), &area);
    area.appendWidet(fileWidget);
    area.insertWidget(webWidget, 1);
    QCOMPARE(area.m_widgetList.size(), 2);
    QCOMPARE(area.m_widgetList.at(1)->pluginId(), QStringLiteral("Web Page"));

    area.setVisibleList(QStringList{QStringLiteral("File")});
    QCOMPARE(fileWidget->m_enabled, true);
    QCOMPARE(webWidget->m_enabled, false);

    appendBestItem(area, QStringLiteral("File"), makeResultInfo(QStringLiteral("best-one")));
    appendResultItem(fileWidget, makeResultInfo(QStringLiteral("file-one")));
    appendResultItem(webWidget, makeResultInfo(QStringLiteral("web-one"), QStringLiteral("web")));
    area.onWidgetSizeChanged();
    QVERIFY(!resizeSpy.isEmpty());

    area.pressEnter();
    QVERIFY(area.getSelectedState());
    QCOMPARE(area.m_selectedPluginID, area.m_bestListWidget->getWidgetName());
    area.pressDown();
    area.pressUp();
    area.pressEnter();
    QVERIFY(area.m_detail_open_state);
    const int openCallsBefore = pluginFor(QStringLiteral("File"))->m_openActionCalls;
    area.pressEnter();
    QVERIFY(pluginFor(QStringLiteral("File"))->m_openActionCalls > openCallsBefore);

    QString fileId(QStringLiteral("File"));
    area.setSelectionInfo(fileId);
    QCOMPARE(area.m_selectedPluginID, QStringLiteral("File"));
    area.pressDown();
    area.pressUp();
    area.sendKeyPressSignal(fileId);

    QMouseEvent press(QEvent::MouseButtonPress,
                      QPoint(10, 10),
                      area.mapTo(&area, QPoint(10, 10)),
                      area.viewport()->mapToGlobal(QPoint(10, 10)),
                      Qt::LeftButton,
                      Qt::LeftButton,
                      Qt::NoModifier,
                      Qt::MouseEventSynthesizedByApplication);
    QVERIFY(QApplication::sendEvent(area.viewport(), &press));
    QMouseEvent move(QEvent::MouseMove,
                     QPoint(10, 20),
                     area.mapTo(&area, QPoint(10, 20)),
                     area.viewport()->mapToGlobal(QPoint(10, 20)),
                     Qt::LeftButton,
                     Qt::LeftButton,
                     Qt::NoModifier,
                     Qt::MouseEventSynthesizedByApplication);
    QVERIFY(QApplication::sendEvent(area.viewport(), &move));

    QVERIFY(area.moveWidget(QStringLiteral("Web Page"), 0));
    QVERIFY(!area.moveWidget(QStringLiteral("Missing"), 0));
    QVERIFY(area.removeWidget(QStringLiteral("Web Page")));
    QVERIFY(!area.removeWidget(QStringLiteral("Missing")));
}

void StackPagesWhiteboxTest::resultAreaCoversKeyboardBoundaryBranches()
{
    ResultArea area;
    area.resize(656, 260);
    area.show();
    qApp->processEvents();

    auto *fileWidget = new ResultWidget(QStringLiteral("File"), &area);
    auto *webWidget = new ResultWidget(QStringLiteral("Web Page"), &area);
    area.appendWidet(fileWidget);
    area.appendWidet(webWidget);

    appendBestItem(area, QStringLiteral("File"), makeResultInfo(QStringLiteral("best-one")));
    appendResultItem(fileWidget, makeResultInfo(QStringLiteral("file-one")));
    appendResultItem(webWidget, makeResultInfo(QStringLiteral("web-one"), QStringLiteral("web")));

    area.m_selectedPluginID = area.m_bestListWidget->getWidgetName();
    area.m_isSelected = true;
    area.m_bestListWidget->setResultSelection(area.m_bestListWidget->getModlIndex(0, 0));
    area.pressDown();
    QCOMPARE(area.m_selectedPluginID, QStringLiteral("File"));
    area.pressUp();
    area.pressDown();
    QCOMPARE(fileWidget->getCurrentSelection().row(), 0);

    fileWidget->setResultSelection(fileWidget->getModlIndex(0, 0));
    area.pressUp();
    QCOMPARE(area.m_selectedPluginID, area.m_bestListWidget->getWidgetName());
    QCOMPARE(area.m_bestListWidget->getCurrentSelection().row(), 0);

    QString bestId = area.m_bestListWidget->getWidgetName();
    area.setSelectionInfo(bestId);
    QCOMPARE(area.m_selectedPluginID, bestId);
    QCOMPARE(area.m_isClicked, false);
}

void StackPagesWhiteboxTest::resultAreaCoversAdditionalSelectionAndFloatingBranches()
{
    ResultArea area;
    area.resize(656, 300);
    area.show();
    qApp->processEvents();

    auto *fileWidget = new ResultWidget(QStringLiteral("File"), &area);
    auto *appWidget = new ResultWidget(QStringLiteral("App"), &area);
    area.appendWidet(fileWidget);
    area.appendWidet(appWidget);

    appendBestItem(area, QStringLiteral("File"), makeResultInfo(QStringLiteral("best-a")));
    appendBestItem(area, QStringLiteral("App"), makeResultInfo(QStringLiteral("best-b")));
    appendResultItem(fileWidget, makeResultInfo(QStringLiteral("file-a")));
    appendResultItem(fileWidget, makeResultInfo(QStringLiteral("file-b")));
    appendResultItem(appWidget, makeResultInfo(QStringLiteral("app-a"), QStringLiteral("app")));
    appendResultItem(appWidget, makeResultInfo(QStringLiteral("app-b"), QStringLiteral("app")));

    area.m_isSelected = false;
    area.pressEnter();
    QCOMPARE(area.m_selectedPluginID, area.m_bestListWidget->getWidgetName());
    QVERIFY(area.m_bestListWidget->getCurrentSelection().isValid());

    area.m_bestListWidget->setResultSelection(area.m_bestListWidget->getModlIndex(0, 0));
    area.pressDown();
    QCOMPARE(area.m_bestListWidget->getCurrentSelection().row(), 1);

    area.m_selectedPluginID = area.m_bestListWidget->getWidgetName();
    area.pressUp();
    QCOMPARE(area.m_bestListWidget->getCurrentSelection().row(), 0);

    area.m_bestListWidget->clearResultSelection();
    area.pressUp();
    QCOMPARE(area.m_bestListWidget->getCurrentSelection().row(), 1);

    area.m_selectedPluginID = fileWidget->pluginId();
    fileWidget->setResultSelection(fileWidget->getModlIndex(0, 0));
    area.pressDown();
    QCOMPARE(fileWidget->getCurrentSelection().row(), 1);

    area.pressUp();
    QCOMPARE(fileWidget->getCurrentSelection().row(), 0);

    fileWidget->clearResultSelection();
    area.pressUp();
    QCOMPARE(fileWidget->getCurrentSelection().row(), 1);

    QSignalSpy keySpy(&area, SIGNAL(keyPressChanged(QString,SearchPluginIface::ResultInfo)));
    QVERIFY(keySpy.isValid());
    area.m_detail_open_state = true;
    QString fileId = fileWidget->pluginId();
    fileWidget->clearResultSelection();
    area.sendKeyPressSignal(fileId);
    QCOMPARE(keySpy.count(), 0);

    SearchPluginIface::ResultInfo currentInfo = makeResultInfo(QStringLiteral("current-best"));
    Q_EMIT area.m_bestListWidget->currentRowChanged(QStringLiteral("File"), currentInfo);
    QVERIFY(area.m_detail_open_state);
    QVERIFY(area.m_isSelected);
    QCOMPARE(area.m_selectedPluginID, area.m_bestListWidget->getWidgetName());

    fileWidget->setFixedHeight(560);
    Q_EMIT fileWidget->showMoreClicked();
    QVERIFY(!area.m_titleLabel->isHidden());
    Q_EMIT area.startSearch(QStringLiteral("hide-floating-title"));
    QVERIFY(area.m_titleLabel->isHidden());

    area.verticalScrollBar()->setValue(0);
    fileWidget->setFixedHeight(200);
    Q_EMIT fileWidget->showMoreClicked();
    QCOMPARE(area.verticalScrollBar()->value(), fileWidget->pos().ry());
    QVERIFY(area.m_titleLabel->isHidden());

    ResultArea webOnlyArea;
    webOnlyArea.resize(656, 200);
    webOnlyArea.show();
    qApp->processEvents();

    auto *webWidget = new ResultWidget(QStringLiteral("Web Page"), &webOnlyArea);
    webOnlyArea.appendWidet(webWidget);
    appendResultItem(webWidget, makeResultInfo(QStringLiteral("web-only"), QStringLiteral("web")));

    webOnlyArea.m_selectedPluginID = webWidget->pluginId();
    webOnlyArea.m_isSelected = false;
    webOnlyArea.pressEnter();
    QVERIFY(webOnlyArea.getSelectedState());
    QCOMPARE(webOnlyArea.m_selectedPluginID, webWidget->pluginId());
    QVERIFY(webWidget->getCurrentSelection().isValid());
}

void StackPagesWhiteboxTest::resultAreaCoversEmptyBestLastPluginAndInvalidSelectionBranches()
{
    ResultArea area;
    area.resize(656, 260);
    area.show();
    qApp->processEvents();

    QSignalSpy keySpy(&area, SIGNAL(keyPressChanged(QString,SearchPluginIface::ResultInfo)));
    QSignalSpy scrollShownSpy(area.m_scrollBar, SIGNAL(scrollBarAppeared()));
    QSignalSpy scrollHiddenSpy(area.m_scrollBar, SIGNAL(scrollBarIsHideen()));
    QVERIFY(keySpy.isValid());
    QVERIFY(scrollShownSpy.isValid());
    QVERIFY(scrollHiddenSpy.isValid());

    auto *fileWidget = new ResultWidget(QStringLiteral("File"), &area);
    auto *webWidget = new ResultWidget(QStringLiteral("Web Page"), &area);
    area.appendWidet(fileWidget);
    area.appendWidet(webWidget);
    appendResultItem(webWidget, makeResultInfo(QStringLiteral("fallback-web"), QStringLiteral("web")));

    area.m_selectedPluginID = webWidget->pluginId();
    area.pressEnter();
    QVERIFY(area.getSelectedState());
    QCOMPARE(area.m_selectedPluginID, webWidget->pluginId());
    QVERIFY(webWidget->getCurrentSelection().isValid());

    area.pressDown();
    QCOMPARE(area.m_selectedPluginID, webWidget->pluginId());

    area.m_isSelected = false;
    area.pressUp();
    QCOMPARE(area.m_selectedPluginID, webWidget->pluginId());

    area.m_isSelected = true;
    area.m_detail_open_state = true;
    webWidget->clearResultSelection();
    QString webId = webWidget->pluginId();
    keySpy.clear();
    area.sendKeyPressSignal(webId);
    QCOMPARE(keySpy.count(), 0);

    QShowEvent showEvent;
    QHideEvent hideEvent;
    area.m_scrollBar->showEvent(&showEvent);
    area.m_scrollBar->hideEvent(&hideEvent);
    QVERIFY(scrollShownSpy.count() >= 1);
    QVERIFY(scrollHiddenSpy.count() >= 1);

    Q_EMIT area.startSearch(QStringLiteral("reset"));
    QVERIFY(!area.m_detail_open_state);
    QVERIFY(area.m_isSelected);
    QCOMPARE(area.m_selectedPluginID, webWidget->pluginId());
    QVERIFY(!area.m_isClicked);

    webWidget->clearResult();
    Q_EMIT area.startSearch(QStringLiteral("empty-reset"));
    QVERIFY(!area.m_isSelected);
    QVERIFY(area.m_selectedPluginID.isEmpty());
    Q_EMIT area.stopSearch();
}

void StackPagesWhiteboxTest::resultAreaCoversFloatingTitleScrollbarAndDetailBranches()
{
    ResultArea area;
    area.resize(240, 180);
    area.show();
    qApp->processEvents();

    auto *fileWidget = new ResultWidget(QStringLiteral("File"), &area);
    area.appendWidet(fileWidget);
    for (int i = 0; i < 8; ++i) {
        appendResultItem(fileWidget, makeResultInfo(QStringLiteral("many-%1").arg(i)));
    }
    fileWidget->expandListSlot();
    fileWidget->onListLengthChanged(fileWidget->getResultNum());
    fileWidget->setFixedHeight(560);

    Q_EMIT fileWidget->showMoreClicked();
    QVERIFY(!area.m_titleLabel->isHidden());
    Q_EMIT area.m_titleLabel->retractClicked();
    QVERIFY(area.m_titleLabel->isHidden());

    Q_EMIT fileWidget->retractClicked();
    QVERIFY(area.m_titleLabel->isHidden());

    QVERIFY(area.getVScrollBarWidth() >= 0);

    DetailArea detailArea;
    SearchPluginIface::ResultInfo info = makeResultInfo(QStringLiteral("detail-paint"));
    Q_EMIT detailArea.setWidgetInfo(QStringLiteral("File"), info);
    QVERIFY(detailArea.isVisible());
    QVERIFY(detailArea.m_detailWidget->m_detailPage != nullptr);

    QPixmap detailPixmap(detailArea.m_detailWidget->size());
    detailPixmap.fill(Qt::transparent);
    detailArea.m_detailWidget->render(&detailPixmap);

    QWidget container;
    QVBoxLayout layout(&container);
    layout.addWidget(new QLabel(QStringLiteral("child"), &container));
    detailArea.m_detailWidget->clearLayout(&layout);
    QCOMPARE(layout.count(), 0);
    detailArea.m_detailWidget->clearLayout(nullptr);
}

void StackPagesWhiteboxTest::resultAreaCoversMoreBoundaryMouseAndScrollBranches()
{
    ResultArea area;
    area.resize(260, 180);
    area.show();
    qApp->processEvents();

    auto *firstWidget = new ResultWidget(QStringLiteral("File"), &area);
    auto *secondWidget = new ResultWidget(QStringLiteral("App"), &area);
    auto *webWidget = new ResultWidget(QStringLiteral("Web Page"), &area);
    area.appendWidet(firstWidget);
    area.appendWidet(secondWidget);
    area.appendWidet(webWidget);

    appendBestItem(area, QStringLiteral("File"), makeResultInfo(QStringLiteral("best-a")));
    appendBestItem(area, QStringLiteral("App"), makeResultInfo(QStringLiteral("best-b")));
    for (int i = 0; i < 8; ++i) {
        appendResultItem(firstWidget, makeResultInfo(QStringLiteral("file-%1").arg(i)));
        appendResultItem(secondWidget, makeResultInfo(QStringLiteral("app-%1").arg(i), QStringLiteral("app")));
    }
    appendResultItem(webWidget, makeResultInfo(QStringLiteral("web-last"), QStringLiteral("web")));

    firstWidget->expandListSlot();
    secondWidget->expandListSlot();
    firstWidget->setFixedHeight(560);
    secondWidget->setFixedHeight(560);
    area.onWidgetSizeChanged();

    Q_EMIT firstWidget->showMoreClicked();
    QVERIFY(!area.m_titleLabel->isHidden());
    area.verticalScrollBar()->setValue(firstWidget->pos().ry() + 31);
    area.verticalScrollBar()->setValue(secondWidget->pos().ry() + 31);
    area.verticalScrollBar()->setValue(0);

    Q_EMIT area.m_titleLabel->retractClicked();
    QVERIFY(area.m_titleLabel->isHidden());

    area.m_isSelected = true;
    area.m_selectedPluginID = firstWidget->pluginId();
    firstWidget->setResultSelection(firstWidget->getModlIndex(7, 0));
    area.pressDown();
    QCOMPARE(area.m_selectedPluginID, secondWidget->pluginId());

    secondWidget->setResultSelection(secondWidget->getModlIndex(0, 0));
    area.pressUp();
    QCOMPARE(area.m_selectedPluginID, firstWidget->pluginId());

    firstWidget->setResultSelection(firstWidget->getModlIndex(-1, 0));
    area.pressUp();
    QVERIFY(firstWidget->getCurrentSelection().isValid());

    QMouseEvent normalPress(QEvent::MouseButtonPress,
                            QPoint(5, 5),
                            area.viewport()->mapToGlobal(QPoint(5, 5)),
                            Qt::LeftButton,
                            Qt::LeftButton,
                            Qt::NoModifier);
    area.mousePressEvent(&normalPress);
    QMouseEvent normalMove(QEvent::MouseMove,
                           QPoint(5, 12),
                           area.viewport()->mapToGlobal(QPoint(5, 12)),
                           Qt::LeftButton,
                           Qt::LeftButton,
                           Qt::NoModifier);
    area.mouseMoveEvent(&normalMove);
    QMouseEvent normalRelease(QEvent::MouseButtonRelease,
                              QPoint(5, 12),
                              area.viewport()->mapToGlobal(QPoint(5, 12)),
                              Qt::LeftButton,
                              Qt::LeftButton,
                              Qt::NoModifier);
    area.mouseReleaseEvent(&normalRelease);

    QEvent resizeEvent(QEvent::Resize);
    area.viewportEvent(&resizeEvent);
}

void StackPagesWhiteboxTest::resultAreaCoversBestListWithinBoundsAndNonBestActivationBranches()
{
    ResultArea area;
    area.resize(656, 300);
    area.show();
    qApp->processEvents();

    auto *fileWidget = new ResultWidget(QStringLiteral("File"), &area);
    auto *appWidget = new ResultWidget(QStringLiteral("App"), &area);
    area.appendWidet(fileWidget);
    area.appendWidet(appWidget);

    appendBestItem(area, QStringLiteral("File"), makeResultInfo(QStringLiteral("best-a")));
    appendBestItem(area, QStringLiteral("File"), makeResultInfo(QStringLiteral("best-b")));
    appendResultItem(fileWidget, makeResultInfo(QStringLiteral("file-one")));

    // Test: pressEnter when NOT selected, best-list has results → selects best-list first item (L144-148)
    area.pressEnter();
    QVERIFY(area.getSelectedState());
    QCOMPARE(area.m_selectedPluginID, area.m_bestListWidget->getWidgetName());

    // Test: pressDown at best-list boundary → clears best-list, jumps to next widget (L184-194)
    // When currentIndex is invalid (-1), index.row() < 0, enters the jump-to-next path
    area.m_bestListWidget->m_bestListView->setExpanded(true);
    area.m_bestListWidget->clearResultSelection();
    area.m_selectedPluginID = area.m_bestListWidget->getWidgetName();
    area.pressDown();
    QCOMPARE(area.m_selectedPluginID, fileWidget->pluginId());

    // Test: pressEnter with detail_open + non-best-match widget activation (L156-159)
    fileWidget->setResultSelection(fileWidget->getModlIndex(0, 0));
    area.m_selectedPluginID = fileWidget->pluginId();
    area.m_isSelected = true;
    area.m_detail_open_state = true;
    int callsBefore = pluginFor(QStringLiteral("File"))->m_openActionCalls;
    area.pressEnter();
    QVERIFY(pluginFor(QStringLiteral("File"))->m_openActionCalls > callsBefore);
}

void StackPagesWhiteboxTest::resultAreaCoversBestListSignalsAndWidgetRetractBranches()
{
    ResultArea area;
    area.resize(260, 180);
    area.show();
    qApp->processEvents();

    auto *fileWidget = new ResultWidget(QStringLiteral("File"), &area);
    area.appendWidet(fileWidget);
    appendBestItem(area, QStringLiteral("File"), makeResultInfo(QStringLiteral("best-signal")));
    appendResultItem(fileWidget, makeResultInfo(QStringLiteral("file-signal")));

    QSignalSpy rowClickedSpy(&area, SIGNAL(rowClicked()));
    QVERIFY(rowClickedSpy.isValid());
    Q_EMIT area.m_bestListWidget->rowClicked();
    QCOMPARE(rowClickedSpy.count(), 1);

    Q_EMIT area.resizeWidth(320);
    QCOMPARE(area.m_bestListWidget->width(), 320);
    QCOMPARE(area.m_titleLabel->width(), 320);

    area.m_isClicked = false;
    Q_EMIT area.m_bestListWidget->sizeChanged();
    QCOMPARE(area.m_selectedPluginID, area.m_bestListWidget->getWidgetName());
    QVERIFY(area.m_isSelected);
    QVERIFY(area.m_bestListWidget->getCurrentSelection().isValid());

    area.m_isClicked = true;
    area.m_selectedPluginID = QStringLiteral("manual");
    Q_EMIT area.m_bestListWidget->sizeChanged();
    QCOMPARE(area.m_selectedPluginID, QStringLiteral("manual"));

    fileWidget->setFixedHeight(560);
    Q_EMIT fileWidget->showMoreClicked();
    QVERIFY(!area.m_titleLabel->isHidden());
    Q_EMIT fileWidget->retractClicked();
    QVERIFY(area.m_titleLabel->isHidden());
}

void StackPagesWhiteboxTest::searchResultPageCoversPluginOrderingResizeAndDetailBranches()
{
    SearchResultPage page;
    page.resize(700, 552);
    page.show();
    qApp->processEvents();

    QSignalSpy resizeWidthSpy(&page, SIGNAL(resizeWidth(int)));
    QSignalSpy currentRowSpy(&page, SIGNAL(currentRowChanged(QString,SearchPluginIface::ResultInfo)));
    QVERIFY(resizeWidthSpy.isValid());
    QVERIFY(currentRowSpy.isValid());
    QVERIFY(page.m_resultArea != nullptr);
    QVERIFY(page.m_detailArea != nullptr);
    QCOMPARE(page.m_resultArea->m_widgetList.size(), 3);

    page.setWidth(672);
    QVERIFY(!resizeWidthSpy.isEmpty());
    page.sendResizeWidthSignal(300);
    QCOMPARE(resizeWidthSpy.last().at(0).toInt(), 300);

    page.appendPlugin(QStringLiteral("Extra"));
    QVERIFY(page.m_resultArea->m_widgetList.size() >= 4);
    page.movePlugin(QStringLiteral("Extra"), 1);
    page.appendPlugin(QStringLiteral("MissingOrder"));
    QVERIFY(page.m_resultArea->m_widgetList.size() >= 5);

    ResultWidget *firstWidget = page.m_resultArea->m_widgetList.first();
    appendResultItem(firstWidget, makeResultInfo(QStringLiteral("page-file")));
    QModelIndex first = firstWidget->getModlIndex(0, 0);
    QVERIFY(first.isValid());
    firstWidget->m_resultView->onRowSelectedSlot(first);
    QVERIFY(!currentRowSpy.isEmpty());
    QCOMPARE(page.getSelectedState(), true);

    SearchPluginIface::ResultInfo info = makeResultInfo(QStringLiteral("detail"));
    page.m_detailArea->m_detailWidget->updateDetailPage(firstWidget->pluginId(), info);
    page.m_detailArea->m_detailWidget->updateDetailPage(firstWidget->pluginId(), info);
    page.m_detailArea->m_detailWidget->updateDetailPage(QStringLiteral("App"), info);
    QVERIFY(pluginFor(firstWidget->pluginId())->m_detailPageCalls >= 2);
    QVERIFY(pluginFor(QStringLiteral("App"))->m_detailPageCalls >= 1);

    page.m_detailArea->hide();
    Q_EMIT page.m_resultArea->scrollBarAppeared();
    Q_EMIT page.m_resultArea->scrollBarIsHideen();
    page.m_detailArea->show();
    Q_EMIT page.m_resultArea->scrollBarAppeared();
    Q_EMIT page.m_resultArea->scrollBarIsHideen();
    QVERIFY(resizeWidthSpy.count() >= 5);

    resizeWidthSpy.clear();
    QString pluginId = firstWidget->pluginId();
    page.m_resultArea->sendKeyPressSignal(pluginId);
    QVERIFY(!resizeWidthSpy.isEmpty());
    Q_EMIT firstWidget->rowClicked();
    QVERIFY(resizeWidthSpy.count() >= 2);

    Q_EMIT page.startSearch(QStringLiteral("keyword"));
    page.pressEnter();
    page.pressDown();
    page.pressUp();
    Q_EMIT page.stopSearch();

    Q_EMIT GlobalSettings::getInstance().valueChanged(WINDOW_RADIUS_KEY, 10);
    QCOMPARE(page.m_radius, 10);
    Q_EMIT GlobalSettings::getInstance().valueChanged(TRANSPARENCY_KEY, 0.75);

    QPixmap pixmap(page.size());
    pixmap.fill(Qt::transparent);
    page.render(&pixmap);
}

void StackPagesWhiteboxTest::searchResultPageCoversOrderedAppendRowClickAndUtilityBranches()
{
    QCOMPARE(escapeHtml(QStringLiteral("<b>safe</b>")), QStringLiteral("&lt;b&gt;safe&lt;/b&gt;"));

    qApp->setProperty("kradius-max", 7);
    DetailWidget detailWidget;
    qApp->setProperty("kradius-max", 15);
    Q_EMIT GlobalSettings::getInstance().valueChanged(WIDGET_THEME_NAME_KEY, QStringLiteral("theme"));
    QCOMPARE(detailWidget.m_radius, 15);

    SearchResultPage page;
    page.resize(700, 552);
    page.show();
    qApp->processEvents();

    QSignalSpy resizeWidthSpy(&page, SIGNAL(resizeWidth(int)));
    QVERIFY(resizeWidthSpy.isValid());

    const int before = page.m_resultArea->m_widgetList.size();
    page.appendPlugin(QStringLiteral("App"));
    QCOMPARE(page.m_resultArea->m_widgetList.size(), before + 1);
    QCOMPARE(page.m_resultArea->m_widgetList.at(2)->pluginId(), QStringLiteral("App"));

    ResultWidget *insertedWidget = page.m_resultArea->m_widgetList.at(2);
    page.m_detailArea->hide();
    resizeWidthSpy.clear();
    Q_EMIT insertedWidget->rowClicked();
    QVERIFY(!resizeWidthSpy.isEmpty());

    page.m_detailArea->show();
    resizeWidthSpy.clear();
    Q_EMIT page.m_resultArea->scrollBarAppeared();
    Q_EMIT page.m_resultArea->scrollBarIsHideen();
    QVERIFY(resizeWidthSpy.count() >= 2);
}

QTEST_MAIN(StackPagesWhiteboxTest)

#include "stack-pages-whitebox-test.moc"
