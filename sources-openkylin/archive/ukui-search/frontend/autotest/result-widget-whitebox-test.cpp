#include <QApplication>
#include <QSignalSpy>
#include <QStyleFactory>
#include <QTest>

#include "global-settings.h"
#include "icon-loader.h"
#include "show-more-label.h"

#define private public
#define protected public
#include "result-view.h"
#undef protected
#undef private

using namespace UkuiSearch;

namespace UkuiSearch {

namespace {
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
        ++m_openActionCalls;
        m_lastActionKey = key;
    }
    QWidget *detailPage(const ResultInfo &ri) override
    {
        Q_UNUSED(ri)
        ++m_detailPageCalls;
        return new QWidget;
    }

    int m_openActionCalls = 0;
    int m_detailPageCalls = 0;
    QString m_lastActionKey;

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
    return {PluginInfo(QStringLiteral("File"), true, 1, false, false)};
}

SearchPluginIface *SearchPluginManager::getPlugin(const QString &pluginId)
{
    if (pluginId == QStringLiteral("Missing")) {
        return nullptr;
    }
    static DummySearchPlugin plugin;
    return &plugin;
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
        return 0;
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
SearchPluginIface::ResultInfo makeResultInfo(const QString &name)
{
    return SearchPluginIface::ResultInfo(
        QIcon(),
        name,
        QStringLiteral("tooltip-") + name,
        QStringLiteral("file"),
        {},
        QStringLiteral("action-") + name,
        1,
        true,
        false,
        0,
        QStringLiteral("icon-") + name);
}
}

class ResultWidgetWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void coversWidgetFacadeAndLayoutBranches();
    void coversResultViewSignalsAndPluginBranches();
    void coversShowMorePaletteAndMissingPluginBranches();
};

void ResultWidgetWhiteboxTest::initTestCase()
{
    qRegisterMetaType<SearchPluginIface::ResultInfo>("SearchPluginIface::ResultInfo");
    if (auto *style = QStyleFactory::create(QStringLiteral("Fusion"))) {
        qApp->setStyle(style);
    }
}

void ResultWidgetWhiteboxTest::coversWidgetFacadeAndLayoutBranches()
{
    ResultWidget widget(QStringLiteral("File"));
    widget.resize(500, 220);
    widget.show();
    qApp->processEvents();

    QCOMPARE(widget.pluginId(), QStringLiteral("File"));
    QCOMPARE(widget.pluginName(), QStringLiteral("Dummy File"));
    widget.setEnabled(false);
    QCOMPARE(widget.m_enabled, false);

    QSignalSpy sizeSpy(&widget, SIGNAL(sizeChanged()));
    QSignalSpy currentRowSpy(&widget, SIGNAL(currentRowChanged(QString,SearchPluginIface::ResultInfo)));
    QSignalSpy showMoreSpy(&widget, SIGNAL(showMoreClicked()));
    QSignalSpy retractSpy(&widget, SIGNAL(retractClicked()));
    QVERIFY(sizeSpy.isValid());
    QVERIFY(currentRowSpy.isValid());
    QVERIFY(showMoreSpy.isValid());
    QVERIFY(retractSpy.isValid());

    widget.m_resultView->m_model->appendInfo(makeResultInfo(QStringLiteral("alpha")));
    widget.m_resultView->onItemListChanged(1);
    widget.onListLengthChanged(1);
    QCOMPARE(widget.getResultNum(), 1);
    QVERIFY(widget.isVisible());
    QVERIFY(widget.height() > 0);
    QVERIFY(!sizeSpy.isEmpty());

    QModelIndex first = widget.getModlIndex(0, 0);
    QVERIFY(first.isValid());
    QCOMPARE(widget.getIndexResultInfo(first).name, QStringLiteral("alpha"));
    widget.setResultSelection(first);
    QCOMPARE(widget.getCurrentSelection(), first);
    widget.m_resultView->onRowSelectedSlot(first);
    QVERIFY(!currentRowSpy.isEmpty());

    widget.clearResultSelection();
    QVERIFY(!widget.getCurrentSelection().isValid());
    Q_EMIT widget.clearSelectedRow();
    QVERIFY(!widget.getCurrentSelection().isValid());

    widget.expandListSlot();
    QVERIFY(widget.getExpandState());
    widget.reduceListSlot();
    QVERIFY(!widget.getExpandState());

    widget.resetTitleLabel();
    widget.setTitileLableHide(false);
    QVERIFY(!widget.m_titleLabel->isVisible());
    widget.setTitileLableHide(true);
    QVERIFY(widget.m_titleLabel->isVisible());

    Q_EMIT widget.resizeWidth(480);
    QCOMPARE(widget.width(), 480);

    widget.clearResult();
    QVERIFY(!widget.isVisible());
    QCOMPARE(widget.height(), 0);
    widget.onListLengthChanged(0);
    QCOMPARE(widget.height(), 0);
}

