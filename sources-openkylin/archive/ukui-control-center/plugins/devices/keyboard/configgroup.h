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

#ifndef CONFIGGROUP_H
#define CONFIGGROUP_H

#include "configitem.h"
#include <QObject>
#include <memory>

class ConfigGroup : public QObject {
  Q_OBJECT
  Q_PROPERTY(QList<ConfigItem *> items READ items NOTIFY itemsChanged)
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString description READ description CONSTANT)

public:
  ConfigGroup(const QString &name, const QString &description, QObject *parent)
      : QObject(parent), m_name(name), m_description(description) {}
  ~ConfigGroup() override = default;

  void releaseItems();
  void addItem(std::unique_ptr<ConfigItem> item);
  QList<ConfigItem *> items() const {
    QList<ConfigItem *> result;
    for (auto &item : m_items) {
      result << item.get();
    }
    return result;
  }

private:
  QString name() const { return m_name; }
  QString description() const { return m_description; }

private:
  std::vector<std::unique_ptr<ConfigItem>> m_items;
  QString m_name;
  QString m_description;

signals:
  void itemsChanged();
};

#endif // CONFIGGROUP_H
