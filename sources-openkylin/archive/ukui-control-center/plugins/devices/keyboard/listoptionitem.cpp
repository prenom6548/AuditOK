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

#include "listoptionitem.h"

#include <QAbstractListModel>
#include <QDBusVariant>
#include <QJSValue>
#include <QJSValueIterator>
#include <QJsonDocument>
#include <QMap>
#include <QtDBus/QDBusArgument>
#include <algorithm>
#include <functional>
#include <memory>

#include "configmanager.h"
#include "configvarianthelper.h"

namespace {
static QVariant normalizeJSValue(const QVariant &value) {
  if (value.canConvert<QJSValue>()) {
    QJSValue jsValue = value.value<QJSValue>();
    if (jsValue.isArray()) {
      QVariantList list;
      int length = jsValue.property("length").toInt();
      for (int i = 0; i < length; ++i) {
        list.append(normalizeJSValue(jsValue.property(i).toVariant()));
      }
      return list;
    } else if (jsValue.isObject()) {
      QVariantMap map;
      QJSValueIterator it(jsValue);
      while (it.hasNext()) {
        it.next();
        map[it.name()] = normalizeJSValue(it.value().toVariant());
      }
      return map;
    } else {
      return jsValue.toVariant();
    }
  }

  if (value.canConvert<QVariantMap>()) {
    QVariantMap map = value.toMap();
    for (auto it = map.begin(); it != map.end(); ++it) {
      *it = normalizeJSValue(*it);
    }
    return map;
  }

  if (value.canConvert<QVariantList>()) {
    QVariantList list = value.toList();
    for (auto &item : list) {
      item = normalizeJSValue(item);
    }
    return list;
  }

  return value;
}

const ConfigModel *getConfigModel(const QObject *object) {
  const QObject *parent = object;
  const ConfigModel *model = nullptr;
  while (parent) {
    model = qobject_cast<const ConfigModel *>(parent);
    if (model) {
      break;
    }
    parent = parent->parent();
  }
  return model;
}

QVariantList mapToList(const QVariantMap &map) {
  QVariantList list;
  QStringList keys = map.keys();
  std::sort(keys.begin(), keys.end(), [](const QString &a, const QString &b) {
    bool okA = false, okB = false;
    const auto numA = a.toInt(&okA);
    const auto numB = b.toInt(&okB);
    return (okA && okB) ? (numA < numB) : (a < b);
  });
  for (const auto &key : keys) {
    list.append(map.value(key));
  }
  return list;
}

bool toBoolVariant(const QVariant &value, bool fallback = false) {
  if (!value.isValid() || value.isNull()) {
    return fallback;
  }
  if (value.type() == QVariant::Bool) {
    return value.toBool();
  }
  const auto str = value.toString();
  if (str.compare(QStringLiteral("True"), Qt::CaseInsensitive) == 0 ||
      str.compare(QStringLiteral("1")) == 0) {
    return true;
  }
  if (str.compare(QStringLiteral("False"), Qt::CaseInsensitive) == 0 ||
      str.compare(QStringLiteral("0")) == 0) {
    return false;
  }
  return value.toBool();
}

enum class TypeCategory { Boolean, Integer, StringLike, List, Other };

struct TypeDescriptor {
  TypeCategory category = TypeCategory::Other;
  QString listElementType;
};

TypeDescriptor describeType(const QString &type) {
  TypeDescriptor desc;
  if (type == "Boolean") {
    desc.category = TypeCategory::Boolean;
    return desc;
  }
  if (type == "Integer") {
    desc.category = TypeCategory::Integer;
    return desc;
  }
  if (type == "Enum" || type == "String" || type == "Key") {
    desc.category = TypeCategory::StringLike;
    return desc;
  }
  if (type.startsWith("List|")) {
    desc.category = TypeCategory::List;
    desc.listElementType = type.mid(5);
    return desc;
  }
  return desc;
}

QVariantMap buildFieldDefinition(const FcitxQtConfigOption &option) {
  QVariantMap field;
  field["name"] = option.name();
  field["type"] = option.type();
  field["desc"] = option.description();
  field["properties"] = option.properties();
  field["defaultValue"] = normalizeVariant(option.defaultValue().variant());

  if (option.type() == "Enum") {
    QVariantList enumOptions;
    auto props = option.properties();
    int i = 0;
    while (true) {
      const auto key = QString("Enum/%1").arg(i);
      const auto value = readString(props, key);
      if (value.isEmpty()) {
        break;
      }
      auto text = readString(props, QString("EnumI18n/%1").arg(i));
      if (text.isEmpty()) {
        text = value;
      }
      QVariantMap entry;
      entry["value"] = value;
      entry["text"] = text;
      enumOptions << entry;
      i++;
    }
    field["enumOptions"] = enumOptions;
  }
  return field;
}

QVariant normalizeByType(const QString &type, const QVariant &value,
                         const QVariant &defaultValue = QVariant()) {
  QVariant effectiveValue = value;
  const auto descriptor = describeType(type);

  if (!effectiveValue.isValid() || effectiveValue.isNull()) {
    effectiveValue = defaultValue;
  }

  if (!effectiveValue.isValid() || effectiveValue.isNull()) {
    switch (descriptor.category) {
    case TypeCategory::Boolean:
      return false;
    case TypeCategory::Integer:
      return 0;
    case TypeCategory::StringLike:
      return QString();
    case TypeCategory::List:
      return QVariantList();
    case TypeCategory::Other:
      return QVariant();
    }
  }

  switch (descriptor.category) {
  case TypeCategory::Boolean:
    return toBoolVariant(effectiveValue);
  case TypeCategory::Integer: {
    bool ok = false;
    const auto number = effectiveValue.toInt(&ok);
    return ok ? number : 0;
  }
  case TypeCategory::StringLike:
    return effectiveValue.toString();
  case TypeCategory::List:
    if (effectiveValue.canConvert<QVariantMap>()) {
      return mapToList(effectiveValue.toMap());
    }
    return effectiveValue.canConvert<QVariantList>() ? effectiveValue.toList()
                                                     : QVariantList();
  case TypeCategory::Other:
  default:
    return effectiveValue;
  }
}

QVariant serializeByType(const QString &type, const QVariant &value,
                         const QVariant &defaultValue = QVariant()) {
  QVariant effectiveValue =
      value.isValid() && !value.isNull() ? value : defaultValue;
  const auto descriptor = describeType(type);

  switch (descriptor.category) {
  case TypeCategory::Boolean:
    return toBoolVariant(effectiveValue) ? QStringLiteral("True")
                                         : QStringLiteral("False");
  case TypeCategory::Integer:
    return effectiveValue.toInt();
  case TypeCategory::StringLike:
    return effectiveValue.toString();
  case TypeCategory::List: {
    const QString elementType = descriptor.listElementType;
    QVariantList list;
    if (effectiveValue.canConvert<QVariantList>()) {
      list = effectiveValue.toList();
    } else if (effectiveValue.canConvert<QVariantMap>()) {
      list = mapToList(effectiveValue.toMap());
    }
    QVariantMap map;
    for (int i = 0; i < list.size(); ++i) {
      map[QString::number(i)] = serializeByType(elementType, list.at(i));
    }
    return map;
  }
  case TypeCategory::Other:
  default:
    return effectiveValue.toString();
  }
}

} // namespace

