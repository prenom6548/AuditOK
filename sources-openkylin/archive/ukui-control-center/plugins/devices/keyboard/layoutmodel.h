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

#ifndef LAYOUTMODEL_H
#define LAYOUTMODEL_H

#include "fcitxdbustypes.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

enum { LayoutLanguageRole = 0x3423545, LayoutInfoRole };

class LanguageModel : public QStandardItemModel {
  Q_OBJECT
public:
  LanguageModel(QObject *parent = nullptr);
  Q_INVOKABLE QString language(int row) const;
  void append(const QString &name, const QString &language);
};

class LanguageFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QString language READ language WRITE setLanguage);

public:
  using QSortFilterProxyModel::QSortFilterProxyModel;
  // Forward role names.
  QHash<int, QByteArray> roleNames() const override {
    if (sourceModel()) {
      return sourceModel()->roleNames();
    }
    return QSortFilterProxyModel::roleNames();
  }

  const QString &language() const { return m_language; }
  void setLanguage(const QString &language);

  Q_INVOKABLE QVariant layoutInfo(int row) const;

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &) const override;
  bool lessThan(const QModelIndex &left,
                const QModelIndex &right) const override;

private:
  QString m_language;
};

class LayoutInfoModel : public QAbstractListModel {
  Q_OBJECT
public:
  using QAbstractListModel::QAbstractListModel;
  QHash<int, QByteArray> roleNames() const override;

  auto &layoutInfo() const { return m_layoutInfo; }
  void setLayoutInfo(FcitxQtLayoutInfoList info);

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
  FcitxQtLayoutInfoList m_layoutInfo;
};

class VariantInfoModel : public QAbstractListModel {
  Q_OBJECT
public:
  using QAbstractListModel::QAbstractListModel;
  QHash<int, QByteArray> roleNames() const override;

  auto &variantInfo() const { return m_variantInfo; }
  void setVariantInfo(const FcitxQtLayoutInfo &info);

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
  FcitxQtVariantInfoList m_variantInfo;
};

#endif // LAYOUTMODEL_H
