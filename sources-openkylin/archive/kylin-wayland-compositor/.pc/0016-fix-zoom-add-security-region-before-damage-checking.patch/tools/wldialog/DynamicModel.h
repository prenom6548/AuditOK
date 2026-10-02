// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef DYNAMIC_WIDGET_MODEL_H
#define DYNAMIC_WIDGET_MODEL_H

#include <QAbstractListModel>
#include <QJsonArray>

class DynamicWidgetModel : public QAbstractListModel
{
    Q_OBJECT
  public:
    explicit DynamicWidgetModel(QObject *parent = nullptr);
    ~DynamicWidgetModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void loadFromJson(const QJsonObject &req);

    QMap<QString, bool> getCheckBoxStates() const;

  public slots:
    void updateCheckBoxState(const QString &name, bool checked);
    void clear();

  private:
    void loadCheckboxes(const QString &parent, QVariantList &listItem, const QJsonArray &req);
    void loadCustoms(const QString &parent, QVariantList &listItem, const QJsonArray &req);

    QVariantList m_dataList;

    QMap<QString, bool> m_checkBoxStates;
};

#endif // DYNAMIC_WIDGET_MODEL_H
