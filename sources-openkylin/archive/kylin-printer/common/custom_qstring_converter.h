/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#ifndef CUSTOM_QSTRING_CONVERTER
#define CUSTOM_QSTRING_CONVERTER

#include <QString>
#include <QByteArrayList>

#define QTC(x) (CustomQStringConverter(x).QStringToChar())

class CustomQStringConverter {
public:
    CustomQStringConverter(const QString &qstr = QString());
    ~CustomQStringConverter();
    char *QStringToChar(const QString &qstr = QString());
private:
    void append(const QString &qstr = QString());
    QByteArrayList m_buff;
};


#endif // CUSTOM_QSTRING_CONVERTER