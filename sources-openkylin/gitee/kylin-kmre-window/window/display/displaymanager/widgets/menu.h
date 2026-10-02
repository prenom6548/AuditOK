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

#ifndef MENU_H
#define MENU_H

#include <QMenu>

class Action;

class Menu : public QMenu
{
    Q_OBJECT
public:
    Menu(QWidget *parent = nullptr);
};

class TitleMenu : public Menu
{
    Q_OBJECT
public:
    explicit TitleMenu(const QString &pkgName, QWidget *parent = nullptr);
    ~TitleMenu();

    void updateUi();
    void updateAppConfigMenuUi();

signals:
    void sigManual();
    void sigClearApk();
    void sigCloseEnv();
    void sigPref();
    void sigAbout();
    void sigQuit();
    void sigScreenShot();
    void sigRecordScreen();
    void sigKeyboard();
    void sigOpenStorage(const QString &path);
    void sigOpenGallery(const QString &path);
    void sigOpenMMFiles(const QString &path);
    void sigOpenQQFiles(const QString &path);
    void sigToTop(bool top);
    void sigToHide(bool hide);
    void sigAppMultiplier(bool b);
    void sigLockScreen();
    void sigShake();
    void sigGps();
    void sigSensor();
    void sigRotate();
    void sigJoystick(); //增加游戏按键的信号
    void sigscroll();   //调整滚轮灵敏度
    void sigMultipleEnabled(bool enable);

private:
    void createOneLevelAction();
    void createAppConfigMenu();
    void updateActions(QString name, Action *action, QMenu *menu);
    QString initLink();

private:
    Menu   *m_appConfigMenu = nullptr;
    Action *mMultipleAction = nullptr;
    Action *mBootAction = nullptr;
    Action *m_manulAction = nullptr;
    //Action *m_closeEnvAciton = nullptr;
    Action *m_prefAction = nullptr;
    //Action *m_screenshotAction = nullptr;
    Action *m_recordScreenAction = nullptr;
    Action *m_keyboardAction = nullptr;
    Action *m_openStorageAction = nullptr;
    Action *m_aboutAction = nullptr;
    Action *m_quitAction = nullptr;
    Action *m_shakeAction = nullptr;
    //Action *m_gpsAction = nullptr;
    Action *m_sensorAction = nullptr;
    //Action *m_lockScreenAction = nullptr;
    //Action *m_rotationAction = nullptr;
    Action *m_joystickAction = nullptr;
    //Action *m_scrollAction = nullptr;

    QString mPkgName;
};

#endif // MENU_H
