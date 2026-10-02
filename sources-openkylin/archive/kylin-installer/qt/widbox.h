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
#ifndef WIDBOX_H
#define WIDBOX_H
#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGSettings>


class widbox:public QWidget
{
    Q_OBJECT
public:
    widbox(QString text,QWidget *parent = nullptr);

    QLabel * widinfo;

    QLabel * windicon;

    QLabel * icon;

    QPushButton * widbutton;

    QPushButton * buttonclose;

    //QGSettings     *m_pGsettings;

    QVBoxLayout * Allbox;

    QHBoxLayout * layoutBox1;

    QHBoxLayout * layoutBox2;

    QHBoxLayout * layoutBox3;

    QString textname;

    QGSettings * m_pGsettingFontSize;

    QString setAutoWrap(const QFontMetrics& font, const QString& text, int nLabelSize);

    void gsettingInit();
    QFont getSystemFont(int size );




};

#endif // WIDBOX_H
