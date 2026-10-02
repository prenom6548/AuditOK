/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#include "ukui-style-conf-settings.h"
#include "libkysettings.h"
#include <glib.h>
#include <QDebug>

//char to QString
QString qtify_name(const char *name)
{
    bool next_cap = false;
    QString result;

    while (*name) {
        if (*name == '-') {
            next_cap = true;
        } else if (next_cap) {
            result.append(QChar(*name).toUpper().toLatin1());
            next_cap = false;
        } else {
            result.append(*name);
        }

        name++;
    }

    return result;
}

//QString to char
char * unqtify_name(const QString &name)
{
    const char *p;
    QByteArray bytes;
    GString *str;

    bytes = name.toUtf8();
    str = g_string_new (NULL);

    for (p = bytes.constData(); *p; p++) {
        const QChar c(*p);
        if (c.isUpper()) {
            g_string_append_c (str, '-');
            g_string_append_c (str, c.toLower().toLatin1());
        }
        else {
            g_string_append_c (str, *p);
        }
    }

    return g_string_free(str, FALSE);
}


static void settingChanged(KSettings *ukuistyle_settings, const char *key, void *user_data)
{
    ukuiStyleConfSettings *self = (ukuiStyleConfSettings *)user_data;

    QMetaObject::invokeMethod(self, "changed", Qt::QueuedConnection, Q_ARG(QString, qtify_name(key)));
}

ukuiStyleConfSettings::ukuiStyleConfSettings(const QByteArray &schema_id)
{
    if (!ukuistyle_settings)
        ukuistyle_settings = kdk_conf2_new(schema_id, NULL);

    signal_handler_id = kdk_conf2_connect_signal(ukuistyle_settings, "changed",
                                                 (KCallBack)settingChanged, this);
}

ukuiStyleConfSettings::~ukuiStyleConfSettings()
{
    if (ukuistyle_settings) {
        kdk_conf2_ksettings_destroy(ukuistyle_settings);
        ukuistyle_settings = NULL;
    }
}

QVariant ukuiStyleConfSettings::types_to_qvariant(const char *key,const char* value) const
{
    char *type = kdk_conf2_get_type(this->ukuistyle_settings, key);
    switch (*type) {
    case VARIANT_CLASS_BOOLEAN:
        return QVariant(value);

    case VARIANT_CLASS_BYTE:
        return QVariant(QString(value));

    case VARIANT_CLASS_INT16:
        return QVariant(QString(value).toShort());

    case VARIANT_CLASS_UINT16:
        return QVariant(QString(value).toUShort());

    case VARIANT_CLASS_INT32:
        return QVariant(QString(value).toInt());

    case VARIANT_CLASS_UINT32:
        return QVariant(QString(value).toUInt());

    case VARIANT_CLASS_INT64:
        return QVariant(QString(value).toLongLong());

    case VARIANT_CLASS_UINT64:
        return QVariant(QString(value).toULongLong());

    case VARIANT_CLASS_DOUBLE:
        return QVariant(QString(value).toDouble());

    case VARIANT_CLASS_STRING:
        return QVariant(QString(value));

    case VARIANT_CLASS_ARRAY:
//        if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING_ARRAY)) {
//            GVariantIter iter;
//            QStringList list;
//            const gchar *str;

//            g_variant_iter_init (&iter, value);
//            while (g_variant_iter_next (&iter, "&s", &str))
//                list.append (str);

//            return QVariant(list);
//        } else if (g_variant_is_of_type(value, G_VARIANT_TYPE_BYTESTRING)) {
//            return QVariant(QByteArray(g_variant_get_bytestring(value)));
//        } else if (g_variant_is_of_type(value, G_VARIANT_TYPE("a{ss}"))) {
//            GVariantIter iter;
//            QMap<QString, QVariant> map;
//            const gchar *key;
//            const gchar *val;

//            g_variant_iter_init (&iter, value);
//            while (g_variant_iter_next (&iter, "{&s&s}", &key, &val))
//                map.insert(key, QVariant(val));

//            return map;
//        }

        // fall through
    default:
        g_assert_not_reached();
    }
}

QVariant ukuiStyleConfSettings::get(const QString &key) const
{
    char *ckey = unqtify_name(key);
    char *value = kdk_conf2_get_value(ukuistyle_settings, ckey);

    QVariant qvalue = types_to_qvariant(ckey, value);

    g_free(value);
    g_free(ckey);
    return qvalue;
}

void ukuiStyleConfSettings::set(const QString &key, const QVariant &value)
{
    if (!this->trySet(key, value))
        qWarning("unable to set key '%s' to value '%s'", key.toUtf8().constData(), value.toString().toUtf8().constData());
}

bool ukuiStyleConfSettings::trySet(const QString &key, const QVariant &value)
{
    if (!ukuistyle_settings)
        return false;

    char *ckey = unqtify_name(key);
    char *cvalue = unqtify_name(value.value<QString>());
    bool success = false;

    success = kdk_conf2_set_value(ukuistyle_settings, ckey, cvalue);
    g_free(ckey);
    g_free(cvalue);
    return success;
}

QStringList ukuiStyleConfSettings::keys() const
{
    QStringList list;

    char **keys = kdk_conf2_list_keys(ukuistyle_settings);
    for (int i = 0; keys[i]; i++)
        list.append(qtify_name(keys[i]));

    g_strfreev(keys);

    return list;
}

void ukuiStyleConfSettings::reset(const QString &qkey)
{
    if (!ukuistyle_settings)
        return;

    char *key = unqtify_name(qkey);
    kdk_conf2_reset(ukuistyle_settings, key);
    g_free(key);
}

bool ukuiStyleConfSettings::isSettingsAvailable(const QString &schema_id)
{
    char *schema = unqtify_name(schema_id);
    bool result = false;
    if (kdk_conf2_new(schema, NULL)) {
        result = true;
    }

    g_free(schema);
    return result;
}
