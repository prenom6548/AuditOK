/*
 * UKUI Intelligent Data Manager
 *
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef AICONDITIONSHELPER_H
#define AICONDITIONSHELPER_H

#include <QObject>

#ifdef USE_AI
#include <business/data-management/datamanagement.h>
#endif

class AIConditionsHelper : public QObject
{
    Q_OBJECT
public:
    explicit AIConditionsHelper(QObject *parent = nullptr);
    ~AIConditionsHelper();

    QJsonDocument pharseConditions(const QString &description, bool *ok = nullptr);

    bool isFileMatchedWithDescriptionCondition(const QString &filePath, const QString &descriptionCondition, bool *ok = nullptr);

    bool isFileMatchedWithLables(const QString &filePath, const QStringList &labels, bool *ok = nullptr);

    bool isValid();

private:
    void setOkResult(bool *ok, bool result);
#ifdef USE_AI
    DataManagementSession *session = nullptr;
#endif
    bool inited = false;
};

#endif // AICONDITIONSHELPER_H
