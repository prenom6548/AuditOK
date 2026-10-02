/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef MATTINGINTERACTIVEQML_H
#define MATTINGINTERACTIVEQML_H

#include <QObject>
#include "global/variable.h"
#include <QDebug>

class MattingInteractiveQml: public QObject
{
    Q_OBJECT
public:
    explicit MattingInteractiveQml(QObject *parent = nullptr);
    Q_PROPERTY(int operateWay READ getOperateWay WRITE setOperateWay NOTIFY operateWayChanged)
    int getOperateWay()
    {
        return m_operateWay;
    }
    void setOperateWay(int way)
    {
        m_operateWay = way;
        Q_EMIT operateWayChanged(m_operateWay);
    }

    static MattingInteractiveQml *getInstance();

private:
    int m_operateWay = 0;
    static MattingInteractiveQml *m_mattingInteractionQml; //单例指针

Q_SIGNALS:
    void operateWayChanged(QVariant);
};

#endif // MATTINGINTERACTIVEQML_H
