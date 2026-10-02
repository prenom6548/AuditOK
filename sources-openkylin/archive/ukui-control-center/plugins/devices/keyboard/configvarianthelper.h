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

#ifndef CONFIGVARIANTHELPER_H
#define CONFIGVARIANTHELPER_H

#include <QDebug>
#include <QObject>
#include <QStringList>
#include <QtCore>
#include <QtDBus>

static QVariantMap toMap(const QVariant &variant) {
  QVariantMap map;
  if (variant.canConvert<QDBusArgument>()) {
    auto argument = qvariant_cast<QDBusArgument>(variant);
    argument >> map;
  }
  if (variant.canConvert<QVariantMap>()) {
    map = variant.toMap();
  }
  return map;
}

static QString valueFromVariantMapByPath(const QVariantMap &map,
                                         const QStringList &path, int depth) {
  auto iter = map.find(path[depth]);
  if (iter == map.end()) {
    return QString();
  }
  if (depth + 1 == path.size()) {
    if (iter->canConvert<QString>()) {
      return iter->toString();
    }
  } else {
    QVariantMap map = toMap(*iter);

    if (!map.isEmpty()) {
      return valueFromVariantMapByPath(map, path, depth + 1);
    }
  }
  return QString();
}

static QVariant valueFromVariantHelper(const QVariant &value,
                                       const QStringList &pathList, int depth) {
  if (depth == pathList.size()) {
    return value;
  }
  auto map = toMap(value);
  // Make it finishes faster.
  if (map.isEmpty() || !map.contains(pathList[depth])) {
    return {};
  }
  return valueFromVariantHelper(map[pathList[depth]], pathList, depth + 1);
}

static QVariant readVariant(const QVariant &value, const QString &path) {
  auto pathList = path.split("/");
  return valueFromVariantHelper(toMap(value), pathList, 0);
}

static QString readString(const QVariantMap &map, const QString &path) {
  //    qDebug() << "path:" << path;

  auto pathList = path.split("/");
  if (pathList.empty()) {
    return QString();
  }
  return valueFromVariantMapByPath(map, pathList, 0);
}

static bool readBool(const QVariantMap &map, const QString &path) {
  return readString(map, path) == "True";
}

static void writeVariantHelper(QVariantMap &map, const QStringList &path,
                               const QVariant &value, int depth) {
  if (depth + 1 == path.size()) {
    map[path[depth]] = value;
  } else {
    auto iter = map.find(path[depth]);
    if (iter == map.end()) {
      iter = map.insert(path[depth], QVariantMap());
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (iter->typeId() != QMetaType::QVariantMap) {
#else
    if (iter->type() != QVariant::Map) {
#endif
      auto oldValue = *iter;
      *iter = QVariantMap({{"", oldValue}});
    }

    auto &nextMap = *static_cast<QVariantMap *>(iter->data());
    writeVariantHelper(nextMap, path, value, depth + 1);
  }
}

static void writeVariant(QVariantMap &map, const QString &path,
                         const QVariant &value) {
  auto pathList = path.split("/");
  if (pathList.empty()) {
    return;
  }
  writeVariantHelper(map, pathList, value, 0);
}

static QVariant normalizeVariant(const QVariant &value) {
  if (value.canConvert<QDBusArgument>()) {
    auto argument = qvariant_cast<QDBusArgument>(value);
    QVariantMap map;
    argument >> map;
    for (auto it = map.begin(); it != map.end(); ++it) {
      it.value() = normalizeVariant(it.value());
    }
    return map;
  }
  if (value.type() == QVariant::Map) {
    QVariantMap map = value.toMap();
    for (auto it = map.begin(); it != map.end(); ++it) {
      it.value() = normalizeVariant(it.value());
    }
    return map;
  }
  if (value.type() == QVariant::List) {
    QVariantList list = value.toList();
    QVariantList result;
    for (const auto &item : list) {
      result.append(normalizeVariant(item));
    }
    return result;
  }
  return value;
}

#endif // CONFIGVARIANTHELPER_H
