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
#ifndef CUSTOM_TRANS_WIDGET_H
#define CUSTOM_TRANS_WIDGET_H

#include <QWidget>


/**
 * @brief 解决默认widget毛玻璃特效问题
 * 
 */
class CustomTransWidget : public QWidget
{
    Q_OBJECT
private:
    double m_tran;//透明度

    void transChange(int transparency);
    void initConnect();
    void paintEvent(QPaintEvent *event);
public:
    explicit CustomTransWidget(QWidget *parent = nullptr);
    ~CustomTransWidget();
    
};

#endif // CUSTOM_TRANS_WIDGET_H