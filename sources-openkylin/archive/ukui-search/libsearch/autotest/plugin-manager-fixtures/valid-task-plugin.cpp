#include <QtPlugin>
#include <QIcon>

#include "plugininterface/search-task-plugin-iface.h"

using namespace UkuiSearch;

class ValidTaskPlugin : public SearchTaskPluginIface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SearchTaskPluginIface_iid FILE "valid-task-plugin.json")
    Q_INTERFACES(UkuiSearch::SearchTaskPluginIface)

public:
    void setController(const SearchController &searchController) override { Q_UNUSED(searchController) }
    PluginType pluginType() override { return PluginType::SearchTaskPlugin; }
    const QString name() override { return QStringLiteral("Fixture Task Plugin"); }
    const QString description() override { return QStringLiteral("Fixture Task Plugin"); }
    const QIcon icon() override { return QIcon(); }
    void setEnable(bool enable) override { m_enable = enable; }
    bool isEnable() override { return m_enable; }
    QString getCustomSearchType() override { return QStringLiteral("fixture-task-plugin"); }
    SearchProperty::SearchType getSearchType() override { return SearchProperty::SearchType::Custom; }
    void startSearch() override {}
    void stop() override {}
    bool isSearching() override { return false; }

private:
    bool m_enable = true;
};

#include "valid-task-plugin.moc"
