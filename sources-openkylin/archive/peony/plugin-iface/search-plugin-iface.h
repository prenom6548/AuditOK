#ifndef SEARCHPLUGINIFACE_H
#define SEARCHPLUGINIFACE_H

#include <plugin-iface.h>
#include <peony-core_global.h>

#define SearchPluginInterface_iid "org.ukui.peony-qt.plugin-iface.SearchPluginInterface"

namespace Peony {
class PEONYCORESHARED_EXPORT SearchPluginIface : public PluginInterface {

public:
    virtual ~SearchPluginIface() {}

    virtual bool isSupportUri(const QString &uri) = 0;

    virtual int priority() = 0;

    virtual void initSearch(const QString &searchUri = "", const QStringList &searchDir = {""}, const QStringList &searchKey = {""}) = 0;

    virtual void startSearch() = 0;

    virtual void stopSearch() = 0;

    virtual QQueue<QString>& getSearchResult() = 0;

    virtual void resultDequeue() = 0;
};
}

Q_DECLARE_INTERFACE(Peony::SearchPluginIface, SearchPluginInterface_iid)

#endif // SEARCHPLUGINIFACE_H
