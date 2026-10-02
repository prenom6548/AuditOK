#include <QObject>
#include <QtPlugin>

#include "plugininterface/search-plugin-iface.h"

using namespace UkuiSearch;

class WrongVersionSearchPlugin : public QObject, public SearchPluginIface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SearchPluginIface_iid FILE "wrong-version-search-plugin.json")
    Q_INTERFACES(UkuiSearch::SearchPluginIface)

public:
    PluginType pluginType() override { return PluginType::SearchPlugin; }
    const QString name() override { return QStringLiteral("Wrong Version Search Plugin"); }
    const QString description() override { return QStringLiteral("Wrong Version Search Plugin"); }
    const QIcon icon() override { return QIcon(); }
    void setEnable(bool enable) override { m_enable = enable; }
    bool isEnable() override { return m_enable; }
    QString getPluginName() override { return name(); }
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
        Q_UNUSED(key)
        Q_UNUSED(type)
    }
    QWidget *detailPage(const ResultInfo &ri) override
    {
        Q_UNUSED(ri)
        return nullptr;
    }

private:
    bool m_enable = true;
};

#include "wrong-version-search-plugin.moc"
