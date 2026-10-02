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

#ifndef AUTOTEST_UTILS_FAKE_FUNCTION_CONTROL_H
#define AUTOTEST_UTILS_FAKE_FUNCTION_CONTROL_H

#include <QObject>
#include <QJsonObject>

namespace UkuiQuick {

class FunctionControl : public QObject
{
    Q_OBJECT
public:
    static FunctionControl *instance();

    void addWatch(const QString &name);
    QJsonObject functionControlObject(const QString &name) const;

    static void reset();
    static void setFunctionControlObject(const QString &name, const QJsonObject &object);
    static QStringList watchedNames();

Q_SIGNALS:
    void functionControlChanged(const QString &name);
    void functionControlExit();
};

} // UkuiQuick

#endif
