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

#ifndef CONFIGITEM_H
#define CONFIGITEM_H

#include "fcitxdbustypes.h"
#include <QDebug>
#include <QObject>
#include <QStringList>

class ConfigItem : public QObject {
  Q_OBJECT

public:
  explicit ConfigItem(const QString &path, QObject *parent)
      : QObject(parent), m_path(path) {}

  ~ConfigItem() override = default;
  static std::unique_ptr<ConfigItem> create(const FcitxQtConfigOption &option,
                                            const QString &path,
                                            QObject *parent);

  virtual void readValueFrom(const QVariantMap &map) = 0;
  virtual void writeValueTo(QVariantMap &map) = 0;
  virtual void restoreToDefault() = 0;
  virtual bool isValid() const { return true; }

  const QString &path() const { return m_path; }

private:
  QString m_path;

signals:
  void valueChanged();
};

#endif // CONFIGITEM_H
