#ifndef __KOM_BURIEDPOINT_H__
#define __KOM_BURIEDPOINT_H__

#include <QString>
#include <QMap>

namespace kom {

enum BuriedPointPage {
    MainApplication = 0,
    PluginsServiceSupport,
    PluginsToolBox,
    PluginsWinDataMigration,
    PluginsRemoteAssistanceTool,

    ServiceMonitor
};

class BuriedPoint {
public:
    BuriedPoint() = default;
    ~BuriedPoint() = default;

    static void uploadMessage(BuriedPointPage buried_point_page, const QString &event_code);
    static void uploadMessage(BuriedPointPage buried_point_page, const QString &event_code, const QString &data);
    static void uploadMessage(BuriedPointPage buried_point_page, const QString &event_code, const QMap<QString, QString> &custom_property);

private:
    static QString BuriedPointPageToString(BuriedPointPage buried_point_page);
};

}

#endif