void ResultWidgetWhiteboxTest::coversResultViewSignalsAndPluginBranches()
{
    auto *plugin = static_cast<DummySearchPlugin *>(SearchPluginManager::getInstance()->getPlugin(QStringLiteral("File")));
    plugin->m_openActionCalls = 0;
    plugin->m_lastActionKey.clear();

    ResultWidget widget(QStringLiteral("File"));
    widget.show();
    qApp->processEvents();

    QSignalSpy labelResetSpy(widget.m_resultView, SIGNAL(lableReset()));
    QSignalSpy bestListSpy(&widget, SIGNAL(sendBestListData(QString,SearchPluginIface::ResultInfo)));
    QSignalSpy rowClickedSpy(&widget, SIGNAL(rowClicked()));
    QVERIFY(labelResetSpy.isValid());
    QVERIFY(bestListSpy.isValid());
    QVERIFY(rowClickedSpy.isValid());

    Q_EMIT widget.startSearch(QStringLiteral("needle"));
    QCOMPARE(labelResetSpy.count(), 1);
    QVERIFY(!widget.m_resultView->isExpanded());
    Q_EMIT widget.stopSearch();

    auto openInfo = makeResultInfo(QStringLiteral("open-me"));
    openInfo.searchID = widget.m_resultView->m_model->m_searchID;
    widget.m_resultView->m_model->appendInfo(openInfo);
    widget.m_resultView->onItemListChanged(1);
    QModelIndex first = widget.getModlIndex(0, 0);
    QVERIFY(first.isValid());
    widget.setResultSelection(first);

    widget.activateIndex();
    QCOMPARE(plugin->m_openActionCalls, 1);
    QCOMPARE(plugin->m_lastActionKey, QStringLiteral("action-open-me"));

    Q_EMIT widget.m_resultView->sendBestListData(QStringLiteral("File"), makeResultInfo(QStringLiteral("best-forward")));
    QVERIFY(bestListSpy.count() >= 1);

    Q_EMIT widget.m_resultView->clicked(first);
    QVERIFY(rowClickedSpy.count() >= 1);

    QAction action(QStringLiteral("unused"), &widget);
    widget.m_resultView->onMenuTriggered(&action);
    QVERIFY(plugin->m_openActionCalls >= 1);
}

void ResultWidgetWhiteboxTest::coversShowMorePaletteAndMissingPluginBranches()
{
    ResultWidget widget(QStringLiteral("File"));
    widget.resize(500, 260);
    widget.show();
    qApp->processEvents();

    QSignalSpy sizeSpy(&widget, SIGNAL(sizeChanged()));
    QSignalSpy showMoreSpy(&widget, SIGNAL(showMoreClicked()));
    QSignalSpy retractSpy(&widget, SIGNAL(retractClicked()));
    QVERIFY(sizeSpy.isValid());
    QVERIFY(showMoreSpy.isValid());
    QVERIFY(retractSpy.isValid());

    for (int i = 0; i < 6; ++i) {
        auto info = makeResultInfo(QStringLiteral("row-%1").arg(i));
        info.searchID = widget.m_resultView->m_model->m_searchID;
        widget.m_resultView->m_model->appendInfo(info);
    }
    widget.m_resultView->onItemListChanged(6);
    widget.onListLengthChanged(6);

    auto *showMore = widget.m_titleLabel->findChild<ShowMoreLabel *>();
    QVERIFY(showMore != nullptr);
    auto *icon = showMore->findChild<QLabel *>();
    QVERIFY(icon != nullptr);
    QVERIFY(showMore->isVisible());

    QTest::mouseClick(icon, Qt::LeftButton);
    QCOMPARE(showMoreSpy.count(), 1);
    QVERIFY(widget.getExpandState());

    QTest::mouseClick(icon, Qt::LeftButton);
    QCOMPARE(retractSpy.count(), 1);
    QVERIFY(!widget.getExpandState());

    const int sizeSignalsBefore = sizeSpy.count();
    Q_EMIT qApp->paletteChanged(qApp->palette());
    QVERIFY(sizeSpy.count() > sizeSignalsBefore);

    ResultView missingView(QStringLiteral("Missing"));
    QAction action(QStringLiteral("missing"), &missingView);
    missingView.onMenuTriggered(&action);
}

QTEST_MAIN(ResultWidgetWhiteboxTest)

#include "result-widget-whitebox-test.moc"
