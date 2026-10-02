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
#ifndef SUREWIDGET_H
#define SUREWIDGET_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class Surewidget;
}

class Configuration;

class Surewidget : public QWidget
{
    Q_OBJECT

public:
    explicit Surewidget(QWidget *parent = nullptr);
    ~Surewidget();
    void setText(const QString tips);
    void setStatus(int stateType, int lockType);

private:
    Ui::Surewidget *ui;
    void on_cancelBtn_Clicked();
    void onLoadingImage();
    void updatePixmap();
    static QString getHibited_tr(int type);

Q_SIGNALS:
    void cancelButtonclicked();
    void confirmButtonclicked();

private:
    int blockStatus = 0;
    QPixmap         m_waitingPixmap;
    QTimer          *w_timer;
    Configuration   *m_configuration;
    double          curFontSize;
    double m_ptToPx = 1.0;
    int             m_stateType = 0;
};

#endif // SUREWIDGET_H
