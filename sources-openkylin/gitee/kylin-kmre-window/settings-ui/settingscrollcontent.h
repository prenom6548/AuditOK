/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
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

#ifndef SETTINGSCROLLCONTENT_H
#define SETTINGSCROLLCONTENT_H

#include <QWidget>
#include <QLabel>

class QScrollArea;
class QPropertyAnimation;

class SettingScrollContent : public QWidget
{
    Q_OBJECT

public:
    explicit SettingScrollContent(QWidget *parent = 0);

    void stopScroll();
    QWidget *setContent(QWidget * const widget);
    QWidget *content() const { return m_content; }
    void initAnimation();

protected:
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

protected:
    QScrollArea *m_scrollArea;
    QWidget *m_content;
    QLabel *m_title;
    QPropertyAnimation *m_scrollAnimation;
    QPropertyAnimation *m_wheelAnimation;
    double m_speedPeriod;
};

#endif // SETTINGSCROLLCONTENT_H
