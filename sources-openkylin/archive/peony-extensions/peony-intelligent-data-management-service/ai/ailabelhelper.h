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

#ifndef AILABELHELPER_H
#define AILABELHELPER_H

#include <QObject>

#ifdef USE_AI
#include <business/data-management/datamanagement.h>
#endif

class AILabelHelper : public QObject
{
    Q_OBJECT
public:
    explicit AILabelHelper(QObject *parent = nullptr);
    ~AILabelHelper();

    bool isValid();

    QStringList getTopLabels(int maxCount, bool *ok = nullptr);
    QStringList getSimilarLabels(const QString &label, bool *ok = nullptr);

    QStringList getFileLabels(const QString &filePath, bool *ok = nullptr);
    QStringList getFilesByLabels(const QStringList &labels, bool *ok = nullptr);

private:
    void setOkResult(bool *ok, bool result);
#ifdef USE_AI
    DataManagementSession *session = nullptr;
#endif
    bool inited = false;
};

#endif // AILABELHELPER_H
