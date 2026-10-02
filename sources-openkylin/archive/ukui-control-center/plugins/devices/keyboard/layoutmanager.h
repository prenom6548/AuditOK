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

#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include "fcitxdbusprovider.h"
#include "languagecode.h"
#include "layoutmodel.h"
#include <QSet>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringListModel>

class LayoutManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(LanguageModel *languageModel READ languageModel CONSTANT)
  Q_PROPERTY(LanguageFilterModel *layoutModel READ layoutModel CONSTANT)
  Q_PROPERTY(LanguageFilterModel *variantModel READ variantModel CONSTANT)
  Q_PROPERTY(QString defaultLayout READ defaultLayout WRITE setDefaultLayout)
  Q_PROPERTY(FcitxQtStringKeyValueList imEntries READ imEntries CONSTANT)

public:
  LayoutManager(FcitxDbusProvider *dbus, QObject *parent = nullptr);
  ~LayoutManager();

  auto languageModel() const { return m_languageModel.get(); }
  auto layoutModel() const { return m_layoutFilterModel.get(); }
  auto variantModel() const { return m_variantFilterModel.get(); }

  Q_INVOKABLE int layoutIndex(const QString &layoutString);
  Q_INVOKABLE int variantIndex(const QString &layoutString);
  Q_INVOKABLE QString layoutDescription(const QString &layoutString);
  Q_INVOKABLE void joinImEntries(const QString &im);

  Q_INVOKABLE void setVariantInfo(const FcitxQtLayoutInfo &info) const {
    m_variantModel->setVariantInfo(info);
  }

  Q_INVOKABLE QString layout(int layoutIdx, int variantIdx) const {
    auto layoutModelIndex = m_layoutFilterModel->index(layoutIdx, 0);
    auto variantModelIndex = m_variantFilterModel->index(variantIdx, 0);
    if (layoutModelIndex.isValid() && variantModelIndex.isValid()) {
      auto layout = layoutModelIndex.data(Qt::UserRole).toString();
      auto variant = variantModelIndex.data(Qt::UserRole).toString();
      if (layout.isEmpty()) {
        return QString();
      }
      if (variant.isEmpty()) {
        return layout;
      }
      return QString("%1-%2").arg(layout, variant);
    }
    return QString();
  }

  bool loaded() const { return m_loaded; }

signals:
  void loadedChanged();
  void layoutChanged(const QString &currentLayout);

public slots:
  void save();
  void getLayouts();
  void getCurrentLayout();
  void fetchLayoutFinished(QDBusPendingCallWatcher *watcher);

private:
  void setLoaded(bool loaded) {
    if (m_loaded != loaded) {
      m_loaded = loaded;
      Q_EMIT loadedChanged();
    }
  }

  QString defaultLayout() const { return m_defaultLayout; }

  void setDefaultLayout(QString layout) {
    if (m_defaultLayout != layout) {
      m_defaultLayout = layout;
    }
  }

  FcitxQtStringKeyValueList imEntries() const { return m_imEntries; }

  FcitxDbusProvider *m_provider;
  bool m_loaded = false;
  QString m_currentGroup;
  QString m_currentLayout;
  FcitxQtStringKeyValueList m_imEntries;
  QString m_defaultLayout;
  std::unique_ptr<LanguageModel> m_languageModel;
  std::unique_ptr<LayoutInfoModel> m_layoutModel;
  std::unique_ptr<VariantInfoModel> m_variantModel;
  std::unique_ptr<LanguageFilterModel> m_layoutFilterModel;
  std::unique_ptr<LanguageFilterModel> m_variantFilterModel;
  LanguageCode m_languageCode;
};

#endif // LAYOUTMANAGER_H
