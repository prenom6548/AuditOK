/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef TIPWIDGET_H
#define TIPWIDGET_H

#include <QDialog>

class TipWidget : public QDialog
{
    Q_OBJECT
public:
    enum TipType {
        ERROR,
        OK,
        WARNING
    };

    explicit TipWidget(QWidget *parent = nullptr);
    ~TipWidget();

    QWidget *getWidget(){return w;}
    void setLightTheme();
    void setBlackTheme();

    static void showTip(QString tip, int show_time, QSize size, QWidget* parent = nullptr, TipType type = OK);

signals:

private:
    void resizeEvent(QResizeEvent *event) override;

    QWidget *w;
};

#endif // TIPWIDGET_H
