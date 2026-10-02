#include <libkydatacollect.h>
#include "kom-buriedpoint.h"

namespace kom {

QString BuriedPoint::BuriedPointPageToString(BuriedPointPage buried_point_page) {
    switch (buried_point_page) {
    case BuriedPointPage::MainApplication:
        return QString("MainApplication");
    case BuriedPointPage::PluginsServiceSupport:
        return QString("PluginsServiceSupport");
    case BuriedPointPage::PluginsToolBox:
        return QString("PluginsToolBox");
    case BuriedPointPage::PluginsWinDataMigration:
        return QString("PluginsWinDataMigration");
    case BuriedPointPage::PluginsRemoteAssistanceTool:
        return QString("PluginsRemoteAssistanceTool");
    case BuriedPointPage::PluginsDriverManager:
        return QString("PluginsDriverManager");
    case BuriedPointPage::ServiceMonitor:
        return QString("ServiceMonitor");
    default:
        return QString("UnknowType");
    }
    return QString("UnknowType");
}

void BuriedPoint::uploadMessage(BuriedPointPage buried_point_page, const QString &event_code, const QMap<QString, QString> &custom_property) {
    KTrackData *track_data = kdk_dia_data_init(KEventSourceType::KEVENTSOURCE_DESKTOP, KEventType::KEVENT_CLICK);

    int cursor{0};
    int property_size{custom_property.size()};
    KCustomProperty property[property_size];
    for (auto iter = custom_property.begin(); iter != custom_property.end(); iter++) {
        property[cursor].key = strdup(iter.key().toLocal8Bit().data());
        property[cursor].value = strdup(iter.value().toLocal8Bit().data());
        cursor++;
    }

    kdk_dia_append_custom_property(track_data, property, property_size);
    kdk_dia_upload_default(track_data,
                           event_code.toLocal8Bit().data(),
                           BuriedPointPageToString(buried_point_page).toLocal8Bit().data());

    for (int i = 0; i < property_size; i++) {
        free(property[i].key);
        free(property[i].value);
    }
    kdk_dia_data_free(track_data);
}

void BuriedPoint::uploadMessage(BuriedPointPage buried_point_page, const QString &event_code, const QString &data) {
    QMap<QString, QString> custom_property;
    custom_property.insert("ExtraData", data);
    uploadMessage(buried_point_page, event_code, custom_property);
}

void BuriedPoint::uploadMessage(BuriedPointPage buried_point_page, const QString &event_code) {
    uploadMessage(buried_point_page, event_code, QMap<QString, QString>());
}

}
