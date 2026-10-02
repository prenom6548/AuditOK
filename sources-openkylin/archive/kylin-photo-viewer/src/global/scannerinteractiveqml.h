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
#ifndef SCANNERINTERACTIVEQML_H
#define SCANNERINTERACTIVEQML_H

#include <QObject>
#include <QDebug>
#include "global/variable.h"

class ScannerInteractiveQml : public QObject
{
    Q_OBJECT
public:
    explicit ScannerInteractiveQml(QObject *parent = nullptr);
    Q_PROPERTY(int operateWay READ getOperateWay WRITE setOperateWay NOTIFY operateWayChanged)

    static ScannerInteractiveQml *getInstance();    //获取单例
    int getOperateWay();                            //操作方式
    void setOperateWay(int way);

private:
    static ScannerInteractiveQml *m_interactionQml; //单例指针
    int m_operateWay = 0;           //操作方式

Q_SIGNALS:
    void operateWayChanged(QVariant);       //操作方式

};

#endif // SCANNERINTERACTIVEQML_H
