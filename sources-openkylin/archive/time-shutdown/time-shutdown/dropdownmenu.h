/*
 * Ukui-shutdown-timer
 *
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
 *
 * Authors:  zhangtianze <zhangtianze@kylinos.cn>
 *
 */

#ifndef DROPDOWNMENU_H
#define DROPDOWNMENU_H

#include <QObject>
#include <QMenu>
#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QToolButton>
#include <QGSettings>

#define MENU_WIDTH 324

class MenuItem : public QToolButton
{
    Q_OBJECT
public:
    MenuItem(QString week, int id, QWidget *parent = nullptr);
    QLabel       *m_pweekLabel;
    QLabel       *m_pselectedLabelIcon;
    QHBoxLayout  *m_pHWeekLayout;
    int           m_id = 0;

    void setIconVisible(bool status);
    void setSelectIcon(const QPixmap &pixmap);
protected:
    void mouseReleaseEvent(QMouseEvent *event);
Q_SIGNALS:
    void showLabelIcon(int id);
    void hideLabelIcon(int id);
};

class DropDownMenu : public QMenu
{
    Q_OBJECT
public:
    DropDownMenu();
    void initAllDateWidget();
    void initLabelStatus(QList<int> selectWeek);
    QStringList getWeekStringList();
    QString getChosenList(QList<int> selectList);
    QList<int> getWeekDateList(QString indexString);
private:
    void initItemSelectIcon();
    void setItemSelectIcon(const QPixmap &pixmap);
    QPixmap getDarkStylePixmap(const QPixmap &originalPixmap) const;

public Q_SLOTS:
    void handleShowLabelSlot(int id);
    void handleHideLabelSlot(int id);
private:
    QStringList m_pWeekDateList = {
        QObject::tr("never"),
        QObject::tr("Only this shutdown"),
        QObject::tr("Everyday"),
        QObject::tr("Mon"),
        QObject::tr("Tue"),
        QObject::tr("Wed"),
        QObject::tr("Thu"),
        QObject::tr("Fri"),
        QObject::tr("Sat"),
        QObject::tr("Sun")
    };
    QList<int> m_selectWeek;
    QGSettings *m_pStyleGSettings = nullptr;
};

#endif // DROPDOWNMENU_H

