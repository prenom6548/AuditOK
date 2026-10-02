#include <QSignalSpy>
#include <QTest>

#include "plugininterface/action-transmiter.h"
#include "plugininterface/search-plugin-iface.h"

using namespace UkuiSearch;

class StubSearchPlugin : public SearchPluginIface
{
public:
    PluginType pluginType() override { return PluginType::SearchPlugin; }
    const QString name() override { return QStringLiteral("stub"); }
    const QString description() override { return QStringLiteral("stub"); }
    const QIcon icon() override { return QIcon(); }
    QString getPluginName() override { return QStringLiteral("stub"); }
    void setEnable(bool enable) override { m_enable = enable; }
    bool isEnable() override { return m_enable; }
    size_t KeywordSearch(QString keyword, DataQueue<ResultInfo> *searchResult) override
    {
        Q_UNUSED(keyword)
        Q_UNUSED(searchResult)
        return 0;
    }
    QWidget *detailPage(const ResultInfo &ri) override
    {
        Q_UNUSED(ri)
        return nullptr;
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
        Q_UNUSED(key)
        Q_UNUSED(type)
    }

private:
    bool m_enable = true;
};

class SearchPluginIfaceWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void resultInfoStoresConstructorValues();
    void invokeActionsForwardsHideUiToTransmiter();
    void invokeActionsIgnoresNonHideUiActions();
};

void SearchPluginIfaceWhiteboxTest::resultInfoStoresConstructorValues()
{
    const QVector<SearchPluginIface::DescriptionInfo> descriptions {
        {QStringLiteral("Path"), QStringLiteral("/tmp/demo.txt")},
        {QStringLiteral("Modified"), QStringLiteral("2026-05-06")}
    };

    SearchPluginIface::ResultInfo result(QIcon(),
                                         QStringLiteral("Demo"),
                                         QStringLiteral("Demo tooltip"),
                                         QStringLiteral("file"),
                                         descriptions,
                                         QStringLiteral("/tmp/demo.txt"),
                                         7,
                                         false,
                                         true,
                                         42,
                                         QStringLiteral("text-x-generic"));

    QCOMPARE(result.name, QStringLiteral("Demo"));
    QCOMPARE(result.toolTip, QStringLiteral("Demo tooltip"));
    QCOMPARE(result.resourceType, QStringLiteral("file"));
    QCOMPARE(result.description.size(), 2);
    QCOMPARE(result.description.at(0).key, QStringLiteral("Path"));
    QCOMPARE(result.description.at(0).value, QStringLiteral("/tmp/demo.txt"));
    QCOMPARE(result.actionKey, QStringLiteral("/tmp/demo.txt"));
    QCOMPARE(result.type, 7);
    QVERIFY(!result.showInBestMatch);
    QVERIFY(result.displayNameAsUrl);
    QCOMPARE(result.searchID, size_t(42));
    QCOMPARE(result.iconName, QStringLiteral("text-x-generic"));
}

void SearchPluginIfaceWhiteboxTest::invokeActionsForwardsHideUiToTransmiter()
{
    StubSearchPlugin plugin;
    QSignalSpy spy(ActionTransmiter::getInstance(), &ActionTransmiter::hideUIAction);

    plugin.invokeActions(SearchPluginIface::InvokableAction::HideUI);

    QCOMPARE(spy.count(), 1);
}

void SearchPluginIfaceWhiteboxTest::invokeActionsIgnoresNonHideUiActions()
{
    StubSearchPlugin plugin;
    QSignalSpy spy(ActionTransmiter::getInstance(), &ActionTransmiter::hideUIAction);

    plugin.invokeActions(SearchPluginIface::InvokableAction::None);
    plugin.invokeActions(SearchPluginIface::InvokableActions(
        QFlag(int(SearchPluginIface::InvokableAction::None) |
              int(SearchPluginIface::InvokableAction::HideUI))));

    QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(SearchPluginIfaceWhiteboxTest)

#include "search-plugin-iface-whitebox-test.moc"
