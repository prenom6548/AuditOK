// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QJsonObject>

#include "DynamicModel.h"

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

void DynamicWidgetModel::clear()
{
    beginResetModel();
    m_dataList.clear();
    endResetModel();

    m_checkBoxStates.clear();
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

        QVariantMap item;
        item["data"] = data;
        item["child"] = childList;
        item["type"] = "Custom";
        listItem.append(item);
    }
}

void DynamicWidgetModel::loadFromJson(const QJsonObject &req)
{
    beginResetModel();

    if (req.contains("check_boxes") && req["check_boxes"].isArray()) {
        loadCheckboxes("", m_dataList, req["check_boxes"].toArray());
    }

    if (req.contains("customs") && req["customs"].isArray()) {
        loadCustoms("", m_dataList, req["customs"].toArray());
    }

    endResetModel();
}

QMap<QString, bool> DynamicWidgetModel::getCheckBoxStates() const
{
    return m_checkBoxStates;
}