void ListOptionItemModel::readValueFrom(const QVariantMap &map,
                                        const QString &path) {
  beginResetModel();
  int i = 0;
  m_values.clear();
  while (true) {
    auto value = readVariant(
        map, QString("%1%2%3").arg(path).arg(path.isEmpty() ? "" : "/").arg(i));
    if (value.isNull()) {
      break;
    }
    m_values << value;
    i++;
  }
  endResetModel();
}

void ListOptionItemModel::writeValueTo(QVariantMap &map, const QString &path) {
  int i = 0;
  for (const auto &value : m_values) {
    QVariant cleanedValue = normalizeJSValue(value);
    writeVariant(map, QString("%1/%2").arg(path).arg(i), cleanedValue);
    i++;
  }
  if (i == 0) {
    map[path] = QVariantMap();
  }
}

bool ListOptionItemModel::insertRows(int row, int count,
                                     const QModelIndex &parent) {
  if (row < 0 || row > m_values.size() || count <= 0) {
    return false;
  }
  beginInsertRows(parent, row, row + count - 1);
  for (int i = 0; i < count; ++i) {
    m_values.insert(row + i, QVariant());
  }
  endInsertRows();
  return true;
}

bool ListOptionItemModel::removeRows(int row, int count,
                                     const QModelIndex &parent) {
  if (row < 0 || row + count > m_values.size() || count <= 0) {
    return false;
  }
  beginRemoveRows(parent, row, row + count - 1);
  for (int i = 0; i < count; ++i) {
    m_values.removeAt(row);
  }
  endRemoveRows();
  return true;
}

