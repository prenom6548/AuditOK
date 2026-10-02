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

#ifndef __ADDINPUTMETHODMODEL_H_
#define __ADDINPUTMETHODMODEL_H_

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QSortFilterProxyModel>

#include "fcitxdbusprovider.h"

class SearchDataModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit SearchDataModel(QObject *parent = nullptr);
  ~SearchDataModel() override = default;

  enum Roles {
    UniqueNameRole = Qt::UserRole + 1,
    NameRole,
    IconRole,
    LanguageRole
  };

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Roles::NameRole) const override;
  QHash<int, QByteArray> roleNames() const override;
  void addData(const FcitxQtInputMethodEntryList &allIMs,
               FcitxQtFullInputMethodEntryList currentIMS);

public slots:
  void onAvailableInputMethodsChanged(const FcitxQtInputMethodEntryList &allIMs, const FcitxQtFullInputMethodEntryList &currentIMS);

private:
  FcitxQtInputMethodEntryList filterInputMethodList_;
};

class SearchInputMethodModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY
                 filterTextChanged)
  Q_PROPERTY(
      bool onlyShowCurrentLanguage READ onlyShowCurrentLanguage WRITE
          setOnlyShowCurrentLanguage NOTIFY onlyShowCurrentLanguageChanged)

public:
  SearchInputMethodModel(const FcitxDbusProvider *provider,
                         QObject *parent = nullptr);
  ~SearchInputMethodModel() override = default;

protected:
  bool filterAcceptsRow(int source_row,
                        const QModelIndex &source_parent) const override;

private:
  const QString filterText() const;
  void setFilterText(const QString &text);
  bool onlyShowCurrentLanguage() const;
  void setOnlyShowCurrentLanguage(bool flag);
  bool isMatchCurrentLanguage(const QString &inputMethodLang) const;

private:
  const FcitxDbusProvider *provider_ = nullptr;
  QString filterText_;
  bool onlyShowCurrentLanguage_ = true;

Q_SIGNALS:
  void filterTextChanged();
  void onlyShowCurrentLanguageChanged();
};
#endif
