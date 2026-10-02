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

#ifndef _INPUTMETHODLISTMODEL_H_
#define _INPUTMETHODLISTMODEL_H_

#include <QObject>
#include <QString>

#include "fcitxdbusprovider.h"

class InputMethodListModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      FcitxQtFullInputMethodEntryList currentGroupInputMethodEntries READ
          getCurrentGroupInputMethods NOTIFY currentGroupInputMethodsChanged)

public:
  InputMethodListModel(const FcitxDbusProvider *provider,
                       QObject *parent = nullptr);
  ~InputMethodListModel() override = default;

signals:
  void currentGroupInputMethodsChanged();

  void availableInputMethodsChanged(
      const FcitxQtInputMethodEntryList &allIMs,
      const FcitxQtFullInputMethodEntryList &currentIMS);

  void inputMethodsChanged(const QString &firstImName);

public:
  Q_INVOKABLE void addInputMethod(QString uniqueName);
  Q_INVOKABLE void deleteInputMethod(int index);
  Q_INVOKABLE void moveUpInputMethod(int index);
  Q_INVOKABLE void moveDownInputMethod(int index);
  Q_INVOKABLE void moveTo(int srcIndex, int destIndex);
  Q_INVOKABLE void updateCurrentGroupInputMethodList();

private:
  void init();
  void availableInputMethods();
  void inputMethodInfoFromCurrentGroup();

  const FcitxQtStringKeyValueList
  convertToKeyValueList(FcitxQtFullInputMethodEntryList inputMethodList) const;
  const FcitxQtFullInputMethodEntryList getCurrentGroupInputMethods() const {
    return currentGroupInputMethods_;
  }

  void notifyInputMethodsChanged();

private:
  const FcitxDbusProvider *provider_ = nullptr;
  FcitxQtInputMethodEntryList availableInputMethods_;
  FcitxQtFullInputMethodEntryList currentGroupInputMethods_;
  QString currentGroup_;
  QString currentLayout_;
};
#endif
