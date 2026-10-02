/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: maomao <baijunjie@kylinos.cn>
 *
 */
#ifndef UKUI_SEARCH_DATEFORMAT_H
#define UKUI_SEARCH_DATEFORMAT_H

#include <QObject>
#include <QMutexLocker>
#include <QString>
#include <QDebug>
#include <QDBusConnection>
#include <libkydate.h>

static QMutex g_formatMutex;
static const QString DEFAULT_DATETIME_FORMAT = "yyyy/MM/dd hh:mm:ss";

class DateFormat : public QObject
{
    Q_OBJECT

public:
    static DateFormat& getInstance() {
        static DateFormat instance;
        return instance;
    }

    QString format() {
        QMutexLocker locker(&g_formatMutex);
        if (m_dateFormat == DEFAULT_DATETIME_FORMAT) {
            return QString();
        }
        return m_dateFormat;
    }

public Q_SLOTS:
            void updateDateFormat(QString format) {
        QMutexLocker locker(&g_formatMutex);
        m_dateFormat = format + " hh:mm:ss";
    }
private:
    explicit DateFormat(QObject *parent = nullptr) : QObject(parent)
    {
        char* shortFormat = kdk_system_get_shortformat();
        m_dateFormat = shortFormat;
        free(shortFormat);
        if (m_dateFormat.isEmpty()) {
            m_dateFormat = DEFAULT_DATETIME_FORMAT;
        } else {
            m_dateFormat += " hh:mm:ss";
        }

        qDebug() << "connect sdk date signal" << QDBusConnection::sessionBus().connect("com.kylin.kysdk.DateServer",
                                                                                       "/com/kylin/kysdk/Date",
                                                                                       "com.kylin.kysdk.DateInterface",
                                                                                       "ShortDateSignal",
                                                                                       this, SLOT(updateDateFormat(QString)));
    }
    DateFormat(const DateFormat&) = delete;
    DateFormat& operator = (const DateFormat&) = delete;
    ~DateFormat() = default;

    QString m_dateFormat;
};


#endif //UKUI_SEARCH_DATEFORMAT_H
