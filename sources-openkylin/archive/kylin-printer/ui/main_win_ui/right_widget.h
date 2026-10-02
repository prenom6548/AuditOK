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
#ifndef RIGHT_WIDGET_H
#define RIGHT_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include "main_win_property_widget.h"
class EmptyWidget:public QWidget
{
    Q_OBJECT
public:
    explicit EmptyWidget(QWidget *parent);

    void setQSSFontSize(QFont curFont);

protected:
    void paintEvent(QPaintEvent *e);

private:
    QLabel* m_first;
    QLabel* m_second;
    QPushButton* m_icon;
    QVBoxLayout* m_VLayoutExhbition;
};

class RightWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RightWidget(QWidget *parent);
    void setQSSFontSize(QFont curFont);
public slots:
    void updateExhbitionSector(const UkuiUsbPrinter&);
    void hideExhbitionSector();

protected:
    void paintEvent(QPaintEvent *) override;
    MainWinPropertyWidget *m_deviceExhbitionMyWidget;
    EmptyWidget *m_emptyWidget;
signals:

private:
    QVBoxLayout *m_VLayoutExhbition;
    QHBoxLayout *m_HLayoutProperty;

    QWidget *m_mainWin;

    void initPropertySectorQSS();
    void initExhbitionSectorQSS(QWidget *parent);
    friend class MainWindow;
};

#endif // RIGHT_WIDGET_H
