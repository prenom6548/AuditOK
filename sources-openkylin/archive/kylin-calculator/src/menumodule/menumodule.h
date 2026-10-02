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

#ifndef MENUMODULE_H
#define MENUMODULE_H

#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include <QKeyEvent>
#include <QToolButton>
#include <QDebug>
#include <QString>
#include <QLabel>
#include <QRect>
#include <QScreen>
#include <QDialog>
#include <libkydatacollect.h>
#include "kaboutdialog.h"

#include "highlight-effect.h"

class menuModule : public QWidget
{
    Q_OBJECT
public:
    explicit menuModule(QWidget *);

signals:
    void menuModuleClose();
    void menuModuleChanged(QString);
    void pullupHelp();

public:
    QToolButton *menuButton = nullptr;

public:
    kdk::KAboutDialog *m_aboutWindow = nullptr;

private:
    QMenu *m_menu = nullptr;
    QMenu *themeMenu = nullptr;
    QSize iconSize;

public slots:

private:
    void keyPressEvent(QKeyEvent *event);
    void init();
    void initAction();
    void triggerMenu(QAction *act); //主菜单动作4
    void aboutAction();
    void helpAction();
    QString getVersion();
};

#endif // MENUMODULE_H
