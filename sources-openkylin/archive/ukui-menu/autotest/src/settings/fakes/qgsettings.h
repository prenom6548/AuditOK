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

#ifndef AUTOTEST_FAKE_QGSETTINGS_H
#define AUTOTEST_FAKE_QGSETTINGS_H

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QVariant>

class QGSettings : public QObject
{
    Q_OBJECT

public:
    QGSettings(const QByteArray &schema_id, const QByteArray &path = QByteArray(), QObject *parent = nullptr);
    ~QGSettings() override = default;

    QVariant get(const QString &key) const;
    void set(const QString &key, const QVariant &value);
    bool trySet(const QString &key, const QVariant &value);
    QStringList keys() const;
    QVariantList choices(const QString &key) const;
    void reset(const QString &key);

    static bool isSchemaInstalled(const QByteArray &schema_id);

Q_SIGNALS:
    void changed(const QString &key);
};

namespace FakeQGSettingsStore {

void reset();
void setSchemaInstalled(const QByteArray &schema, bool installed);
void setSchemaData(const QByteArray &schema, const QHash<QString, QVariant> &values);
QHash<QString, QVariant> schemaData(const QByteArray &schema);
QList<QPair<QString, QVariant> > setCalls(const QByteArray &schema);

}

#endif
