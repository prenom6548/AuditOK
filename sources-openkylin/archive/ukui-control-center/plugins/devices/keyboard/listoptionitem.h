/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LISTOPTIONITEM_H
#define LISTOPTIONITEM_H

#include <QAbstractListModel>
#include <QObject>
#include <QVariant>
#include <functional>
#include <memory>

#include "configitem.h"
#include "configvarianthelper.h"
#include "fcitxdbustypes.h"

class ListOptionItemModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit ListOptionItemModel(QObject *parent = nullptr)
      : QAbstractListModel(parent) {}

  void setPrettifyHandler(std::function<QString(const QVariant &)> handler) {
    m_prettify = std::move(handler);
  }

  int rowCount(const QModelIndex &parent = QModelIndex()) const override {
    if (parent.isValid()) {
      return 0;
    }
    return m_values.size();
  }

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override {
    if (!index.isValid() || index.row() >= m_values.size()) {
      return QVariant();
    }

    const auto &value = m_values.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
      return m_prettify ? m_prettify(value) : value.toString();
    case Qt::UserRole:
      return normalizeVariant(value);
    }
    return QVariant();
  }

  QHash<int, QByteArray> roleNames() const override {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::UserRole] = "value";
    return roles;
  }

  void readValueFrom(const QVariantMap &map, const QString &path);
  void writeValueTo(QVariantMap &map, const QString &path);

  bool insertRows(int row, int count,
                  const QModelIndex &parent = QModelIndex()) override;
  bool removeRows(int row, int count,
                  const QModelIndex &parent = QModelIndex()) override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  void addItem(const QVariant &value = QVariant());
  void removeItem(int index);
  void moveUp(int index);
  void moveDown(int index);
  void editItem(int index, const QVariant &value);
  QVariant getItem(int index) const;

private:
  QList<QVariant> m_values;
  std::function<QString(const QVariant &)> m_prettify;
};

class ListOptionItem : public ConfigItem {
  Q_OBJECT
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)
  Q_PROPERTY(QString desc READ desc CONSTANT)
  Q_PROPERTY(QAbstractListModel *model READ model CONSTANT)
  Q_PROPERTY(int count READ count NOTIFY countChanged)
  Q_PROPERTY(QString subType READ subType CONSTANT)
  Q_PROPERTY(QString subDescription READ subDescription CONSTANT)
  Q_PROPERTY(QVariantMap subProperties READ subProperties CONSTANT)
  Q_PROPERTY(QVariant defaultSubValue READ defaultSubValue CONSTANT)
  Q_PROPERTY(QVariantList subFields READ subFields CONSTANT)
  Q_PROPERTY(QVariantList subEnumOptions READ subEnumOptions CONSTANT)

public:
  ListOptionItem(const FcitxQtConfigOption &option, const QString &path,
                 QObject *parent);

  void readValueFrom(const QVariantMap &map) override;
  void writeValueTo(QVariantMap &map) override;
  void restoreToDefault() override;

  Q_INVOKABLE void addItem(const QVariant &value = QVariant());
  Q_INVOKABLE void removeItem(int index);
  Q_INVOKABLE void moveUp(int index);
  Q_INVOKABLE void moveDown(int index);
  Q_INVOKABLE void editItem(int index, const QVariant &value);
  Q_INVOKABLE QVariant getItem(int index) const;
  Q_INVOKABLE QVariant prepareEditValue(const QVariant &rawValue) const;
  Q_INVOKABLE QVariant serializeEditValue(const QVariant &editValue) const;

  QString name() const;
  QString type() const;
  QString desc() const;
  QString subType() const;
  QString subDescription() const;
  QAbstractListModel *model() const;
  int count() const;
  QVariantMap subProperties() const;
  QVariant defaultSubValue() const;
  QVariantList subFields() const;
  QVariantList subEnumOptions() const;

Q_SIGNALS:
  void countChanged();

private:
  QVariantMap parseStructuredValue(const QVariant &rawValue) const;
  QVariantMap serializeStructuredValue(const QVariant &editValue) const;
  QString prettifyValue(const QVariant &value) const;
  QString prettifyOption(const FcitxQtConfigOption &option,
                         const QVariant &value) const;

  QString m_name;
  QString m_desc;
  std::unique_ptr<ListOptionItemModel> m_model;
  QString m_type = "List";
  QString m_subType;
  QVariantMap m_defaultValue;
  FcitxQtConfigOption m_subOption;
  QVariantList m_subFields;
  mutable bool m_subEnumInitialized = false;
  mutable QVariantList m_subEnumOptions;
};

#endif // LISTOPTIONITEM_H
