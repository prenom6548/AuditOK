// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef DYNAMICWIDGETMODEL_H
#define DYNAMICWIDGETMODEL_H

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

    const QMap<QString, bool> &getCheckBoxStates() const;
    const QMap<QString, QString> &getTabPages() const;
    const QMap<QString, QStringList> &getListKeys() const;

    static QVariantMap buildListItem(const QJsonObject &req, const QString &parent = "");

  public slots:
    void updateCheckBoxState(const QString &name, bool checked);
    void updateCurrentTabPage(const QString &name, const QString &curPage);
    void updateListItems(const QString &name, const QStringList &keys);

    void clear();

  private:
    void loadCheckboxes(const QString &parent, QVariantList &listItem, const QJsonArray &req);
    void loadCustoms(const QString &parent, QVariantList &listItem, const QJsonArray &req);
    void loadTabPages(const QString &parent, QVariantList &listItem, const QJsonObject &json);
    void loadList(const QString &parent, QVariantList &listItem, const QJsonObject &json);

    QVariantList m_dataList;

    QMap<QString, bool> m_checkBoxStates;
    QMap<QString, QString> m_tabPages;
    QMap<QString, QStringList> m_listKeys;
};

#endif // DYNAMICWIDGETMODEL_H
