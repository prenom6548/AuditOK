/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#ifndef KTIPLABEL_H
#define KTIPLABEL_H

#include <QLabel>
#include "klabel.h"

class KTipLabel : public KLabel
{
    Q_OBJECT
public:
    KTipLabel(QWidget *parent = nullptr);
    KTipLabel(QString strText, QWidget *parent = nullptr);
    QString getElidedText(QFont font,int width,QString strInfo);
public slots:
    void setText(const QString &);
//    void setKToolTip(const QString text);
protected:
    void paintEvent(QPaintEvent *event);
    //bool eventFilter(QObject *obj, QEvent *event);
private:
    QString m_strText;

};

#endif // KTIPLABEL_H
