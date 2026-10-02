/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QList>
#include <QTimer>

class QMovie;

class LoadingWidget : public QWidget
{
    Q_OBJECT

public:
    LoadingWidget(QWidget * parent = 0);
    ~LoadingWidget();

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent* event) Q_DECL_OVERRIDE;

public slots:
    void onUpdateIcon();

private:
    QMovie *m_movie = nullptr;
    QLabel *m_label = nullptr;

    bool mUseUkuiThemeIcon = true;
    QList<QPixmap> mIconList;
    QTimer *mTimer = nullptr;
    uint32_t mCounter = 0;
};

#endif // LOADINGWIDGET_H
