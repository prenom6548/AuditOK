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

#include "layoutmanager.h"
#include <QDebug>

LayoutManager::LayoutManager(FcitxDbusProvider *dbus, QObject *parent)
    : QObject(parent), m_provider(dbus),
      m_languageModel(new LanguageModel(this)),
      m_layoutModel(new LayoutInfoModel(this)),
      m_variantModel(new VariantInfoModel(this)),
      m_layoutFilterModel(new LanguageFilterModel(this)),
      m_variantFilterModel(new LanguageFilterModel(this)) {
  m_layoutFilterModel->setSourceModel(m_layoutModel.get());
  m_variantFilterModel->setSourceModel(m_variantModel.get());
}

LayoutManager::~LayoutManager() {}

void LayoutManager::getLayouts() {
  setLoaded(false);
  if (!m_provider && !m_provider->dbusProxy()) {
    return;
  }

  auto call = m_provider->dbusProxy()->AvailableKeyboardLayouts();
  auto watcher = new QDBusPendingCallWatcher(call, this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          &LayoutManager::fetchLayoutFinished);
}

void LayoutManager::fetchLayoutFinished(QDBusPendingCallWatcher *watcher) {
  watcher->deleteLater();
  QDBusPendingReply<FcitxQtLayoutInfoList> reply = *watcher;
  if (reply.isError()) {
    return;
  }
  QSet<QString> languages;
  auto layoutInfo = reply.value();
  for (const auto &layout : layoutInfo) {
    for (const auto &language : layout.languages()) {
      languages << language;
    }
    for (const auto &variant : layout.variants()) {
      for (const auto &language : variant.languages()) {
        languages << language;
      }
    }
  }
  QStringList languageList;
  for (const auto &language : languages) {
    languageList << language;
  }
  languageList.sort();
  m_languageModel->clear();

  QStandardItem *item = new QStandardItem(tr("Any language"));
  item->setData("", Qt::UserRole);
  m_languageModel->append(tr("Any language"), "");
  for (const auto &language : languageList) {
    QString languageName = m_languageCode.codeToLocale(language);
    if (languageName.isEmpty()) {
      languageName = language;
    } else {
      languageName = QString("%1 (%2)").arg(languageName, language);
    }
    m_languageModel->append(languageName, language);
  }
  m_layoutModel->setLayoutInfo(std::move(layoutInfo));
  setLoaded(true);
}

void LayoutManager::getCurrentLayout() {
  if (!m_provider && !m_provider->dbusProxy()) {
    return;
  }
  auto imcall0 = m_provider->dbusProxy()->CurrentInputMethodGroup();
  auto imcallwatcher0 = new QDBusPendingCallWatcher(imcall0, this);
  connect(imcallwatcher0, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QString> group = *watcher;
            if (!group.isError()) {
              m_currentGroup = group.value();
            }
            watcher->deleteLater();
          });
  imcallwatcher0->waitForFinished();
  auto imcall1 = m_provider->dbusProxy()->InputMethodGroupInfo(m_currentGroup);

  auto imcallwatcher1 = new QDBusPendingCallWatcher(imcall1, this);
  connect(imcallwatcher1, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QString, FcitxQtStringKeyValueList> reply =
                *watcher;
            if (!reply.isError()) {
              m_currentLayout = reply.argumentAt<0>();
              m_imEntries = reply.argumentAt<1>();
              emit layoutChanged(m_currentLayout);
            } else {
              m_currentLayout.clear();
              m_imEntries.clear();
            }
            watcher->deleteLater();
          });
}

int LayoutManager::layoutIndex(const QString &layoutString) {
  auto dashPos = layoutString.indexOf("-");
  QString layout;
  if (dashPos >= 0) {
    layout = layoutString.left(dashPos);
  } else {
    layout = layoutString;
  }
  auto &info = m_layoutModel->layoutInfo();
  auto iter = std::find_if(info.begin(), info.end(),
                           [&layout](const FcitxQtLayoutInfo &info) {
                             return info.layout() == layout;
                           });
  if (iter != info.end()) {
    auto row = std::distance(info.begin(), iter);
    return m_layoutFilterModel->mapFromSource(m_layoutModel->index(row)).row();
  }
  return 0;
}

int LayoutManager::variantIndex(const QString &layoutString) {
  auto dashPos = layoutString.indexOf("-");
  QString variant;
  if (dashPos >= 0) {
    variant = layoutString.mid(dashPos + 1);
  }
  auto &vinfo = m_variantModel->variantInfo();
  auto iter = std::find_if(vinfo.begin(), vinfo.end(),
                           [&variant](const FcitxQtVariantInfo &info) {
                             return info.variant() == variant;
                           });
  if (iter != vinfo.end()) {
    auto row = std::distance(vinfo.begin(), iter);
    return m_variantFilterModel->mapFromSource(m_variantModel->index(row))
        .row();
  }
  return 0;
}

QString LayoutManager::layoutDescription(const QString &layoutString) {
  auto dashPos = layoutString.indexOf("-");
  QString layout;
  QString variant;
  if (dashPos >= 0) {
    layout = layoutString.left(dashPos);
    variant = layoutString.mid(dashPos + 1);
  } else {
    layout = layoutString;
  }
  auto &info = m_layoutModel->layoutInfo();
  auto iter = std::find_if(info.begin(), info.end(),
                           [&layout](const FcitxQtLayoutInfo &info) {
                             return info.layout() == layout;
                           });
  if (iter == info.end()) {
    return QString();
  }

  if (variant.isEmpty()) {
    return iter->description();
  }

  auto variantIter =
      std::find_if(iter->variants().begin(), iter->variants().end(),
                   [&variant](const FcitxQtVariantInfo &info) {
                     return info.variant() == variant;
                   });
  if (variantIter == iter->variants().end()) {
    return iter->description();
  }
  return QString(("%1 - %2"))
      .arg(iter->description(), variantIter->description());
}

void LayoutManager::joinImEntries(const QString &im) {
  FcitxQtStringKeyValue imEntry;
  int i = 0;
  auto imEntries = m_imEntries;
  for (; i < imEntries.size(); i++) {
    if (imEntries[i].key() == im) {
      imEntry = imEntries[i];
      imEntries.removeAt(i);
      break;
    }
  }
  if (i == imEntries.size()) {
    imEntry.setKey(im);
  }
  imEntries.push_front(imEntry);
  m_imEntries = imEntries;
}

void LayoutManager::save() {
  if (!m_provider && !m_provider->dbusProxy()) {
    return;
  }

  m_provider->dbusProxy()->SetInputMethodGroupInfo(
      m_currentGroup, m_defaultLayout, m_imEntries);
}
