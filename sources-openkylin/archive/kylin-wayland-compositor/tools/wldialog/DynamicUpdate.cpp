// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QJsonArray>
#include <QJsonObject>

#include "DynamicUpdate.h"
#include "IconUtils.h"

DynamicDataUpdate::DynamicDataUpdate(QObject *parent) : QObject(parent) {}

void DynamicDataUpdate::updateList(const QJsonObject &json)
{
    QVariantMap list;
    QString mode = "list";
    if (json.contains("mode")) {
        mode = json["mode"].toString("list");
    }
    if (mode.isEmpty() || mode != "icon") {
        list["type"] = "ListView";
    } else {
        list["type"] = "GridView";
    }

    QVariantMap dataItem;
    QString name;
    if (json.contains("name")) {
        name = json["name"].toString();
        dataItem["name"] = name;
    }

    int textRow = 1;
    bool mediaWidget = false;
    QVariantList listItems = {};

    if (json.contains("data") && json["data"].isArray()) {
        QJsonArray array = json["data"].toArray();
        for (const auto &val : array) {
            QJsonObject obj = val.toObject();
            int row = 2;
            QVariantMap item;
            item["key"] = obj["key"].toString();
            item["title"] = obj["title"].toString();
            if (obj.contains("sub_title")) {
                item["subTitle"] = obj["sub_title"].toString();
                if (textRow < row) {
                    textRow = row;
                }
            }

            if (obj.contains("icon")) {
                QString i_name = obj["icon"].toString();
                if (i_name == "fallback" && !obj["title"].toString().isEmpty()) {
                    i_name = obj["title"].toString();
                }
                item["icon"] = IconUtils::instance().getIconUrl(i_name);
            }

            if (obj.contains("text")) {
                item["text"] = obj["text"].toString();
                row++;
                if (textRow < row) {
                    textRow = row;
                }
            }

            if (obj.contains("selected")) {
                item["selected"] = obj["selected"].toBool();
            }

            if (obj.contains("media_widget")) {
                mediaWidget = true;
            }

            if (obj.contains("remove")) {
                item["remove"] = obj["remove"].toBool();
            }

            listItems.append(item);
        }
    }
    list["child"] = listItems;
    dataItem["textRow"] = textRow;
    dataItem["mediaWidget"] = mediaWidget;
    list["data"] = dataItem;

    setList(list);
}

void DynamicDataUpdate::updateNode(const QJsonObject &json)
{
    if (json.contains("list")) {
        QJsonObject listObj = json["list"].toObject();
        updateList(listObj);
    }
}

QVariantMap DynamicDataUpdate::list() const
{
    return m_listMap;
}

void DynamicDataUpdate::setList(const QVariantMap &info)
{
    m_listMap = info;
    emit listChanged();
}
