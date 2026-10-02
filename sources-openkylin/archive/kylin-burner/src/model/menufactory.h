/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef MENUFACTORY_H
#define MENUFACTORY_H

#include <QObject>
#include <QMutex>
#include <QAction>
#include <QMenu>

class MenuFactoryTestFriend;
class MenuFactory : public QObject
{
    Q_OBJECT
    friend class MenuFactoryTestFriend;
public:
    static MenuFactory *instance();
public:
    MenuFactory(const MenuFactory*)=delete;
    MenuFactory *operator =(const MenuFactory*)=delete;
public:
    QMenu *getMenu(int);
private:
    explicit MenuFactory(QObject *parent = nullptr);
    ~MenuFactory();

public slots:
    void newCDROM(void *);
    void takeCDROM(void *);
    void removeEarse(void *);
    void newEarse(void *);

private slots:
    void checkEject(bool checked);
    void showabout(bool checked);
    void slotsDataSetting(bool checked);
    void slotsDiskCheck(bool checked);
    void slotsHelp(bool checked);

private:
    static MenuFactory *m_oInstance;
    static QMutex       m_oMutex;
private:
    QMenu              *m_oSubEject;
    QMenu              *m_oSubErase;
    QAction            *m_oActionSetting;
    QAction            *m_oActionDataSetting;
    QAction            *m_oActionFileFilter;
    QAction            *m_oActionFileFilterView;
    QAction            *m_oActionDiskCheck;
    QAction            *m_oActionBurnerSetting;
    QAction            *m_oActionEject;
    QAction            *m_oActionEarse;
    QAction            *m_oActionVersion;
    QAction            *m_oActionAbout;
    QAction            *m_oActionHelp;
    QAction            *m_oActionExit;
    QAction            *m_oSeparator;
    QList<QAction *>    m_oCommonActions;
    QList<QMenu *>      m_oMenus;
    QList<void *>       m_oEjects;
    QList<QAction *>    m_oEjectActions;
    QList<void *>       m_oErases;
    QList<QAction *>    m_oEraseActions;
Q_SIGNALS:
    // set burn setting
    void setSetting(bool);
    // set view filter file.
    void setViewFilterFile(bool);
    // menu
    void setting(bool);
    void fileFilter(bool);
    void viewFilterFile(bool);
    void diskCheck(bool);
    void burnSetting(bool);
    void eject(bool);
    void eject(QString);
    void earse(bool);
    void earse(QString);
    void version(bool);
};
#define MENU MenuFactory::instance()

#endif // MENUFACTORY_H