bool ListOptionItemModel::setData(const QModelIndex &index,
                                  const QVariant &value, int role) {
  if (!index.isValid() || index.row() >= m_values.size()) {
    return false;
  }
  if (role == Qt::EditRole || role == Qt::UserRole) {
    m_values[index.row()] = normalizeJSValue(value);
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::UserRole, role});
    return true;
  }
  return false;
}

Qt::ItemFlags ListOptionItemModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void ListOptionItemModel::addItem(const QVariant &value) {
  int row = m_values.size();
  beginInsertRows(QModelIndex(), row, row);
  m_values.append(normalizeJSValue(value));
  endInsertRows();
}

void ListOptionItemModel::removeItem(int index) {
  if (index >= 0 && index < m_values.size()) {
    beginRemoveRows(QModelIndex(), index, index);
    m_values.removeAt(index);
    endRemoveRows();
  }
}

void ListOptionItemModel::moveUp(int index) {
  if (index > 0 && index < m_values.size()) {
    beginMoveRows(QModelIndex(), index, index, QModelIndex(), index - 1);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    m_values.swapItemsAt(index, index - 1);
#else
    m_values.swap(index, index - 1);
#endif
    endMoveRows();
  }
}

void ListOptionItemModel::moveDown(int index) {
  if (index >= 0 && index < m_values.size() - 1) {
    beginMoveRows(QModelIndex(), index, index, QModelIndex(), index + 2);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    m_values.swapItemsAt(index, index + 1);
#else
    m_values.swap(index, index + 1);
#endif
    endMoveRows();
  }
}

void ListOptionItemModel::editItem(int index, const QVariant &value) {
  if (index >= 0 && index < m_values.size()) {
    QModelIndex modelIndex = createIndex(index, 0);
    setData(modelIndex, value);
  }
}

QVariant ListOptionItemModel::getItem(int index) const {
  if (index >= 0 && index < m_values.size()) {
    return normalizeVariant(m_values.at(index));
  }
  return QVariant();
}

ListOptionItem::ListOptionItem(const FcitxQtConfigOption &option,
                               const QString &path, QObject *parent)
    : ConfigItem(path, parent), m_name(option.name()),
      m_desc(option.description()),
      m_model(std::make_unique<ListOptionItemModel>(this)) {
  m_model->setPrettifyHandler(
      [this](const QVariant &value) { return this->prettifyValue(value); });

  auto variant = option.defaultValue().variant();
  if (variant.canConvert<QDBusArgument>()) {
    auto argument = qvariant_cast<QDBusArgument>(variant);
    argument >> m_defaultValue;
  }

  if (option.type().startsWith("List|")) {
    m_subType = option.type().mid(5); // 移除 "List|" 前缀
    m_subOption = option;
    m_subOption.setType(m_subType);
    auto props = option.properties();
    if (props.contains("ListConstrain")) {
      auto constrain = props.value("ListConstrain").toMap();
      props.remove("ListConstrain");
      for (auto iter = constrain.cbegin(); iter != constrain.cend(); ++iter) {
        props[iter.key()] = iter.value();
      }
    }
    m_subOption.setProperties(props);
    m_subOption.setDefaultValue(QDBusVariant());

    if (const auto *model = getConfigModel(this)) {
      if (!m_subType.isEmpty()) {
        if (const auto *options = model->optionsForType(m_subType)) {
          for (const auto &opt : *options) {
            m_subFields << buildFieldDefinition(opt);
          }
        }
      }
    }
  }
}

