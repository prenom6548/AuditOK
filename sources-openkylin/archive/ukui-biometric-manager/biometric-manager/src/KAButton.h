/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef KABUTTON_H
#define KABUTTON_H

#include <QPushButton>

class KAButton : public QPushButton
{
    Q_OBJECT
public:
    KAButton(QWidget *parent = nullptr);
    KAButton(QString strText, QWidget *parent = nullptr);
    QString getElidedText(QFont font,int width,QString strInfo);
public slots:
    void setText(const QString &);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QString m_strText;
};

#endif // KABUTTON_H
