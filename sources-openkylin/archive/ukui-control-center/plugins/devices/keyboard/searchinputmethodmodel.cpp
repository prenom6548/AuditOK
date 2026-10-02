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

#include "searchinputmethodmodel.h"

#include <QDebug>
#include <QLocale>
#include <QSet>

SearchDataModel::SearchDataModel(QObject *parent)
    : QAbstractListModel(parent) {}

int SearchDataModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return filterInputMethodList_.size();
}

QVariant SearchDataModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || index.row() >= filterInputMethodList_.size())
  {
    return QVariant();
  }

  const auto &entry = filterInputMethodList_[index.row()];
  switch (role)
  {
  case UniqueNameRole:
    return entry.uniqueName();
  case NameRole:
    return entry.name();
  case IconRole:
    return entry.icon();
  case LanguageRole:
    return entry.languageCode();
  default:
    return QVariant();
  }

  return QVariant();
}

QHash<int, QByteArray> SearchDataModel::roleNames() const
{
  return {{UniqueNameRole, "uniqueName"},
          {NameRole, "name"},
          {IconRole, "icon"},
          {LanguageRole, "languageCode"}};
}

void SearchDataModel::addData(const FcitxQtInputMethodEntryList &allIMs,
                              FcitxQtFullInputMethodEntryList currentIMS)
{
  beginResetModel();
  QSet<QString> currentKeys;
  filterInputMethodList_.clear();
  for (const auto &currentEntry : currentIMS)
  {
    currentKeys.insert(currentEntry.uniqueName());
  }

  for (const auto &entry : allIMs)
  {
    if (!currentKeys.contains(entry.uniqueName()))
    {
      filterInputMethodList_.append(entry);
    }
  }

  endResetModel();
}

void SearchDataModel::onAvailableInputMethodsChanged(
    const FcitxQtInputMethodEntryList &allIMs,
    const FcitxQtFullInputMethodEntryList &currentIMS)
{
  addData(allIMs, currentIMS);
}

SearchInputMethodModel::SearchInputMethodModel(
    const FcitxDbusProvider *provider, QObject *parent)
    : QSortFilterProxyModel(parent), provider_(provider)
{
}

const QString SearchInputMethodModel::filterText() const { return filterText_; }

void SearchInputMethodModel::setFilterText(const QString &text)
{
  if (filterText_ == text)
  {
    return;
  }

  filterText_ = text;

  emit filterTextChanged();
  invalidateFilter();
}

bool SearchInputMethodModel::onlyShowCurrentLanguage() const
{
  return onlyShowCurrentLanguage_;
}

void SearchInputMethodModel::setOnlyShowCurrentLanguage(bool flag)
{
  if (onlyShowCurrentLanguage_ == flag)
  {
    return;
  }

  onlyShowCurrentLanguage_ = flag;
  emit onlyShowCurrentLanguageChanged();
  invalidateFilter();
}

bool SearchInputMethodModel::isMatchCurrentLanguage(
    const QString &language) const
{
  const QString sysLang = QLocale::system().name();

  return language.startsWith(sysLang);
}

bool SearchInputMethodModel::filterAcceptsRow(
    int source_row, const QModelIndex &source_parent) const
{
  const auto sourceModel = this->sourceModel();
  const auto index = sourceModel->index(source_row, 0, source_parent);

  if (onlyShowCurrentLanguage_)
  {
    const QString language =
        sourceModel->data(index, SearchDataModel::LanguageRole).toString();
    if (!isMatchCurrentLanguage(language))
    {
      return false;
    }
  }

  const auto name =
      sourceModel->data(index, SearchDataModel::NameRole).toString();
  const auto uniqueName =
      sourceModel->data(index, SearchDataModel::UniqueNameRole).toString();

  return name.contains(filterText_, Qt::CaseInsensitive) ||
         uniqueName.contains(filterText_, Qt::CaseInsensitive);
}
