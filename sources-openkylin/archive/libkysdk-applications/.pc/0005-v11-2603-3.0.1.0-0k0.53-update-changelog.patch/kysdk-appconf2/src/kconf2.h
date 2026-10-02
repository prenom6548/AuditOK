/*
 * libkysdk-appconf2's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#ifndef KCONF2_H
#define KCONF2_H

#include <QObject>
#include <QVariant>
#include <libkysettings.h>
namespace kdk
{

class KConf2Private;

class KConf2 : public QObject
{
    Q_OBJECT
public:
    KConf2(const QByteArray &schema_id);

    ~KConf2();

    static KConf2 *globalInstance();

    QVariant get(const QString &key) const;

    void set(const QString &key, const QVariant &value);

    bool trySet(const QString &key, const QVariant &value);

    QStringList keys() const;

    void reset(const QString &key);

    static bool isSettingsAvailable(const QString &schema_id);

    QStringList schemas(const QString &app, const QString &version) const;

    QString getDescrition(QString &key);

    QString getSummary(QString &key);

    QString getType(QString &key);

    bool isWritable(QString &key);

    QString getId();

    QString getVersion();

    KSettings *getChild(QString &name);

    QStringList children() const;

Q_SIGNALS:
    void changed(const QString &key);

private:
    Q_DECLARE_PRIVATE(KConf2)
    KConf2Private *const d_ptr;
};

}
#endif // KCONF2_H
