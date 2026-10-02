/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "qgsettings.h"

namespace {

struct SchemaState
{
    bool installed = false;
    QHash<QString, QVariant> values;
    QList<QPair<QString, QVariant> > setCalls;
};

QHash<QByteArray, SchemaState> &schemaStore()
{
    static QHash<QByteArray, SchemaState> store;
    return store;
}

QByteArray &currentSchema(QGSettings *instance)
{
    static QHash<QGSettings *, QByteArray> schemas;
    return schemas[instance];
}

}

QGSettings::QGSettings(const QByteArray &schema_id, const QByteArray &path, QObject *parent)
    : QObject(parent)
{
    Q_UNUSED(path);
    currentSchema(this) = schema_id;
}

QVariant QGSettings::get(const QString &key) const
{
    return schemaStore().value(currentSchema(const_cast<QGSettings *>(this))).values.value(key);
}

void QGSettings::set(const QString &key, const QVariant &value)
{
    SchemaState &state = schemaStore()[currentSchema(this)];
    state.values.insert(key, value);
    state.setCalls.append(qMakePair(key, value));
}

bool QGSettings::trySet(const QString &key, const QVariant &value)
{
    set(key, value);
    return true;
}

QStringList QGSettings::keys() const
{
    return schemaStore().value(currentSchema(const_cast<QGSettings *>(this))).values.keys();
}

QVariantList QGSettings::choices(const QString &key) const
{
    Q_UNUSED(key);
    return {};
}

void QGSettings::reset(const QString &key)
{
    schemaStore()[currentSchema(this)].values.remove(key);
}

bool QGSettings::isSchemaInstalled(const QByteArray &schema_id)
{
    return schemaStore().value(schema_id).installed;
}

namespace FakeQGSettingsStore {

void reset()
{
    schemaStore().clear();
}

void setSchemaInstalled(const QByteArray &schema, bool installed)
{
    schemaStore()[schema].installed = installed;
}

void setSchemaData(const QByteArray &schema, const QHash<QString, QVariant> &values)
{
    schemaStore()[schema].values = values;
}

QHash<QString, QVariant> schemaData(const QByteArray &schema)
{
    return schemaStore().value(schema).values;
}

QList<QPair<QString, QVariant> > setCalls(const QByteArray &schema)
{
    return schemaStore().value(schema).setCalls;
}

}