void ListOptionItem::readValueFrom(const QVariantMap &map) {
  m_model->readValueFrom(map, path());
  emit countChanged();
  emit valueChanged();
}

void ListOptionItem::writeValueTo(QVariantMap &map) {
  m_model->writeValueTo(map, path());
}

void ListOptionItem::restoreToDefault() {
  m_model->readValueFrom(m_defaultValue, "");
  emit countChanged();
  emit valueChanged();
}

QString ListOptionItem::name() const { return m_name; }

QString ListOptionItem::type() const { return m_type; }

QString ListOptionItem::desc() const { return m_desc; }

QString ListOptionItem::subType() const { return m_subType; }

QString ListOptionItem::subDescription() const {
  return m_subOption.description();
}

QAbstractListModel *ListOptionItem::model() const { return m_model.get(); }

int ListOptionItem::count() const { return m_model->rowCount(); }

QVariantMap ListOptionItem::subProperties() const {
  return m_subOption.properties();
}

QVariant ListOptionItem::defaultSubValue() const {
  if (m_subOption.name().isEmpty()) {
    return QVariant();
  }
  return normalizeVariant(m_subOption.defaultValue().variant());
}

QVariantList ListOptionItem::subFields() const { return m_subFields; }

QVariantList ListOptionItem::subEnumOptions() const {
  if (!m_subEnumInitialized) {
    m_subEnumInitialized = true;
    if (m_subOption.type() == "Enum") {
      auto props = m_subOption.properties();
      int i = 0;
      while (true) {
        const auto key = QString("Enum/%1").arg(i);
        const auto value = readString(props, key);
        if (value.isEmpty()) {
          break;
        }
        auto text = readString(props, QString("EnumI18n/%1").arg(i));
        if (text.isEmpty()) {
          text = value;
        }
        QVariantMap entry;
        entry["value"] = value;
        entry["text"] = text;
        m_subEnumOptions << entry;
        i++;
      }
    }
  }
  return m_subEnumOptions;
}

void ListOptionItem::addItem(const QVariant &value) {
  m_model->addItem(value);
  emit countChanged();
  emit valueChanged();
}

void ListOptionItem::removeItem(int index) {
  m_model->removeItem(index);
  emit countChanged();
  emit valueChanged();
}

void ListOptionItem::moveUp(int index) {
  m_model->moveUp(index);
  emit valueChanged();
}

void ListOptionItem::moveDown(int index) {
  m_model->moveDown(index);
  emit valueChanged();
}

void ListOptionItem::editItem(int index, const QVariant &value) {
  m_model->editItem(index, value);
  emit valueChanged();
}

QVariant ListOptionItem::getItem(int index) const {
  return m_model->getItem(index);
}

QVariant ListOptionItem::prepareEditValue(const QVariant &rawValue) const {
  if (m_subFields.isEmpty()) {
    return normalizeByType(m_subType, rawValue, defaultSubValue());
  }
  return parseStructuredValue(rawValue);
}

QVariant ListOptionItem::serializeEditValue(const QVariant &editValue) const {
  QVariant normalizedValue = normalizeJSValue(editValue);
  if (m_subFields.isEmpty()) {
    return serializeByType(m_subType, normalizedValue, defaultSubValue());
  }
  return serializeStructuredValue(normalizedValue);
}

QVariantMap
ListOptionItem::parseStructuredValue(const QVariant &rawValue) const {
  QVariantMap rawMap;
  if (rawValue.canConvert<QVariantMap>()) {
    rawMap = rawValue.toMap();
  } else if (rawValue.canConvert<QDBusArgument>()) {
    auto argument = qvariant_cast<QDBusArgument>(rawValue);
    argument >> rawMap;
  } else if (rawValue.type() == QVariant::String) {
    const auto str = rawValue.toString();
    if (!str.isEmpty()) {
      QJsonParseError error;
      const auto doc = QJsonDocument::fromJson(str.toUtf8(), &error);
      if (error.error == QJsonParseError::NoError && doc.isObject()) {
        rawMap = doc.object().toVariantMap();
      }
    }
  }

  QVariantMap result;
  for (const auto &fieldVar : m_subFields) {
    const auto field = fieldVar.toMap();
    const auto fieldName = field["name"].toString();
    if (fieldName.isEmpty()) {
      continue;
    }
    QVariant fieldValue = rawMap.value(fieldName);
    if (!fieldValue.isValid()) {
      bool ok;
      const auto numKey = fieldName.toInt(&ok);
      if (ok) {
        fieldValue = rawMap.value(QString::number(numKey));
      }
    }
    result[fieldName] =
        normalizeByType(field.value("type").toString(), fieldValue,
                        field.value("defaultValue"));
  }
  return result;
}

