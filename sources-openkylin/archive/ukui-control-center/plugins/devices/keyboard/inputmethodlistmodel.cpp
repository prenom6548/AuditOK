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

#include "inputmethodlistmodel.h"

#include <QDebug>
#include <QList>

InputMethodListModel::InputMethodListModel(const FcitxDbusProvider *provider,
                                           QObject *parent)
    : QObject(parent), provider_(provider) {
  init();
}

void InputMethodListModel::init() {
  if (provider_ == nullptr) {
    return;
  }

  if (provider_->dbusProxy() == nullptr) {
    return;
  }
  inputMethodInfoFromCurrentGroup();
  availableInputMethods();
}

void InputMethodListModel::availableInputMethods() {
  auto imcall = provider_->dbusProxy()->AvailableInputMethods();
  auto imcallwatcher = new QDBusPendingCallWatcher(imcall, this);
  QObject::connect(imcallwatcher, &QDBusPendingCallWatcher::finished, this,
                   [this](QDBusPendingCallWatcher *watcher) {
                     QDBusPendingReply<FcitxQtInputMethodEntryList> ims =
                         *watcher;
                     if (!ims.isError()) {
                       availableInputMethods_ = ims.value();
                       availableInputMethodsChanged(availableInputMethods_,
                                                    currentGroupInputMethods_);
                     }

                     watcher->deleteLater();
                   });
}

void InputMethodListModel::inputMethodInfoFromCurrentGroup() {
  auto imcall0 = provider_->dbusProxy()->CurrentInputMethodGroup();
  auto imcallwatcher0 = new QDBusPendingCallWatcher(imcall0, this);
  connect(imcallwatcher0, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QString> group = *watcher;
            if (!group.isError()) {
              currentGroup_ = group.value();
            }

            watcher->deleteLater();
          });

  auto imcall1 =
      provider_->dbusProxy()->FullInputMethodGroupInfo(currentGroup_);
  auto imcallwatcher1 = new QDBusPendingCallWatcher(imcall1, this);
  connect(imcallwatcher1, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QString, QString, QString, QVariantMap,
                              FcitxQtFullInputMethodEntryList>
                allInfo = *watcher;
            if (!allInfo.isError()) {
              currentLayout_ = allInfo.argumentAt<2>();
              qInfo() << "InputMethodListModel func:" << __FUNCTION__ << " line : " << __LINE__ << ", currentLayout_:" << currentLayout_;
              currentGroupInputMethods_ = allInfo.argumentAt<4>();
              notifyInputMethodsChanged();
              emit currentGroupInputMethodsChanged();
            }

            watcher->deleteLater();
          });
}

void InputMethodListModel::addInputMethod(QString uniqueName) {
  if (uniqueName.isEmpty()) {
    qWarning() << "InputMethodListModel func:" << __FUNCTION__ << " line : " << __LINE__ << ", uniqueName empty!";
    return;
  }

  for (const auto &im : availableInputMethods_) {
    if (im.uniqueName() == uniqueName) {
      FcitxQtFullInputMethodEntry fullEntry;
      fullEntry.setUniqueName(im.uniqueName());
      fullEntry.setName(im.name());
      fullEntry.setNativeName(im.nativeName());
      fullEntry.setIcon(im.icon());
      fullEntry.setLabel(im.label());
      fullEntry.setLanguageCode(im.languageCode());
      fullEntry.setConfigurable(im.configurable());
      fullEntry.setLayout("");
      fullEntry.setProperties(QVariantMap());
      currentGroupInputMethods_.append(fullEntry);
      break;
    }
  }
}

void InputMethodListModel::deleteInputMethod(int index) {
  if (index < 0 || index >= currentGroupInputMethods_.size()) {
    qDebug() << "Invalid index for deleting input method:" << index;
    return;
  }
  currentGroupInputMethods_.removeAt(index);
}

void InputMethodListModel::moveUpInputMethod(int index) {
  if (index < 1 || index >= currentGroupInputMethods_.size()) {
    qDebug() << "Invalid index for moving up input method:" << index;
    return;
  }

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  currentGroupInputMethods_.swapItemsAt(index, index - 1);
#else
  currentGroupInputMethods_.swap(index, index - 1);
#endif
}
void InputMethodListModel::moveDownInputMethod(int index) {
  if (index < 0 || index >= currentGroupInputMethods_.size() - 1) {
    qDebug() << "Invalid index for moving down input method:" << index;
    return;
  }

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  currentGroupInputMethods_.swapItemsAt(index, index + 1);
#else
  currentGroupInputMethods_.swap(index, index + 1);
#endif
}

void InputMethodListModel::moveTo(int srcIndex, int destIndex) {
  if (srcIndex < 0 || srcIndex >= currentGroupInputMethods_.size()) {
    qDebug() << "Invalid source index for moving input method:" << srcIndex;
    return;
  }
  if (destIndex < 0 || destIndex >= currentGroupInputMethods_.size()) {
    qDebug() << "Invalid destination index for moving input method:"
             << destIndex;
    return;
  }

  if (srcIndex == destIndex) {
    qDebug() << "Source and destination indices are the same, no move needed.";
    return;
  }

  FcitxQtFullInputMethodEntry temp = currentGroupInputMethods_.takeAt(srcIndex);

  currentGroupInputMethods_.insert(destIndex, temp);
}

void InputMethodListModel::updateCurrentGroupInputMethodList() {
  if (provider_ == nullptr) {
    return;
  }

  if (provider_->dbusProxy() == nullptr) {
    return;
  }

  provider_->dbusProxy()->SetInputMethodGroupInfo(
      currentGroup_, currentLayout_, convertToKeyValueList(currentGroupInputMethods_));

  notifyInputMethodsChanged();

  emit currentGroupInputMethodsChanged();
  emit availableInputMethodsChanged(availableInputMethods_,
                               currentGroupInputMethods_);
}

const FcitxQtStringKeyValueList InputMethodListModel::convertToKeyValueList(
    FcitxQtFullInputMethodEntryList inputMethodList) const {
  FcitxQtStringKeyValueList keyValueList;

  for (const auto &entry : inputMethodList) {
    FcitxQtStringKeyValue value;
    value.setKey(entry.uniqueName());
    value.setValue("");
    keyValueList.append(value);
  }

  return keyValueList;
}

void InputMethodListModel::notifyInputMethodsChanged()
{
    if(currentGroupInputMethods_.size() > 0) {
        FcitxQtFullInputMethodEntry temp = currentGroupInputMethods_.at(0);
        emit inputMethodsChanged(temp.name());
    }
}
