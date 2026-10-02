/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LocationListItem_H
#define LocationListItem_H
#include <QLabel>
#include <QPushButton>
namespace kmre_gps
{
class LocationListItem : public QPushButton
{
    Q_OBJECT
public:
    LocationListItem(QPushButton *parent = nullptr);
    ~LocationListItem();
    //设置项名称
    void setItemName(QString name);

private:
    void initWidget();
    //项名称
    QLabel *m_itemNameLabel = nullptr;
};
} // namespace kmre_gps
#endif // LocationListItem_H