QVariantMap
ListOptionItem::serializeStructuredValue(const QVariant &editValue) const {
  QVariantMap editMap =
      editValue.canConvert<QVariantMap>() ? editValue.toMap() : QVariantMap();
  QVariantMap result;
  for (const auto &fieldVar : m_subFields) {
    const auto field = fieldVar.toMap();
    const auto fieldName = field["name"].toString();
    if (!fieldName.isEmpty()) {
      QVariant fieldValue = editMap.value(fieldName);
      result[fieldName] =
          serializeByType(field.value("type").toString(), fieldValue,
                          field.value("defaultValue"));
    }
  }
  return result;
}

QString ListOptionItem::prettifyValue(const QVariant &value) const {
  if (m_subOption.type().isEmpty()) {
    return value.toString();
  }
  return prettifyOption(m_subOption, value);
}

QString ListOptionItem::prettifyOption(const FcitxQtConfigOption &option,
                                       const QVariant &value) const {
  const auto type = option.type();
  if (type == "Integer" || type == "String" || type == "Key") {
    return value.toString();
  }
  if (type == "Boolean") {
    const auto str = value.toString();
    if (str.compare("True", Qt::CaseInsensitive) == 0 ||
        str.compare("1") == 0) {
      return tr("Yes");
    }
    if (str.compare("False", Qt::CaseInsensitive) == 0 ||
        str.compare("0") == 0) {
      return tr("No");
    }
    return str;
  }
  if (type == "Enum") {
    QMap<QString, QString> enumMap;
    const auto props = option.properties();
    int i = 0;
    while (true) {
      const auto key = readString(props, QString("Enum/%1").arg(i));
      if (key.isEmpty()) {
        break;
      }
      auto text = readString(props, QString("EnumI18n/%1").arg(i));
      if (text.isEmpty()) {
        text = key;
      }
      enumMap.insert(key, text);
      i++;
    }
    const auto enumValue = value.toString();
    return enumMap.value(enumValue, enumValue);
  }
  if (type.startsWith("List|")) {
    FcitxQtConfigOption subOption(option);
    subOption.setType(type.mid(5));
    auto props = option.properties();
    if (props.contains("ListConstrain")) {
      auto constrain = props.value("ListConstrain").toMap();
      props.remove("ListConstrain");
      for (auto iter = constrain.cbegin(); iter != constrain.cend(); ++iter) {
        props[iter.key()] = iter.value();
      }
    }
    subOption.setProperties(props);
    subOption.setDefaultValue(QDBusVariant());
    QStringList parts;
    int i = 0;
    while (true) {
      auto subValue = readVariant(value, QString::number(i));
      if (subValue.isNull()) {
        break;
      }
      parts << prettifyOption(subOption, subValue);
      i++;
    }
    return QStringLiteral("[%1]").arg(parts.join(QStringLiteral(" ")));
  }
  auto displayOption =
      option.properties().value("ListDisplayOption").toString();
  if (displayOption.isEmpty()) {
    return value.toString();
  }

  auto displayValue = readVariant(value, displayOption);
  if (displayValue.isNull()) {
    return value.toString();
  }

  const auto *model = getConfigModel(this);
  if (!model) {
    return displayValue.toString();
  }

  const auto *options = model->optionsForType(option.type());
  if (!options) {
    return displayValue.toString();
  }

  for (const auto &subOption : *options) {
    if (subOption.name() == displayOption) {
      return prettifyOption(subOption, displayValue);
    }
  }

  return displayValue.toString();
}
