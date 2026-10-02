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

#ifndef CONDITIONEDIT_H
#define CONDITIONEDIT_H

#include <QWidget>

#include "condition.h"

class ConditionEdit : public QWidget
{
    Q_OBJECT
public:
    explicit ConditionEdit(Condition::Type type, QWidget *parent = nullptr);
    explicit ConditionEdit(QSharedPointer<Condition> condition, QWidget *parent = nullptr);

    QSharedPointer<Condition> getCondition();

protected slots:
    void initUi();
    void switchType(Condition::Type type);

private:
    Condition::Type m_type;
    bool m_isAndCondition = true;
};

#endif // CONDITIONEDIT_H
