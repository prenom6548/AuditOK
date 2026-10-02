// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QJsonObject>

#include "DynamicModel.h"
#include "IconUtils.h"

enum DynamicWidgetRoles {
    DynamicWidgetRole = Qt::UserRole + 1,
};

DynamicWidgetModel::DynamicWidgetModel(QObject *parent) : QAbstractListModel(parent) {}

DynamicWidgetModel::~DynamicWidgetModel()
{
    clear();
}

int DynamicWidgetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_dataList.size();
}

QVariant DynamicWidgetModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == DynamicWidgetRole) {
        return m_dataList.at(index.row());
    }

    return QVariant();
}

QHash<int, QByteArray> DynamicWidgetModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DynamicWidgetRole] = "widgetData";
    return roles;
}

void DynamicWidgetModel::updateCheckBoxState(const QString &name, bool checked)
{
    if (!name.isEmpty()) {
        m_checkBoxStates[name] = checked;
    }
}

void DynamicWidgetModel::updateCurrentTabPage(const QString &name, const QString &curPage)
{
    if (!name.isEmpty()) {
        m_tabPages[name] = curPage;
    }
}

void DynamicWidgetModel::updateListItems(const QString &name, const QStringList &keys)
{
    if (!name.isEmpty()) {
        m_listKeys[name] = keys;
    }
}

void DynamicWidgetModel::clear()
{
    beginResetModel();
    m_dataList.clear();
    endResetModel();

    m_checkBoxStates.clear();
    m_tabPages.clear();
    m_listKeys.clear();
}

void DynamicWidgetModel::loadCheckboxes(const QString &parent, QVariantList &listItem,
                                        const QJsonArray &req)
{
    for (const auto &val : req) {
        QJsonObject obj = val.toObject();
        QString name = obj["name"].toString();
        QVariantMap itemData;
        itemData["name"] = parent.isEmpty() ? name : (parent + "|" + name);
        if (obj.contains("text")) {
            itemData["text"] = obj["text"].toString();
        }
        if (obj.contains("default")) {
            itemData["default"] = obj["default"].toBool();
        }

        QVariantMap item;
        item["data"] = itemData;
        item["type"] = "CheckBox";
        listItem.append(item);
    }
}

void DynamicWidgetModel::loadCustoms(const QString &parent, QVariantList &listItem,
                                     const QJsonArray &req)
{
    for (const auto &val : req) {
        QJsonObject obj = val.toObject();
        QVariantMap data;
        if (obj.contains("title")) {
            data["title"] = obj["title"].toString();
        }

        QVariantList childList;
        if (obj.contains("check_boxes") && obj["check_boxes"].isArray()) {
            loadCheckboxes(parent, childList, obj["check_boxes"].toArray());
        }

        if (obj.contains("list") && obj["list"].isObject()) {
            loadList(parent, childList, obj["list"].toObject());
        }

        QVariantMap item;
        item["data"] = data;
        item["child"] = childList;
        item["type"] = "Custom";
        listItem.append(item);
    }
}

void DynamicWidgetModel::loadTabPages(const QString &parent, QVariantList &listItem,
                                      const QJsonObject &json)
{
    if (json.isEmpty()) {
        return;
    }

    QVariantMap tabPageItem;
    tabPageItem["type"] = "TabPage";
    QVariantMap dataItem;
    if (json.contains("name")) {
        QString name = json["name"].toString();
        dataItem["name"] = parent.isEmpty() ? name : (parent + "|" + name);
    }
    tabPageItem["data"] = dataItem;

    QVariantList tab_pages = {};
    if (json.contains("data")) {
        QJsonArray array = json["data"].toArray();
        for (const auto &val : array) {
            QJsonObject obj = val.toObject();

            QVariantMap item;
            item["type"] = "tabPageContainer";

            QVariantMap data;
            if (obj.contains("title")) {
                data["title"] = obj["title"].toString();
            }

            data["name"] = obj["name"].toString();
            item["data"] = data;

            QString parentStr = dataItem["name"].toString();
            QString currentParentStr = parentStr.isEmpty()
                                           ? obj["name"].toString()
                                           : (parentStr + "|" + obj["name"].toString());

            QVariantList childList;
            if (obj.contains("check_boxes") && obj["check_boxes"].isArray()) {
                loadCheckboxes(currentParentStr, childList, obj["check_boxes"].toArray());
            }

            if (obj.contains("customs") && obj["customs"].isArray()) {
                loadCustoms(currentParentStr, childList, obj["customs"].toArray());
            }

            if (obj.contains("list") && obj["list"].isObject()) {
                loadList(currentParentStr, childList, obj["list"].toObject());
            }

            item["child"] = childList;

            tab_pages.append(item);
        }
    }

    tabPageItem["child"] = tab_pages;

    listItem.append(tabPageItem);
}

void DynamicWidgetModel::loadList(const QString &parent, QVariantList &listItem,
                                  const QJsonObject &json)
{
    if (json.isEmpty()) {
        return;
    }

    listItem.append(buildListItem(json, parent));
}

QVariantMap DynamicWidgetModel::buildListItem(const QJsonObject &json, const QString &parent)
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
    if (json.contains("multiple")) {
        list["multiple"] = json["multiple"].toBool();
    }

    QVariantMap dataItem;
    QString name;
    if (json.contains("name")) {
        name = json["name"].toString();
        dataItem["name"] = parent.isEmpty() ? name : (parent + "|" + name);
    }

    int textRow = 1;
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

            listItems.append(item);
        }
    }

    list["child"] = listItems;
    dataItem["textRow"] = textRow;
    list["data"] = dataItem;

    return list;
}

void DynamicWidgetModel::loadFromJson(const QJsonObject &req)
{
    beginResetModel();
    m_dataList.clear();
    m_checkBoxStates.clear();
    m_tabPages.clear();
    m_listKeys.clear();

    if (req.contains("check_boxes") && req["check_boxes"].isArray()) {
        loadCheckboxes("", m_dataList, req["check_boxes"].toArray());
    }

    if (req.contains("customs") && req["customs"].isArray()) {
        loadCustoms("", m_dataList, req["customs"].toArray());
    }

    if (req.contains("tab_page")) {
        QJsonObject tabPageObj = req["tab_page"].toObject();
        loadTabPages("", m_dataList, tabPageObj);
    }

    if (req.contains("list")) {
        QJsonObject listObj = req["list"].toObject();
        loadList("", m_dataList, listObj);
    }

    endResetModel();
}

const QMap<QString, bool> &DynamicWidgetModel::getCheckBoxStates() const
{
    return m_checkBoxStates;
}

const QMap<QString, QString> &DynamicWidgetModel::getTabPages() const
{
    return m_tabPages;
}

const QMap<QString, QStringList> &DynamicWidgetModel::getListKeys() const
{
    return m_listKeys;
}
